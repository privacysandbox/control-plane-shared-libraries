/*
 * Copyright 2022 Google LLC
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

#include "core/interface/async_executor_interface.h"
#include "core/interface/message_router_interface.h"
#include "core/interface/service_interface.h"
#include "core/message_router/src/message_router.h"
#include "cpio/client_providers/interface/instance_client_provider_interface.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"

namespace google::scp::cpio::client_providers {
/**
 * @brief Provides all required global objects.
 *
 */
class CpioProviderInterface : public core::ServiceInterface {
 public:
  /**
   * @brief Gets the Message Router object.
   *
   * @return std::shared_ptr<core::MessageRouterInterface<google::protobuf::Any,
   * google::protobuf::Any>> message router object.
   */
  virtual std::shared_ptr<core::MessageRouterInterface<google::protobuf::Any,
                                                       google::protobuf::Any>>
  GetMessageRouter() noexcept = 0;

  /**
   * @brief Gets the global Cpu Async Executor. Only create it when it is
   * needed.
   *
   * @param cpu_async_executor the Async Executor for cpu-bound tasks.
   * @return core::ExecutionResult get result.
   */
  virtual core::ExecutionResult GetCpuAsyncExecutor(
      std::shared_ptr<core::AsyncExecutorInterface>&
          cpu_async_executor) noexcept = 0;

  /**
   * @brief Gets the InstanceClientProvider.
   *
   * @return std::shared_ptr<InstanceClientProviderInterface> the
   * InstanceClientProvider.
   */
  virtual std::shared_ptr<InstanceClientProviderInterface>
  GetInstanceClientProvider() noexcept = 0;
};

/// Factory to create CpioProvider.
class CpioProviderFactory {
 public:
  /**
   * @brief Creats CpioProvider.
   *
   * @return std::unique_ptr<CpioProviderInterface> CpioProvider.
   */
  static std::unique_ptr<CpioProviderInterface> Create();
};
}  // namespace google::scp::cpio::client_providers
