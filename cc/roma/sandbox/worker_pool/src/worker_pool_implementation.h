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

#pragma once

#include <memory>
#include <vector>

#include "core/interface/service_interface.h"
#include "public/core/interface/execution_result.h"
#include "roma/sandbox/worker_api/src/worker_api.h"

#include "worker_pool_interface.h"

namespace google::scp::roma::sandbox::worker_pool {

template <typename WorkerApiT>
class WorkerPoolImplementation : public WorkerPool {
 public:
  explicit WorkerPoolImplementation(size_t size = 4) {
    size_ = size;
    for (auto i = 0; i < size_; i++) {
      workers_.push_back(std::make_shared<WorkerApiT>());
    }
  }

  core::ExecutionResult Init() noexcept override {
    for (auto& w : workers_) {
      auto result = w->Init();
      if (!result.Successful()) {
        return result;
      }
    }

    return core::SuccessExecutionResult();
  }

  core::ExecutionResult Run() noexcept override {
    for (auto& w : workers_) {
      auto result = w->Run();
      if (!result.Successful()) {
        return result;
      }
    }

    return core::SuccessExecutionResult();
  }

  core::ExecutionResult Stop() noexcept override {
    for (auto& w : workers_) {
      auto result = w->Stop();
      if (!result.Successful()) {
        return result;
      }
    }

    return core::SuccessExecutionResult();
  }

  size_t GetPoolSize() noexcept { return size_; }

  core::ExecutionResultOr<std::shared_ptr<worker_api::WorkerApi>> GetWoker(
      size_t index) noexcept {
    if (index >= size_) {
      return core::FailureExecutionResult(SC_UNKNOWN);
    }

    return workers_.at(index);
  }

 private:
  size_t size_;
  std::vector<std::shared_ptr<worker_api::WorkerApi>> workers_;
};
}  // namespace google::scp::roma::sandbox::worker_pool
