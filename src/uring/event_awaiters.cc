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
#include "event_loop.hh"

covent::awaiter_sqe::awaiter_sqe(event_loop_uring& l)
  : loop(l) {
  /* nothing to do here */
}

bool covent::awaiter_sqe::await_ready() {
  return false;
}

void covent::awaiter_sqe::await_suspend() {
  setup_sqe(loop.create_sqe(this));
}

void covent::awaiter_sqe::await_resume() {
  on_resume();
}

void covent::awaiter_sqe::complete(event_loop_uring::res_t r,
                                   event_loop_uring::flags_t f) {
  res = r;
  flags = f;
  parent.resume();
}


covent::awaiter_sqe_sleep::awaiter_sqe_sleep(event_loop_uring& l,
                                             std::chrono::nanoseconds&& ns)
  : awaiter_sqe(l) {
  auto secs = duration_cast<std::chrono::seconds>(ns);
  ts = {
    secs.count(),
    (ns - secs).count()
  };

}

void covent::awaiter_sqe_sleep::setup_sqe(io_uring_sqe* sqe) {
  io_uring_prep_timeout(sqe, &ts, 1, 0);
}

void covent::awaiter_sqe_sleep::on_resume() {
  /* nothing to do here */
}
