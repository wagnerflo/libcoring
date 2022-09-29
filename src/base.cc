/// https://github.com/lewissbaker/cppcoro/blob/master/include/cppcoro/task.hpp
/// https://github.com/CarterLi/liburing4cpp/blob/async/include/liburing/task.hpp


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
#include "uring/event_loop.hh"

covent::event_awaiter::event_awaiter(event_awaiter_impl* i)
  : impl(i) {
  /* nothing to do here */
}

covent::event_awaiter::~event_awaiter() {
  delete impl;
}

bool covent::event_awaiter::await_ready() {
  return impl->await_ready();
}

void covent::event_awaiter::await_suspend(std::coroutine_handle<> c) {
  impl->parent = c;
  impl->await_suspend();
}

void covent::event_awaiter::await_resume() {
  impl->await_resume();
}


thread_local covent::event_loop_impl_base* active_loop = nullptr;

void covent::set_active_loop(event_loop_impl_base* loop) {
  if (loop != nullptr && active_loop != nullptr)
    throw std::runtime_error("Running nested loops is not supported.");
  active_loop = loop;
}

covent::event_loop_impl_base& covent::get_active_loop() {
  return *active_loop;
}


template<>
covent::event_loop::event_loop<covent::event_loop_uring>(const event_loop_config&& conf)
  : impl(new event_loop_uring(std::move(conf))) {
  /* nothing to to here */
}

covent::event_loop& covent::get_event_loop(const event_loop_config&& conf) {
  thread_local covent::event_loop loop(std::move(conf));
  return loop;
}
