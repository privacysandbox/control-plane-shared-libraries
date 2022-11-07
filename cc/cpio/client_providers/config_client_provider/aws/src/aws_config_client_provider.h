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

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <aws/ssm/SSMClient.h>

#include "core/interface/async_context.h"
#include "core/interface/message_router_interface.h"
#include "cpio/client_providers/config_client_provider/src/config_client_provider.h"
#include "cpio/proto/config_client.pb.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"

#include "error_codes.h"

namespace google::scp::cpio::client_providers {
/*! @copydoc ConfigClientInterface
 */
class AwsConfigClientProvider : public ConfigClientProvider {
 public:
  /**
   * @brief Constructs a new Aws Config Client Provider object
   *
   * @param config_client_options configurations for ConfigClient.
   * @param message_router message router to subscribe messages.
   */
  AwsConfigClientProvider(
      const std::shared_ptr<ConfigClientOptions>& config_client_options,
      const std::shared_ptr<InstanceClientProviderInterface>&
          instance_client_provider,
      const std::shared_ptr<core::MessageRouterInterface<
          google::protobuf::Any, google::protobuf::Any>>& message_router)
      : ConfigClientProvider(config_client_options, instance_client_provider,
                             message_router) {}

  core::ExecutionResult Init() noexcept override;

  core::ExecutionResult Run() noexcept override;

  core::ExecutionResult Stop() noexcept override;

  core::ExecutionResult GetParameter(
      core::AsyncContext<config_client::GetParameterProtoRequest,
                         config_client::GetParameterProtoResponse>&
          context) noexcept override;

 protected:
  /**
   * @brief Fetches the Parameter values for the given parameter names.
   *
   * @param parameter_values_map returned parameter values.
   * @param parameter_names the given parameter names.
   * @return core::ExecutionResult the fetch results.
   */
  core::ExecutionResult GetParameters(
      std::map<std::string, std::string>& parameter_values_map,
      const std::vector<std::string>& parameter_names) noexcept;

  /**
   * @brief Creates a Client Configuration object.
   *
   * @param client_config returned Client Configuration.
   * @return core::ExecutionResult creation result.
   */
  virtual core::ExecutionResult CreateClientConfiguration(
      std::shared_ptr<Aws::Client::ClientConfiguration>&
          client_config) noexcept;

  /// SSMClient.
  std::shared_ptr<Aws::SSM::SSMClient> ssm_client_;

  /// The parameter values prefetched during initializing. The key is parameter
  /// name and the value is parameter value.
  std::map<std::string, std::string> parameter_values_map_;
};
}  // namespace google::scp::cpio::client_providers
