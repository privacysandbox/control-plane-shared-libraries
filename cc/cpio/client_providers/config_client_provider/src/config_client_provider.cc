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
#include <memory>
#include <utility>

#include "core/interface/async_context.h"
#include "core/interface/message_router_interface.h"
#include "cpio/client_providers/interface/config_client_provider_interface.h"
#include "cpio/client_providers/interface/instance_client_provider_interface.h"
#include "cpio/client_providers/interface/type_def.h"
#include "cpio/proto/config_client.pb.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/interface/config_client/type_def.h"

using google::protobuf::Any;
using google::scp::core::AsyncContext;
using google::scp::core::ExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::common::kZeroUuid;
using google::scp::cpio::config_client::GetEnvironmentNameProtoRequest;
using google::scp::cpio::config_client::GetEnvironmentNameProtoResponse;
using google::scp::cpio::config_client::GetInstanceIdProtoRequest;
using google::scp::cpio::config_client::GetInstanceIdProtoResponse;
using google::scp::cpio::config_client::GetParameterProtoRequest;
using google::scp::cpio::config_client::GetParameterProtoResponse;
using std::bind;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::placeholders::_1;

/// Filename for logging errors
static constexpr char kConfigClientProvider[] = "ConfigClientProvider";

namespace google::scp::cpio::client_providers {
ExecutionResult ConfigClientProvider::Init() noexcept {
  auto execution_result = instance_client_provider_->Init();
  if (!execution_result.Successful()) {
    return execution_result;
  }

  if (message_router_) {
    GetEnvironmentNameProtoRequest get_env_name_request;
    Any any_request_1;
    any_request_1.PackFrom(get_env_name_request);
    auto subscribe_result = message_router_->Subscribe(
        any_request_1.type_url(),
        bind(&ConfigClientProvider::OnGetEnvironmentName, this, _1));
    if (!subscribe_result.Successful()) {
      return subscribe_result;
    }

    GetInstanceIdProtoRequest get_instance_id_request;
    Any any_request_2;
    any_request_2.PackFrom(get_instance_id_request);
    subscribe_result = message_router_->Subscribe(
        any_request_2.type_url(),
        bind(&ConfigClientProvider::OnGetInstanceId, this, _1));
    if (!subscribe_result.Successful()) {
      return subscribe_result;
    }

    GetParameterProtoRequest get_parameter_request;
    Any any_request_3;
    any_request_3.PackFrom(get_parameter_request);
    return message_router_->Subscribe(
        any_request_3.type_url(),
        bind(&ConfigClientProvider::OnGetParameter, this, _1));
  }

  return SuccessExecutionResult();
}

ExecutionResult ConfigClientProvider::Run() noexcept {
  auto execution_result = instance_client_provider_->Run();
  if (!execution_result.Successful()) {
    ERROR(kConfigClientProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to run InstanceClientProvider.");
    return execution_result;
  }

  // Prefetches static metadata by issuing blocking calls. And only error out
  // when try to use them.
  fetch_instance_id_result_ =
      instance_client_provider_->GetInstanceId(instance_id_);
  if (!fetch_instance_id_result_.Successful()) {
    ERROR(kConfigClientProvider, kZeroUuid, kZeroUuid,
          fetch_instance_id_result_,
          "Failed getting AWS instance ID during initialization.");
  }

  fetch_environment_name_result_ =
      instance_client_provider_->GetEnvironmentName(
          environment_name_, config_client_options_->environment_tag,
          instance_id_);
  if (!fetch_environment_name_result_.Successful()) {
    ERROR(kConfigClientProvider, kZeroUuid, kZeroUuid,
          fetch_environment_name_result_,
          "Failed getting the AWS environment name during initialization.");
  }

  return SuccessExecutionResult();
}

ExecutionResult ConfigClientProvider::Stop() noexcept {
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

ExecutionResult ConfigClientProvider::GetEnvironmentName(
    AsyncContext<GetEnvironmentNameProtoRequest,
                 GetEnvironmentNameProtoResponse>& context) noexcept {
  auto response = make_shared<GetEnvironmentNameProtoResponse>();
  auto result = SuccessExecutionResult();
  if (environment_name_.empty()) {
    result = fetch_environment_name_result_;
  } else {
    response->set_environment_name(environment_name_);
  }

  context.response = move(response);
  context.result = result;
  context.Finish();

  return SuccessExecutionResult();
}

void ConfigClientProvider::OnGetEnvironmentName(
    AsyncContext<Any, Any> any_context) noexcept {
  auto request = make_shared<GetEnvironmentNameProtoRequest>();
  any_context.request->UnpackTo(request.get());
  AsyncContext<GetEnvironmentNameProtoRequest, GetEnvironmentNameProtoResponse>
      context(move(request),
              bind(CallbackToPackAnyResponse<GetEnvironmentNameProtoRequest,
                                             GetEnvironmentNameProtoResponse>,
                   any_context, _1));
  context.result = GetEnvironmentName(context);
}

void ConfigClientProvider::OnGetInstanceId(
    AsyncContext<Any, Any> any_context) noexcept {
  auto request = make_shared<GetInstanceIdProtoRequest>();
  any_context.request->UnpackTo(request.get());
  AsyncContext<GetInstanceIdProtoRequest, GetInstanceIdProtoResponse> context(
      move(request), bind(CallbackToPackAnyResponse<GetInstanceIdProtoRequest,
                                                    GetInstanceIdProtoResponse>,
                          any_context, _1));
  context.result = GetInstanceId(context);
}

void ConfigClientProvider::OnGetParameter(
    AsyncContext<Any, Any> any_context) noexcept {
  auto request = make_shared<GetParameterProtoRequest>();
  any_context.request->UnpackTo(request.get());
  AsyncContext<GetParameterProtoRequest, GetParameterProtoResponse> context(
      move(request), bind(CallbackToPackAnyResponse<GetParameterProtoRequest,
                                                    GetParameterProtoResponse>,
                          any_context, _1));
  context.result = GetParameter(context);
}
}  // namespace google::scp::cpio::client_providers
