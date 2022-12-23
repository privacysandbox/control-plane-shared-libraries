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

#include "config_client_provider.h"

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "core/interface/async_context.h"
#include "cpio/client_providers/global_cpio/src/global_cpio.h"
#include "cpio/client_providers/interface/config_client_provider_interface.h"
#include "cpio/client_providers/interface/instance_client_provider_interface.h"
#include "cpio/client_providers/interface/type_def.h"
#include "cpio/proto/config_client.pb.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/interface/config_client/type_def.h"
#include "public/cpio/proto/parameter_service/v1/parameter_service.pb.h"

#include "error_codes.h"

using google::cmrt::sdk::parameter_service::v1::GetParameterRequest;
using google::cmrt::sdk::parameter_service::v1::GetParameterResponse;
using google::scp::core::AsyncContext;
using google::scp::core::ExecutionResult;
using google::scp::core::FailureExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::common::kZeroUuid;
using google::scp::core::errors::
    SC_CONFIG_CLIENT_PROVIDER_INVALID_PARAMETER_NAME;
using google::scp::core::errors::SC_CONFIG_CLIENT_PROVIDER_INVALID_TAG_NAME;
using google::scp::core::errors::SC_CONFIG_CLIENT_PROVIDER_PARAMETER_NOT_FOUND;
using google::scp::core::errors::SC_CONFIG_CLIENT_PROVIDER_TAG_NOT_FOUND;
using google::scp::cpio::config_client::GetInstanceIdProtoRequest;
using google::scp::cpio::config_client::GetInstanceIdProtoResponse;
using google::scp::cpio::config_client::GetTagProtoRequest;
using google::scp::cpio::config_client::GetTagProtoResponse;
using std::bind;
using std::make_shared;
using std::map;
using std::move;
using std::pair;
using std::promise;
using std::shared_ptr;
using std::string;
using std::vector;
using std::placeholders::_1;

/// Filename for logging errors
static constexpr char kConfigClientProvider[] = "ConfigClientProvider";

