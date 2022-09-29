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
#ifndef COVENT_URING_EVENT_LOOP_HH
#define COVENT_URING_EVENT_LOOP_HH

#include <covent/base.hh>
#include <covent/event_loop.hh>
#include <liburing.h>

namespace covent {

  class awaiter_sqe;

  class event_loop_uring : public event_loop_impl_base {
    public:
      using res_t = __s32;
      using flags_t = __u32;

    private:
      io_uring ring = {};

    public:
      event_loop_uring(const event_loop_config&&);
      ~event_loop_uring();

      void run_once();
      io_uring_sqe* create_sqe(awaiter_sqe*);
      event_awaiter create_event_awaiter(std::chrono::nanoseconds&&);
  };

}

#endif
