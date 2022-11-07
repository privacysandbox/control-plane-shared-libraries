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
#include <string>

#include "core/interface/async_context.h"
#include "core/interface/message_router_interface.h"
#include "core/message_router/src/message_router.h"
#include "cpio/client_providers/interface/config_client_provider_interface.h"
#include "cpio/client_providers/interface/instance_client_provider_interface.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"

namespace google::scp::cpio::client_providers {
/*! @copydoc ConfigClientProviderInterface
 */
class ConfigClientProvider : public ConfigClientProviderInterface {
 public:
  virtual ~ConfigClientProvider() = default;

  explicit ConfigClientProvider(
      const std::shared_ptr<ConfigClientOptions>& config_client_options,
      const std::shared_ptr<InstanceClientProviderInterface>&
          instance_client_provider,
      const std::shared_ptr<core::MessageRouterInterface<
          google::protobuf::Any, google::protobuf::Any>>& message_router)
      : config_client_options_(config_client_options),
        instance_client_provider_(instance_client_provider),
        message_router_(message_router) {}

  core::ExecutionResult Init() noexcept override;

  core::ExecutionResult Run() noexcept override;

  core::ExecutionResult Stop() noexcept override;

  core::ExecutionResult GetEnvironmentName(
      core::AsyncContext<config_client::GetEnvironmentNameProtoRequest,
                         config_client::GetEnvironmentNameProtoResponse>&
          context) noexcept override;

  core::ExecutionResult GetInstanceId(
      core::AsyncContext<config_client::GetInstanceIdProtoRequest,
                         config_client::GetInstanceIdProtoResponse>&
          context) noexcept override;

 protected:
  /**
   * @brief Triggered when GetEnvironmentNameProtoRequest arrives.
   *
   * @param context async execution context.
   */
  virtual void OnGetEnvironmentName(
      core::AsyncContext<google::protobuf::Any, google::protobuf::Any>
          context) noexcept;

  /**
   * @brief Triggered when GetInstanceIdProtoRequest arrives.
   *
   * @param context async execution context.
   */
  virtual void OnGetInstanceId(
      core::AsyncContext<google::protobuf::Any, google::protobuf::Any>
          context) noexcept;

  /**
   * @brief Triggered when GetParameterProtoRequest arrives.
   *
   * @param context async execution context.
   */
  virtual void OnGetParameter(
      core::AsyncContext<google::protobuf::Any, google::protobuf::Any>
          context) noexcept;

  /// Configurations for ConfigClient.
  std::shared_ptr<ConfigClientOptions> config_client_options_;

  /// InstanceClientProvider.
  std::shared_ptr<InstanceClientProviderInterface> instance_client_provider_;

  /// The message router where the config client subscribes actions.
  std::shared_ptr<core::MessageRouterInterface<google::protobuf::Any,
                                               google::protobuf::Any>>
      message_router_;

  /// The instance ID pre-fetched during initializing.
  std::string instance_id_;

  /// The environment name pre-fetched during initializing.
  std::string environment_name_;

  /// The result of fetching environment name.
  core::ExecutionResult fetch_environment_name_result_;

  /// The result of fetching instance ID.
  core::ExecutionResult fetch_instance_id_result_;
};
}  // namespace google::scp::cpio::client_providers
