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

#include "core/async_executor/src/async_executor.h"
#include "core/interface/async_executor_interface.h"
#include "core/interface/http_client_interface.h"
#include "core/interface/message_router_interface.h"
#include "core/message_router/src/message_router.h"
#include "cpio/client_providers/interface/cpio_provider_interface.h"
#include "cpio/client_providers/interface/instance_client_provider_interface.h"
#include "cpio/client_providers/interface/role_credentials_provider_interface.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"

namespace google::scp::cpio::client_providers {
/*! @copydoc CpioProviderInterface
 * Provides global objects for native library mode.
 */
class LibCpioProvider : public CpioProviderInterface {
 public:
  LibCpioProvider();

  core::ExecutionResult Init() noexcept override;

  core::ExecutionResult Run() noexcept override;

  core::ExecutionResult Stop() noexcept override;

  std::shared_ptr<core::MessageRouterInterface<google::protobuf::Any,
                                               google::protobuf::Any>>
  GetMessageRouter() noexcept override;

  core::ExecutionResult GetAsyncExecutor(
      std::shared_ptr<core::AsyncExecutorInterface>& async_executor) noexcept
      override;

  core::ExecutionResult GetHttpClient(
      std::shared_ptr<core::HttpClientInterface>& http_client) noexcept
      override;

  std::shared_ptr<InstanceClientProviderInterface>
  GetInstanceClientProvider() noexcept override;

  core::ExecutionResult GetRoleCredentialsProvider(
      std::shared_ptr<RoleCredentialsProviderInterface>&
          role_credentials_provider) noexcept override;

 protected:
  /// Global message router.
  std::shared_ptr<core::MessageRouterInterface<google::protobuf::Any,
                                               google::protobuf::Any>>
      message_router_;

  /// Global async executor.
  std::shared_ptr<core::AsyncExecutorInterface> async_executor_;
  /// Global http client.
  std::shared_ptr<core::HttpClientInterface> http_client_;
  /// Global instance client provider to fetch cloud metadata.
  std::shared_ptr<InstanceClientProviderInterface> instance_client_provider_;
  /// Global role credential provider.
  std::shared_ptr<RoleCredentialsProviderInterface> role_credentials_provider_;
};
}  // namespace google::scp::cpio::client_providers
