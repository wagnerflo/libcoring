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

#include "impl.hh"

namespace covent::detail {

  event_awaiter::event_awaiter(event_awaiter_impl* i)
    : impl(i) {
    /* nothing to do here */
  }

  event_awaiter::~event_awaiter() {
    delete impl;
  }

  bool event_awaiter::await_ready() {
    return impl->await_ready();
  }

  void event_awaiter::await_suspend(std::coroutine_handle<> c) {
    impl->parent = c;
    impl->await_suspend();
  }

  void event_awaiter::await_resume() {
    impl->await_resume();
  }


  thread_local evloop_base* active_loop = nullptr;

  void set_active_loop(evloop_base* loop) {
    if (loop != nullptr && active_loop != nullptr)
      throw std::runtime_error("Running nested loops is not supported.");
    active_loop = loop;
  }

  evloop_base& get_active_loop() {
    return *active_loop;
  }

}

namespace covent {

  event_loop& get_event_loop(const event_loop_config&& conf) {
    thread_local event_loop loop(std::move(conf));
    return loop;
  }

}
