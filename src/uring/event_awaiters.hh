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
#ifndef COVENT_URING_EVENT_AWAITERS_HH
#define COVENT_URING_EVENT_AWAITERS_HH

#include "../impl.hh"
#include "event_loop.hh"

namespace covent {

  class awaiter_sqe : public event_awaiter_impl {
    protected:
      event_loop_uring& loop;
      event_loop_uring::res_t res = 0;
      event_loop_uring::flags_t flags = 0;

    public:
      awaiter_sqe(event_loop_uring&);

      bool await_ready();
      void await_suspend();
      void await_resume();

      void complete(event_loop_uring::res_t, event_loop_uring::flags_t);

      virtual void setup_sqe(io_uring_sqe*) = 0;
      virtual void on_resume() = 0;
  };

  class awaiter_sqe_sleep : public awaiter_sqe {
    protected:
      __kernel_timespec ts;

    public:
      awaiter_sqe_sleep(event_loop_uring&, std::chrono::nanoseconds&&);

      void setup_sqe(io_uring_sqe*);
      void on_resume();
  };

}

#endif
