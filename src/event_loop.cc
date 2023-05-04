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

#include <covent/base.hh>
#include <covent/event_loop.hh>

namespace covent::detail {

  first_task first_task::promise_type::get_return_object() noexcept {
    return { handle_type::from_promise(*this) };
  }

  std::suspend_never first_task::promise_type::initial_suspend() const noexcept {
    return {};
  }

  std::suspend_always first_task::promise_type::final_suspend() const noexcept {
    return {};
  }

  void first_task::promise_type::return_void() const noexcept {
    /* nothing to do here */
  }

  void first_task::promise_type::unhandled_exception() const noexcept {
    /* nothing to do here */
  }

  first_task::first_task(handle_type c) noexcept : coro(c) {
    /* nothing to do here */
  }

  bool first_task::done() const noexcept {
    return coro == nullptr || coro.done();
  }

}
