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
#include <covent/evloops.hh>
#include <covent/task.hh>

#include <any>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace covent {

  class event_loop_config {
    protected:
      std::map<std::string, std::any> values;

    public:
      event_loop_config(std::initializer_list<decltype(values)::value_type>&& v)
        : values(std::forward<decltype(v)>(v)) {
        /* nothing to do here */
      }

      template<typename T, typename C = T>
        requires std::is_convertible_v<T, C>
      T get(std::string&& key, T&& def) const {
        if (auto item = values.find(key); item != values.end())
          return static_cast<C>(std::any_cast<T>(item->second));
        return def;
      }
  };

  class event_loop {
    protected:
      detail::evloop_base* impl;

    public:
      template<typename ImplType = DefaultEventLoopType>
      event_loop(const event_loop_config&& conf = {});

      // not copyable
      event_loop(const event_loop&) = delete;
      event_loop& operator=(const event_loop&) = delete;

      ~event_loop() {
        delete impl;
      }

      template<typename Func, typename ...Args>
      decltype(auto) run(Func const& func, Args&& ...args) {
        using Task = std::result_of_t<Func(Args...)>;
        detail::set_active_loop(impl);
        auto tsk = func(std::forward<Args>(args)...);
        auto entry = [&tsk]() -> task<void, std::suspend_never> {
          co_await tsk;
        }();
        while (!entry.done())
          impl->run_once();
        detail::set_active_loop(nullptr);
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