namespace google::scp::cpio::client_providers {
ConfigClientProvider::ConfigClientProvider(
    const shared_ptr<ConfigClientOptions>& config_client_options,
    const shared_ptr<InstanceClientProviderInterface>& instance_client_provider)
    : config_client_options_(config_client_options),
      instance_client_provider_(instance_client_provider) {
  parameter_client_provider_ = ParameterClientProviderFactory::Create(
      make_shared<ParameterClientOptions>(), instance_client_provider_);
}

ExecutionResult ConfigClientProvider::Init() noexcept {
  auto execution_result = parameter_client_provider_->Init();
  if (!execution_result.Successful()) {
    return execution_result;
  }

  return SuccessExecutionResult();
}

ExecutionResult ConfigClientProvider::Run() noexcept {
  auto execution_result = parameter_client_provider_->Run();
  if (!execution_result.Successful()) {
    ERROR(kConfigClientProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to run InstanceClientProvider.");
    return execution_result;
  }

  // Prefetches static metadata by issuing blocking calls. And only error out
  // when try to use them.
  fetch_instance_id_result_ =
      instance_client_provider_->GetCurrentInstanceId(instance_id_);
  if (!fetch_instance_id_result_.Successful()) {
    ERROR(kConfigClientProvider, kZeroUuid, kZeroUuid,
          fetch_instance_id_result_,
          "Failed getting AWS instance ID during initialization.");
  }

  execution_result = instance_client_provider_->GetTagsOfInstance(
      config_client_options_->tag_names, instance_id_, tag_values_map_);
  if (!execution_result.Successful()) {
    ERROR(kConfigClientProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed getting the tag values during initialization.");
    return execution_result;
  }

  // Prefetches static metadata by issuing blocking calls.
  execution_result = GetParameters(config_client_options_->parameter_names,
                                   parameter_values_map_);
  if (!execution_result.Successful()) {
    ERROR(kConfigClientProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed getting the AWS parameter values during initialization.");
    return execution_result;
  }

  return SuccessExecutionResult();
}

ExecutionResult ConfigClientProvider::Stop() noexcept {
  auto execution_result = parameter_client_provider_->Stop();
  if (!execution_result.Successful()) {
    ERROR(kConfigClientProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to stop InstanceClientProvider.");
    return execution_result;
  }

  return SuccessExecutionResult();
}

ExecutionResult ConfigClientProvider::GetParameter(
    AsyncContext<GetParameterRequest, GetParameterResponse>& context) noexcept {
  if (context.request->parameter_name().empty()) {
    auto execution_result = FailureExecutionResult(
        SC_CONFIG_CLIENT_PROVIDER_INVALID_PARAMETER_NAME);
    ERROR_CONTEXT(kConfigClientProvider, context, execution_result,
                  "Failed to get the parameter value for %s.",
                  context.request->parameter_name().c_str());
    context.result = execution_result;
    context.Finish();
    return execution_result;
  }

  auto response = make_shared<GetParameterResponse>();
  auto result = SuccessExecutionResult();

  auto it = parameter_values_map_.find(context.request->parameter_name());
  if (it == parameter_values_map_.end()) {
    result =
        FailureExecutionResult(SC_CONFIG_CLIENT_PROVIDER_PARAMETER_NOT_FOUND);
  } else {
    response->set_parameter_value(it->second);
  }

  context.response = move(response);
  context.result = result;
  context.Finish();

  return SuccessExecutionResult();
}

ExecutionResult ConfigClientProvider::GetParameters(
    const vector<string>& parameter_names,
    map<string, string>& parameter_values_map) noexcept {
  if (parameter_names.empty()) {
    return SuccessExecutionResult();
  }

  for (auto const& parameter_name : parameter_names) {
    promise<pair<ExecutionResult, shared_ptr<GetParameterResponse>>>
        request_promise;
    AsyncContext<GetParameterRequest, GetParameterResponse> context;
    context.request = make_shared<GetParameterRequest>();
    context.request->set_parameter_name(parameter_name);
    context.callback =
        [&](AsyncContext<GetParameterRequest, GetParameterResponse>& context) {
          request_promise.set_value({context.result, context.response});
        };

    auto execution_result = parameter_client_provider_->GetParameter(context);
    RETURN_IF_FAILURE(execution_result);

    auto result = request_promise.get_future().get();
    RETURN_IF_FAILURE(result.first);

    parameter_values_map[parameter_name] = result.second->parameter_value();
  }

  return SuccessExecutionResult();
}

ExecutionResult ConfigClientProvider::GetInstanceId(
    AsyncContext<GetInstanceIdProtoRequest, GetInstanceIdProtoResponse>&
        context) noexcept {
  auto response = make_shared<GetInstanceIdProtoResponse>();
  auto result = SuccessExecutionResult();
  if (instance_id_.empty()) {
    result = fetch_instance_id_result_;
  } else {
    response->set_instance_id(instance_id_);
  }

  context.response = move(response);
  context.result = result;
  context.Finish();

  return SuccessExecutionResult();
}

ExecutionResult ConfigClientProvider::GetTag(
    AsyncContext<GetTagProtoRequest, GetTagProtoResponse>& context) noexcept {
  if (context.request->tag_name().empty()) {
    auto execution_result =
        FailureExecutionResult(SC_CONFIG_CLIENT_PROVIDER_INVALID_TAG_NAME);
    ERROR_CONTEXT(kConfigClientProvider, context, execution_result,
                  "Failed to get tag with empty tag name.");
    context.result = execution_result;
    context.Finish();
    return execution_result;
  }

  auto response = make_shared<GetTagProtoResponse>();
  auto execution_result = SuccessExecutionResult();

  auto it = tag_values_map_.find(context.request->tag_name());
  if (it == tag_values_map_.end()) {
    execution_result =
        FailureExecutionResult(SC_CONFIG_CLIENT_PROVIDER_TAG_NOT_FOUND);
    ERROR_CONTEXT(kConfigClientProvider, context, execution_result,
                  "Failed to get the tag value for %s.",
                  context.request->tag_name().c_str());
  } else {
    response->set_value(it->second);
  }

  context.response = move(response);
  context.result = execution_result;
  context.Finish();

  return SuccessExecutionResult();
}

#ifndef TEST_CPIO
shared_ptr<ConfigClientProviderInterface> ConfigClientProviderFactory::Create(
    const shared_ptr<ConfigClientOptions>& options) {
  return make_shared<ConfigClientProvider>(
      options, GlobalCpio::GetGlobalCpio()->GetInstanceClientProvider());
}
#endif
}  // namespace google::scp::cpio::client_providers
