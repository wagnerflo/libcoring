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
#ifndef COVENT_EVENT_LOOP_HH
#define COVENT_EVENT_LOOP_HH

#include <covent/base.hh>
#include <covent/task.hh>

#include <any>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace covent {

  using event_loop_config = std::map<std::string, std::any>;

  template<typename T>
  T get_event_loop_config(const event_loop_config& conf, std::string&& key, T&& def) {
    if (auto item = conf.find(key); item != conf.end())
      return std::any_cast<T>(item);
    return def;
  }

  class event_loop {
    protected:
      event_loop_impl_base* impl;

    public:
      template<typename ImplType = event_loop_uring>
      event_loop(const event_loop_config&& = {});

      // not copyable
      event_loop(const event_loop&) = delete;
      event_loop& operator=(const event_loop&) = delete;

      ~event_loop() {
        delete impl;
      }

      template<typename Func, typename ...Args>
      decltype(auto) run(Func const& func, Args&& ...args) {
        using Task = std::result_of_t<Func(Args...)>;
        using Result = Task::result_type;
        set_active_loop(impl);
        auto tsk = [&]() -> entry_task<Result> {
          co_return co_await func(std::forward<Args>(args)...);
        }();
        while (!tsk.done())
          impl->run_once();
        set_active_loop(nullptr);
        if constexpr(!Task::promise_type::ResultIsVoid)
          return tsk.result();
      }
  };

  event_loop& get_event_loop(const event_loop_config&& = {});

  template<typename Func, typename ...Args>
  decltype(auto) run(Func const& func, Args&& ...args) {
    return get_event_loop().run(func, std::forward<Args>(args)...);
  }
}

#endif
