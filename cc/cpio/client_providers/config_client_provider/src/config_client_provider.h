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

#include "core/interface/async_context.h"
#include "cpio/client_providers/interface/config_client_provider_interface.h"
#include "cpio/client_providers/interface/instance_client_provider_interface.h"
#include "cpio/client_providers/interface/parameter_client_provider_interface.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/proto/parameter_service/v1/parameter_service.pb.h"

namespace google::scp::cpio::client_providers {
/*! @copydoc ConfigClientProviderInterface
 */
class ConfigClientProvider : public ConfigClientProviderInterface {
 public:
  virtual ~ConfigClientProvider() = default;

  explicit ConfigClientProvider(
      const std::shared_ptr<ConfigClientOptions>& config_client_options,
      const std::shared_ptr<InstanceClientProviderInterface>&
          instance_client_provider);

  core::ExecutionResult Init() noexcept override;

  core::ExecutionResult Run() noexcept override;

  core::ExecutionResult Stop() noexcept override;

  core::ExecutionResult GetTag(
      core::AsyncContext<config_client::GetTagProtoRequest,
                         config_client::GetTagProtoResponse>& context) noexcept
      override;

  core::ExecutionResult GetInstanceId(
      core::AsyncContext<config_client::GetInstanceIdProtoRequest,
                         config_client::GetInstanceIdProtoResponse>&
          context) noexcept override;

  core::ExecutionResult GetParameter(
      core::AsyncContext<
          cmrt::sdk::parameter_service::v1::GetParameterRequest,
          cmrt::sdk::parameter_service::v1::GetParameterResponse>&
          context) noexcept override;

 protected:
  /**
   * @brief Fetches the Parameter values for the given parameter names.
   *
   * @param parameter_names the given parameter names.
   * @param parameter_values_map returned parameter values.
   * @return core::ExecutionResult the fetch results.
   */
  core::ExecutionResult GetParameters(
      const std::vector<std::string>& parameter_names,
      std::map<std::string, std::string>& parameter_values_map) noexcept;

  /// Configurations for ConfigClient.
  std::shared_ptr<ConfigClientOptions> config_client_options_;

  /// InstanceClientProvider.
  std::shared_ptr<InstanceClientProviderInterface> instance_client_provider_;

  /// ParameterClientProvider.
  std::shared_ptr<ParameterClientProviderInterface> parameter_client_provider_;

  /// The instance ID pre-fetched during initializing.
  std::string instance_id_;

  /// The tag values prefetched during initializing. The key is tag
  /// name and the value is tag value.
  std::map<std::string, std::string> tag_values_map_;

  /// The result of fetching instance ID.
  core::ExecutionResult fetch_instance_id_result_;

  /// The parameter values prefetched during initializing. The key is parameter
  /// name and the value is parameter value.
  std::map<std::string, std::string> parameter_values_map_;
};
}  // namespace google::scp::cpio::client_providers
