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
#ifndef COVENT_CONCEPTS_HH
#define COVENT_CONCEPTS_HH

#include <coroutine>
#include <utility>

namespace covent::concepts {

  namespace detail {
    template<typename T>
    struct get_awaiter {
        using type = T;
    };

    template<typename T>
      requires requires(T&& a) {
        std::forward<T>(a).operator co_await();
      }
    struct get_awaiter<T> {
        using type = decltype(std::declval<T>().operator co_await());
    };

    template<typename T>
      requires requires(T&& a) {
        operator co_await(std::forward<T>(a));
        requires ! (requires {
          std::forward<T>(a).operator co_await();
        });
      }
    struct get_awaiter<T> {
        using type = decltype(operator co_await(std::declval<T>()));
    };

    template<typename T>
    using get_awaiter_t = typename get_awaiter<T>::type;
  }

  template<typename T>
  concept Awaitable = requires {
    typename detail::get_awaiter_t<T>;
    requires requires (detail::get_awaiter_t<T> a, std::coroutine_handle<> h) {
        { a.await_ready() } -> std::convertible_to<bool>;
        a.await_suspend(h);
        a.await_resume();
    };
  };

}

#endif
