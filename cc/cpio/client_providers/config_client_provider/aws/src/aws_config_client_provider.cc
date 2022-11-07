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

#include "aws_config_client_provider.h"

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <aws/core/utils/Outcome.h>
#include <aws/ssm/SSMClient.h>
#include <aws/ssm/model/GetParametersRequest.h>

#include "cc/core/common/uuid/src/uuid.h"
#include "core/interface/async_context.h"
#include "cpio/client_providers/global_cpio/src/global_cpio.h"
#include "cpio/common/aws/src/aws_utils.h"
#include "cpio/proto/config_client.pb.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"

#include "error_codes.h"
#include "ssm_error_converter.h"

using Aws::Client::ClientConfiguration;
using Aws::SSM::SSMClient;
using Aws::SSM::Model::GetParametersOutcome;
using Aws::SSM::Model::GetParametersRequest;
using google::scp::core::AsyncContext;
using google::scp::core::ExecutionResult;
using google::scp::core::FailureExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::common::kZeroUuid;
using google::scp::core::errors::
    SC_AWS_CONFIG_CLIENT_PROVIDER_NOT_ALL_PARAMETERS_FOUND;
using google::scp::core::errors::
    SC_AWS_CONFIG_CLIENT_PROVIDER_PARAMETER_NOT_FOUND;
using google::scp::cpio::common::CreateClientConfiguration;
using google::scp::cpio::config_client::GetParameterProtoRequest;
using google::scp::cpio::config_client::GetParameterProtoResponse;
using std::make_shared;
using std::map;
using std::move;
using std::shared_ptr;
using std::string;
using std::vector;

/// Filename for logging errors
static constexpr char kAwsConfigClientProvider[] = "AwsConfigClientProvider";

namespace google::scp::cpio::client_providers {
ExecutionResult AwsConfigClientProvider::CreateClientConfiguration(
    shared_ptr<ClientConfiguration>& client_config) noexcept {
  auto region = make_shared<string>();
  auto execution_result = instance_client_provider_->GetRegion(*region);
  if (!execution_result.Successful()) {
    ERROR(kAwsConfigClientProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to get region");
    return execution_result;
  }

  client_config = common::CreateClientConfiguration(region);
  return SuccessExecutionResult();
}

ExecutionResult AwsConfigClientProvider::Init() noexcept {
  auto execution_result = ConfigClientProvider::Init();
  if (!execution_result.Successful()) {
    ERROR(kAwsConfigClientProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to initialize ConfigClientProvider");
    return execution_result;
  }

  shared_ptr<ClientConfiguration> client_config;
  execution_result = CreateClientConfiguration(client_config);
  if (!execution_result.Successful()) {
    ERROR(kAwsConfigClientProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to create ClientConfiguration");
    return execution_result;
  }

  ssm_client_ = make_shared<SSMClient>(*client_config);

  return SuccessExecutionResult();
}

ExecutionResult AwsConfigClientProvider::Run() noexcept {
  auto execution_result = ConfigClientProvider::Run();
  if (!execution_result.Successful()) {
    ERROR(kAwsConfigClientProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed running ConfigClientProvider.");
    return execution_result;
  }

  // Prefetches static metadata by issuing blocking calls.
  execution_result = GetParameters(parameter_values_map_,
                                   config_client_options_->parameter_names);
  if (!execution_result.Successful()) {
    ERROR(kAwsConfigClientProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed getting the AWS parameter values during initialization.");
    return execution_result;
  }

  return SuccessExecutionResult();
}

ExecutionResult AwsConfigClientProvider::Stop() noexcept {
  return ConfigClientProvider::Stop();
}

ExecutionResult AwsConfigClientProvider::GetParameter(
    AsyncContext<GetParameterProtoRequest, GetParameterProtoResponse>&
        context) noexcept {
  auto response = make_shared<GetParameterProtoResponse>();
  auto result = SuccessExecutionResult();

  auto it = parameter_values_map_.find(context.request->parameter_name());
  if (it == parameter_values_map_.end()) {
    result = FailureExecutionResult(
        SC_AWS_CONFIG_CLIENT_PROVIDER_PARAMETER_NOT_FOUND);
  } else {
    response->set_value(it->second);
  }

  context.response = move(response);
  context.result = result;
  context.Finish();

  return SuccessExecutionResult();
}

ExecutionResult AwsConfigClientProvider::GetParameters(
    map<string, string>& parameter_values_map,
    const vector<string>& parameter_names) noexcept {
  if (parameter_names.empty()) {
    return SuccessExecutionResult();
  }

  GetParametersRequest request;

  for (auto const& parameter_name : parameter_names)
    request.AddNames(parameter_name.c_str());

  auto outcome = ssm_client_->GetParameters(request);
  if (!outcome.IsSuccess()) {
    auto error_type = outcome.GetError().GetErrorType();
    return SSMErrorConverter::ConvertSSMError(error_type);
  }

  if (outcome.GetResult().GetParameters().size() != parameter_names.size()) {
    return FailureExecutionResult(
        SC_AWS_CONFIG_CLIENT_PROVIDER_NOT_ALL_PARAMETERS_FOUND);
  }

  for (auto parameter : outcome.GetResult().GetParameters()) {
    parameter_values_map.emplace(parameter.GetName(),
                                 string(parameter.GetValue().c_str()));
  }
  return SuccessExecutionResult();
}

#ifndef CPIO_TESTING
std::shared_ptr<ConfigClientProviderInterface>
ConfigClientProviderFactory::Create(
    const std::shared_ptr<ConfigClientOptions>& options) {
  return make_shared<AwsConfigClientProvider>(
      options, GlobalCpio::GetGlobalCpio()->GetInstanceClientProvider(),
      GlobalCpio::GetGlobalCpio()->GetMessageRouter());
}
#endif
}  // namespace google::scp::cpio::client_providers
