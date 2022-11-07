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

#include "event_awaiters.hh"
#include "evloop.hh"

namespace covent::uring {

  awaiter_sqe::awaiter_sqe(evloop& l)
    : loop(l) {
    /* nothing to do here */
  }

  bool awaiter_sqe::await_ready() {
    return false;
  }

  void awaiter_sqe::await_suspend() {
    setup_sqe(loop.create_sqe(this));
  }

  void awaiter_sqe::await_resume() {
    on_resume();
  }

  void awaiter_sqe::complete(res_t r, flags_t f) {
    res = r;
    flags = f;
    if (parent != nullptr && !parent.done())
      parent.resume();
  }


  awaiter_sqe_sleep::awaiter_sqe_sleep(evloop& l,
                                       std::chrono::nanoseconds&& ns)
    : awaiter_sqe(l) {
    auto secs = duration_cast<std::chrono::seconds>(ns);
    ts = {
      secs.count(),
      (ns - secs).count()
    };
  }

  void awaiter_sqe_sleep::setup_sqe(io_uring_sqe* sqe) {
    io_uring_prep_timeout(sqe, &ts, 1, 0);
  }

  void awaiter_sqe_sleep::on_resume() {
    /* nothing to do here */
  }

}
