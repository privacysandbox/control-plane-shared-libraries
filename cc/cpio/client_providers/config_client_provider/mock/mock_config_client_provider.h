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

#include <functional>
#include <memory>

#include <google/protobuf/util/message_differencer.h>

#include "cpio/client_providers/interface/config_client_provider_interface.h"
#include "google/protobuf/any.pb.h"

namespace google::scp::cpio::client_providers::mock {
class MockConfigClientProvider : public ConfigClientProviderInterface {
 public:
  core::ExecutionResult Init() noexcept override {
    return core::SuccessExecutionResult();
  }

  core::ExecutionResult Run() noexcept override {
    return core::SuccessExecutionResult();
  }

  core::ExecutionResult Stop() noexcept override {
    return core::SuccessExecutionResult();
  }

  config_client::GetTagProtoRequest get_tag_request_mock;
  config_client::GetTagProtoResponse get_tag_response_mock;
  core::ExecutionResult get_tag_result_mock = core::SuccessExecutionResult();

  core::ExecutionResult GetTag(
      core::AsyncContext<config_client::GetTagProtoRequest,
                         config_client::GetTagProtoResponse>& context) noexcept
      override {
    context.result = get_tag_result_mock;
    if (google::protobuf::util::MessageDifferencer::Equals(get_tag_request_mock,
                                                           *context.request)) {
      context.response = std::make_shared<config_client::GetTagProtoResponse>(
          get_tag_response_mock);
    }
    context.Finish();
    return core::SuccessExecutionResult();
  }

  config_client::GetInstanceIdProtoRequest get_instance_id_request_mock;
  config_client::GetInstanceIdProtoResponse get_instance_id_response_mock;
  core::ExecutionResult get_instance_id_result_mock =
      core::SuccessExecutionResult();

  core::ExecutionResult GetInstanceId(
      core::AsyncContext<config_client::GetInstanceIdProtoRequest,
                         config_client::GetInstanceIdProtoResponse>&
          context) noexcept override {
    context.result = get_instance_id_result_mock;
    if (google::protobuf::util::MessageDifferencer::Equals(
            get_instance_id_request_mock, *context.request)) {
      context.response =
          std::make_shared<config_client::GetInstanceIdProtoResponse>(
              get_instance_id_response_mock);
    }
    context.Finish();
    return core::SuccessExecutionResult();
  }

  config_client::GetParameterProtoRequest get_parameter_request_mock;
  config_client::GetParameterProtoResponse get_parameter_response_mock;
  core::ExecutionResult get_parameter_result_mock =
      core::SuccessExecutionResult();

  core::ExecutionResult GetParameter(
      core::AsyncContext<config_client::GetParameterProtoRequest,
                         config_client::GetParameterProtoResponse>&
          context) noexcept override {
    context.result = get_parameter_result_mock;
    if (google::protobuf::util::MessageDifferencer::Equals(
            get_parameter_request_mock, *context.request)) {
      context.response =
          std::make_shared<config_client::GetParameterProtoResponse>(
              get_parameter_response_mock);
    }
    context.Finish();
    return core::SuccessExecutionResult();
  }
};
}  // namespace google::scp::cpio::client_providers::mock
