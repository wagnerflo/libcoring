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
#ifndef COVENT_BASE_HH
#define COVENT_BASE_HH

#include <coroutine>
#include <chrono>

namespace covent {

  // forward declarations of implementation interfaces
  class event_awaiter_impl;
  class task_runner;

  // forward declarations of supported event loops
  class event_loop_uring;

  // default event loop type
  using DefaultEventLoopType = event_loop_uring;


  // ...
  class event_awaiter {
    protected:
      event_awaiter_impl* impl;

    public:
      event_awaiter(event_awaiter_impl*);
      ~event_awaiter();

      // not copyable
      event_awaiter(const event_awaiter&) = delete;
      event_awaiter& operator=(const event_awaiter&) = delete;

      bool await_ready();
      void await_suspend(std::coroutine_handle<>);
      void await_resume();
  };


  class event_loop_impl_base {
    public:
      virtual void run_once() = 0;
      virtual event_awaiter create_event_awaiter(std::chrono::nanoseconds&&) = 0;
  };

  void set_active_loop(event_loop_impl_base*);
  event_loop_impl_base& get_active_loop();

}

#endif
