/*
 * Copyright 2023 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dispatcher.h"

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "roma/sandbox/constants/constants.h"

using absl::StatusOr;
using google::scp::core::ExecutionResult;
using google::scp::core::ExecutionResultOr;
using google::scp::core::SuccessExecutionResult;
using std::atomic;
using std::make_shared;
using std::make_unique;
using std::move;
using std::unique_ptr;
using std::vector;
using std::chrono::milliseconds;
using std::this_thread::sleep_for;

namespace google::scp::roma::sandbox::dispatcher {
ExecutionResult Dispatcher::Init() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult Dispatcher::Run() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult Dispatcher::Stop() noexcept {
  allow_dispatch_.store(false);
  return SuccessExecutionResult();
}

ExecutionResult Dispatcher::Broadcast(unique_ptr<CodeObject> code_object,
                                      Callback broadcast_callback) noexcept {
  allow_dispatch_.store(false);
  // We wait until there are no requests running to broadcast new code to all
  // the workers
  while (pending_requests_.load() > 0) {
    sleep_for(milliseconds(5));
  }

  // We set the worker index to zero to make sure we get to all the workers in
  // the dispatch call
  worker_index_.store(0);

  auto worker_count = worker_pool_->GetPoolSize();
  auto finished_counter = make_shared<atomic<size_t>>(0);
  auto responses_storage =
      make_shared<vector<unique_ptr<StatusOr<ResponseObject>>>>(worker_count);

  for (size_t worker_index = 0; worker_index < worker_count; worker_index++) {
    auto callback =
        [worker_count, responses_storage, finished_counter, broadcast_callback,
         worker_index](unique_ptr<StatusOr<ResponseObject>> response) {
          auto& all_resp = *responses_storage;
          // Store responses in the vector
          all_resp[worker_index].swap(response);
          auto finished_value = finished_counter->fetch_add(1);
          // Go through the responses and call the callback on the first failed
          // one. If all succeeded, call the first callback.
          if (finished_value + 1 == worker_count) {
            for (auto& resp : all_resp) {
              if (!resp->ok()) {
                broadcast_callback(::std::move(resp));
                return;
              }
            }
            broadcast_callback(::std::move(all_resp[0]));
          }
        };

    auto code_object_copy = make_unique<CodeObject>(*code_object);

    auto dispatch_result = InternalDispatch(move(code_object_copy), callback);

    if (!dispatch_result.Successful()) {
      allow_dispatch_.store(true);
      return dispatch_result;
    }
  }

  allow_dispatch_.store(true);
  return SuccessExecutionResult();
}
}  // namespace google::scp::roma::sandbox::dispatcher
