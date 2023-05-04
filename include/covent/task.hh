/* Copyright 2022 Florian Wagner <florian@wagner-flo.net>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef COVENT_TASK_HH
#define COVENT_TASK_HH

#include <covent/base.hh>
#include <covent/exceptions.hh>

#include <atomic>
#include <coroutine>
#include <memory>

#include <iostream>

namespace covent {

  template<typename>
  class task;

}

namespace covent::detail {

  struct waiter_list {
      std::coroutine_handle<> continuation;
      waiter_list* next;
  };

  // ...
  class final_awaiter {
    public:
      bool await_ready() const noexcept {
        return false;
      }

      void await_resume() const noexcept {
        /* nothing to do here */
      }

      template<typename PromiseType>
      void await_suspend(std::coroutine_handle<PromiseType> coro) noexcept {
        auto& prms = coro.promise();

        // exchange operation needs to be 'release' so that subsequent
        // awaiters have visibility of the result. Also needs to be
        // 'acquire' so we have visibility of writes to the waiters list
        auto waiters = prms.waiters.exchange(
          prms.state_ready, std::memory_order_acq_rel
        );

        if (waiters == nullptr)
          return;

        waiter_list* waiter = static_cast<waiter_list*>(waiters);
        waiter_list* next;

        while ((next = waiter->next) != nullptr) {
          waiter->continuation.resume();
          waiter = next;
        }

        // resume last waiter at the very end to allow it to potentially
        // be compiled as a tail-call
        waiter->continuation.resume();
      }
  };

  template<
    typename PromiseType,
    bool SynchronizeOnly = false
  >
  class task_awaiter {
    protected:
      std::coroutine_handle<PromiseType> coro;
      waiter_list waiter;

    public:
      task_awaiter(std::coroutine_handle<PromiseType> c) noexcept : coro(c) {
        /* nothing to do here */
      }

      bool await_ready() const noexcept {
        return coro == nullptr || coro.promise().done();
      }

      bool await_suspend(std::coroutine_handle<> awaiter) noexcept {
        waiter.continuation = awaiter;

        auto& prms = coro.promise();
        auto& waiters = prms.waiters;
        auto wtr = &waiter;
        auto old = waiters.load(std::memory_order_acquire);

        // if coro not already started: set waiters to nullptr indicates
        // that it's now running but has no waiters yet
        if (old == prms.state_not_started &&
            waiters.compare_exchange_strong(old, nullptr, std::memory_order_relaxed)) {
          coro.resume();
          old = waiters.load(std::memory_order_acquire);
        }

        // enqueue the waiter into the list of waiting coroutines
        while (true) {
          if (old == prms.state_ready)
            return false;
          wtr->next = static_cast<waiter_list*>(old);
          if (waiters.compare_exchange_weak(
                old, wtr, std::memory_order_release, std::memory_order_acquire))
            break;
        }

        return true;
      }

      decltype(auto) await_resume() requires (!SynchronizeOnly) {
        if (!this->coro)
          throw broken_promise();
        return this->coro.promise().result();
      }

      void await_resume() const noexcept {}
  };

  template<typename TaskType>
  class promise_base {
    friend TaskType;
    friend class final_awaiter;
    template<typename, bool> friend class task_awaiter;

    protected:
      std::atomic<std::uint32_t> refcnt;
      std::atomic<void*> waiters;
      std::exception_ptr exception;
      detail::evloop_base& loop;

      // indicates value is ready
      void* const state_ready = this;

      // indicates coroutine not started
      void* const state_not_started = &waiters;

    public:
      promise_base() noexcept
        : refcnt(1),
          waiters(&waiters),
          exception(nullptr),
          loop(get_active_loop()) {
        /* nothing to do here */
      }

      TaskType get_return_object() noexcept {
        return TaskType(
          TaskType::handle_type::from_promise(
            *static_cast<TaskType::promise_type*>(this)
          )
        );
      }

      auto initial_suspend() noexcept {
        return std::suspend_always { };
      }

      auto final_suspend() noexcept {
        return final_awaiter { };
      }

      void unhandled_exception() noexcept {
        exception = std::current_exception();
      }

      bool done() const noexcept {
        return waiters.load(std::memory_order_acquire) == state_ready;
      }

      template<typename R>
      auto await_transform(covent::task<R>& tsk) const noexcept {
        return tsk.operator co_await();
      }

      template<typename ...Args>
      event_awaiter await_transform(Args... args) const noexcept {
        return loop.create_event_awaiter(std::forward<Args>(args)...);
      }
  };

  // ...
  template<
    typename TaskType,
    typename ResultType
  >
  class promise final : public promise_base<TaskType> {
    protected:
      alignas(ResultType) char resval[sizeof(ResultType)];

    public:
      promise() noexcept = default;

      ~promise() {
        if (this->done() && this->exception == nullptr)
          reinterpret_cast<ResultType*>(&resval)->~ResultType();
      }

      template<typename V>
      void return_value(V&& val)
        noexcept ( std::is_nothrow_constructible_v<ResultType, V&&> )
        requires ( std::is_convertible_v<V&&, ResultType> ) {
        new (&resval) ResultType(std::forward<V>(val));
      }

      ResultType& result() {
        if (this->exception != nullptr)
          std::rethrow_exception(this->exception);
        return *reinterpret_cast<ResultType*>(&resval);
      }
  };

  // ...
  template<
    typename TaskType,
    typename ResultType
  >
  class promise<TaskType, ResultType&> final : public promise_base<TaskType> {
    protected:
      ResultType* resval;

    public:
      promise() noexcept = default;

      void return_value(ResultType& val) noexcept {
        resval = std::addressof(val);
      }

      ResultType& result() {
        if (this->exception != nullptr)
          std::rethrow_exception(this->exception);
        return *resval;
      }
  };

  template<
    typename TaskType
  >
  class promise<TaskType, void> final : public promise_base<TaskType> {
    public:
      promise() noexcept = default;

      void return_void() noexcept {
        /* nothing to do here */
      }

      void result() {
        if (this->exception != nullptr)
          std::rethrow_exception(this->exception);
      }
  };
}

