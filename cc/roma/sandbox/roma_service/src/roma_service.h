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

#include "core/async_executor/src/async_executor.h"
#include "core/interface/service_interface.h"
#include "public/core/interface/execution_result.h"
#include "roma/sandbox/dispatcher/src/dispatcher.h"
#include "roma/sandbox/worker_pool/src/worker_pool_interface.h"

namespace google::scp::roma::sandbox::roma_service {
class RomaService : public core::ServiceInterface {
 public:
  core::ExecutionResult Init() noexcept override;
  core::ExecutionResult Run() noexcept override;
  core::ExecutionResult Stop() noexcept override;

  /// The the instance of the roma service. This function is thread-unsafe when
  /// instance_ is null.
  static RomaService* Instance(const Config& config = Config()) {
    if (instance_ == nullptr) {
      instance_ = new RomaService(config);
    }
    return instance_;
  }

  static void Delete() {
    if (instance_ != nullptr) {
      delete instance_;
    }
    instance_ = nullptr;
  }

  /// Return the dispatcher
  dispatcher::Dispatcher& Dispatcher() { return *dispatcher_; }

  RomaService(const RomaService&) = delete;
  RomaService() = default;

 private:
  explicit RomaService(const Config& config = Config()) { config_ = config; }

  static RomaService* instance_;
  Config config_;
  std::shared_ptr<dispatcher::Dispatcher> dispatcher_;
  std::shared_ptr<worker_pool::WorkerPool> worker_pool_;
  std::shared_ptr<core::AsyncExecutor> async_executor_;
};
}  // namespace google::scp::roma::sandbox::roma_service
