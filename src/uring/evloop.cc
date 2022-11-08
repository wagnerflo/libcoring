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

#include "awaiters.hh"
#include "evloop.hh"

namespace covent {

  // explictly instantiate constructor for event loop implementation
  template<>
  event_loop::event_loop<evloop_uring>(const event_loop_config&& conf)
    : impl(new evloop_uring(std::move(conf))) {
    /* nothing to to here */
  }

}

namespace covent::uring {

  using covent::detail::event_awaiter;

  evloop::evloop(const event_loop_config&& conf) {
    io_uring_params params = {};
    io_uring_queue_init_params(
      conf.get<int, uint32_t>("entries", 256),
      &ring, &params
    );
  }

  evloop::~evloop() {
    io_uring_queue_exit(&ring);
  }

  event_awaiter evloop::create_event_awaiter(std::chrono::nanoseconds&& ns) {
    return { new awaiter_sqe_sleep(*this, std::move(ns)) };
  }

  inline void handle_cqe(io_uring* ring, io_uring_cqe* cqe) {
    auto aw = static_cast<awaiter_sqe*>(io_uring_cqe_get_data(cqe));
    aw->complete(cqe->res, cqe->flags);
    io_uring_cqe_seen(ring, cqe);
  }

  void evloop::run_once() {
    io_uring_submit(&ring);
    io_uring_cqe* cqe;

    // handle all completed without waiting
    while (!io_uring_peek_cqe(&ring, &cqe))
      handle_cqe(&ring, cqe);

    // then actually wait for one
    io_uring_wait_cqe(&ring, &cqe);
    handle_cqe(&ring, cqe);
  }

  io_uring_sqe* evloop::create_sqe(awaiter_sqe* aw) {
    io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    if (sqe == nullptr) {
      // handle overflow
    }
    else {
      io_uring_sqe_set_data(sqe, aw);
    }
    return sqe;
  }

}