namespace covent {

  // ...
  template<
    typename ResultType = void
  >
  class [[nodiscard]] task {
    template<typename R>
    friend bool operator==(const task<R>&, const task<R>&) noexcept;

    template<typename R>
    friend bool operator!=(const task<R>&, const task<R>&) noexcept;

    public:
      using promise_type = detail::promise<task, ResultType>;
      using handle_type = std::coroutine_handle<promise_type>;

    protected:
      handle_type coro;

      void destroy() noexcept {
        if (coro && coro.promise().refcnt.fetch_sub(1, std::memory_order_acq_rel) == 1)
          coro.destroy();
      }

    public:
      task() noexcept : coro(nullptr) {
        /* nothing to do here */
      }

      explicit task(handle_type c) : coro(c) {
        // don't increment the ref-count here since it has already been
        // initialised to 2 (one for task and one for coroutine) in the
        // task_promise constructor
      }

      task(task&& other) noexcept : coro(other.coro) {
        other.coro = nullptr;
      }

      task(const task& other) noexcept : coro(other.coro) {
        if (coro)
          coro.promise().refcnt.fetch_add(1, std::memory_order_relaxed);
      }

      ~task() {
        destroy();
      }

      task& operator=(task&& other) noexcept {
        if (&other != this) {
          destroy();
          coro = other.coro;
          other.coro = nullptr;
        }
        return *this;
      }

      task& operator=(const task& other) noexcept {
        if (coro != other.coro) {
          destroy();
          coro = other.coro;
          if (coro)
            coro.promise().refcnt.fetch_add(1, std::memory_order_relaxed);
        }
        return *this;
      }

      bool done() const noexcept {
        return coro == nullptr || coro.promise().done();
      }

      decltype(auto) result() const {
        if (coro == nullptr)
          throw broken_promise();
        return coro.promise().result();
      }

      detail::task_awaiter<promise_type>
      operator co_await() const noexcept {
        return { coro };
      }

      detail::task_awaiter<promise_type, true>
      when_ready() const noexcept {
        return { coro };
      }
  };

  template<typename R>
  bool operator==(const task<R>& lhs, const task<R>& rhs) noexcept {
    return lhs.coro == rhs.coro;
  }

  template<typename R>
  bool operator!=(const task<R>& lhs, const task<R>& rhs) noexcept {
    return lhs.coro != rhs.coro;
  }

}

#endif
