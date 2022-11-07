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

#include "cpio/client_providers/interface/config_client_provider_interface.h"
#include "cpio/proto/config_client.pb.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/interface/config_client/config_client_interface.h"

#include "error_codes.h"

namespace google::scp::cpio {
/*! @copydoc ConfigClientInterface
 */
class ConfigClient : public ConfigClientInterface {
 public:
  explicit ConfigClient(const std::shared_ptr<ConfigClientOptions>& options);

  core::ExecutionResult Init() noexcept override;

  core::ExecutionResult Run() noexcept override;

  core::ExecutionResult Stop() noexcept override;

  core::ExecutionResult GetParameter(
      GetParameterRequest request,
      Callback<GetParameterResponse> callback) noexcept override;

  core::ExecutionResult GetEnvironment(
      GetEnvironmentRequest request,
      Callback<GetEnvironmentResponse> callback) noexcept override;

  core::ExecutionResult GetInstanceId(
      GetInstanceIdRequest request,
      Callback<GetInstanceIdResponse> callback) noexcept override;

 protected:
  /**
   * @brief Callback when GetParameter result returned.
   *
   * @param request caller's request.
   * @param callback caller's callback
   * @param get_parameter_context execution context.
   */
  void OnGetParameterCallback(
      const GetParameterRequest& request,
      Callback<GetParameterResponse>& callback,
      core::AsyncContext<config_client::GetParameterProtoRequest,
                         config_client::GetParameterProtoResponse>&
          get_parameter_context) noexcept;

  /**
   * @brief Callback when GetEnvironmentName result returned.
   *
   * @param request caller's request.
   * @param callback caller's callback
   * @param get_environment_context execution context.
   */
  void OnGetEnvironmentCallback(
      const GetEnvironmentRequest& request,
      Callback<GetEnvironmentResponse>& callback,
      core::AsyncContext<config_client::GetEnvironmentNameProtoRequest,
                         config_client::GetEnvironmentNameProtoResponse>&
          get_environment_context) noexcept;

  /**
   * @brief Callback when GetInstanceId result returned.
   *
   * @param request caller's request.
   * @param callback caller's callback
   * @param get_instance_context execution context.
   */
  void OnGetInstanceIdCallback(
      const GetInstanceIdRequest& request,
      Callback<GetInstanceIdResponse>& callback,
      core::AsyncContext<config_client::GetInstanceIdProtoRequest,
                         config_client::GetInstanceIdProtoResponse>&
          get_instance_context) noexcept;

  std::shared_ptr<client_providers::ConfigClientProviderInterface>
      config_client_provider_;
};
}  // namespace google::scp::cpio
