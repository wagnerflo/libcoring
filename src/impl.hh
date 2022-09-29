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
#ifndef COVENT_IMPL_HH
#define COVENT_IMPL_HH

#include <covent/base.hh>

namespace covent {

  class event_awaiter_impl {
    friend class event_awaiter;

    protected:
      std::coroutine_handle<> parent = nullptr;

    public:
      virtual bool await_ready() = 0;
      virtual void await_suspend() = 0;
      virtual void await_resume() = 0;
  };

}

#endif
