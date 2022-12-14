// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <aws/core/Aws.h>
#include <aws/core/utils/Outcome.h>
#include <aws/ssm/SSMClient.h>
#include <aws/ssm/SSMErrors.h>
#include <aws/ssm/model/GetParametersRequest.h>

#include "core/interface/async_context.h"
#include "core/message_router/src/message_router.h"
#include "core/test/utils/conditional_wait.h"
#include "cpio/client_providers/config_client_provider/aws/src/aws_config_client_provider.h"
#include "cpio/client_providers/config_client_provider/mock/mock_config_client_provider_with_overrides.h"
#include "cpio/client_providers/config_client_provider/src/error_codes.h"
#include "cpio/client_providers/instance_client_provider/mock/mock_instance_client_provider.h"
#include "cpio/common/aws/src/error_codes.h"
#include "cpio/proto/config_client.pb.h"
#include "public/core/interface/execution_result.h"

using Aws::InitAPI;
using Aws::SDKOptions;
using Aws::ShutdownAPI;
using google::protobuf::Any;
using google::scp::core::AsyncContext;
using google::scp::core::ExecutionStatus;
using google::scp::core::FailureExecutionResult;
using google::scp::core::MessageRouter;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::errors::
    SC_AWS_CONFIG_CLIENT_PROVIDER_PARAMETER_NOT_FOUND;
using google::scp::core::errors::SC_AWS_INTERNAL_SERVICE_ERROR;
using google::scp::core::errors::SC_CONFIG_CLIENT_PROVIDER_INVALID_TAG_NAME;
using google::scp::core::errors::SC_CONFIG_CLIENT_PROVIDER_TAG_NOT_FOUND;
using google::scp::core::errors::SC_MESSAGE_ROUTER_REQUEST_ALREADY_SUBSCRIBED;
using google::scp::core::test::WaitUntil;
using google::scp::cpio::client_providers::mock::
    MockConfigClientProviderWithOverrides;
using google::scp::cpio::config_client::GetInstanceIdProtoRequest;
using google::scp::cpio::config_client::GetInstanceIdProtoResponse;
using google::scp::cpio::config_client::GetParameterProtoRequest;
using google::scp::cpio::config_client::GetParameterProtoResponse;
using google::scp::cpio::config_client::GetTagProtoRequest;
using google::scp::cpio::config_client::GetTagProtoResponse;
using std::atomic;
using std::make_shared;
using std::make_unique;
using std::map;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

static constexpr char kInstanceId[] = "instance_id";
static constexpr char kTagName[] = "tag_name";
static constexpr char kTagValue[] = "tag_value";

namespace google::scp::cpio::client_providers::test {
class ConfigClientProviderTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    SDKOptions options;
    InitAPI(options);
  }

  static void TearDownTestSuite() {
    SDKOptions options;
    ShutdownAPI(options);
  }

  void SetUp() override {
    config_client_options_ = make_shared<ConfigClientOptions>();
    message_router_ = make_shared<MessageRouter>();
    client_ = make_unique<MockConfigClientProviderWithOverrides>(
        config_client_options_, message_router_);

    client_->GetInstanceClientProvider()->instance_id_mock = kInstanceId;
    client_->GetInstanceClientProvider()->tag_values_mock = {
        {kTagName, kTagValue}};
    EXPECT_EQ(client_->Init(), SuccessExecutionResult());
  }

  void TearDown() override {
    EXPECT_EQ(client_->Stop(), SuccessExecutionResult());
  }

  shared_ptr<MessageRouter> message_router_;
  unique_ptr<MockConfigClientProviderWithOverrides> client_;
  shared_ptr<ConfigClientOptions> config_client_options_;
};

TEST_F(ConfigClientProviderTest, EmptyMessageRouter) {
  client_ = make_unique<MockConfigClientProviderWithOverrides>(
      config_client_options_, nullptr);
  EXPECT_EQ(client_->Init(), SuccessExecutionResult());
  EXPECT_EQ(client_->Run(), SuccessExecutionResult());
  EXPECT_EQ(client_->Stop(), SuccessExecutionResult());
}

TEST_F(ConfigClientProviderTest, FailedToFetchInstanceId) {
  FailureExecutionResult failure(SC_AWS_INTERNAL_SERVICE_ERROR);
  client_->GetInstanceClientProvider()->get_instance_id_result_mock = failure;

  EXPECT_EQ(client_->Run(), SuccessExecutionResult());

  atomic<bool> condition = false;
  AsyncContext<GetInstanceIdProtoRequest, GetInstanceIdProtoResponse> context(
      make_shared<GetInstanceIdProtoRequest>(),
      [&](AsyncContext<GetInstanceIdProtoRequest, GetInstanceIdProtoResponse>&
              context) {
        EXPECT_EQ(context.result, failure);
        condition = true;
      });
  EXPECT_EQ(client_->GetInstanceId(context), SuccessExecutionResult());
  WaitUntil([&]() { return condition.load(); });
}

TEST_F(ConfigClientProviderTest, SucceededToFetchInstanceId) {
  EXPECT_EQ(client_->Run(), SuccessExecutionResult());

  atomic<bool> condition = false;
  AsyncContext<GetInstanceIdProtoRequest, GetInstanceIdProtoResponse> context(
      make_shared<GetInstanceIdProtoRequest>(),
      [&](AsyncContext<GetInstanceIdProtoRequest, GetInstanceIdProtoResponse>&
              context) {
        EXPECT_EQ(context.result, SuccessExecutionResult());
        EXPECT_EQ(context.response->instance_id(), kInstanceId);
        condition = true;
      });

  EXPECT_EQ(client_->GetInstanceId(context), SuccessExecutionResult());
  WaitUntil([&]() { return condition.load(); });
}

TEST_F(ConfigClientProviderTest, InvalidTagName) {
  EXPECT_EQ(client_->Run(), SuccessExecutionResult());

  atomic<bool> condition = false;
  auto request = make_shared<GetTagProtoRequest>();
  AsyncContext<GetTagProtoRequest, GetTagProtoResponse> context(
      move(request),
      [&](AsyncContext<GetTagProtoRequest, GetTagProtoResponse>& context) {
        EXPECT_EQ(
            context.result,
            FailureExecutionResult(SC_CONFIG_CLIENT_PROVIDER_INVALID_TAG_NAME));
        condition = true;
      });

  EXPECT_EQ(client_->GetTag(context),
            FailureExecutionResult(SC_CONFIG_CLIENT_PROVIDER_INVALID_TAG_NAME));
  WaitUntil([&]() { return condition.load(); });
}

TEST_F(ConfigClientProviderTest, FailedToFetchTag) {
  FailureExecutionResult result(SC_AWS_INTERNAL_SERVICE_ERROR);
  client_->GetInstanceClientProvider()->get_tags_result_mock = result;

  EXPECT_EQ(client_->Run(),
            FailureExecutionResult(SC_AWS_INTERNAL_SERVICE_ERROR));
}

TEST_F(ConfigClientProviderTest, SucceededToFetchTag) {
  EXPECT_EQ(client_->Run(), SuccessExecutionResult());

  atomic<bool> condition = false;
  auto request = make_shared<GetTagProtoRequest>();
  request->set_tag_name(kTagName);
  AsyncContext<GetTagProtoRequest, GetTagProtoResponse> context(
      move(request),
      [&](AsyncContext<GetTagProtoRequest, GetTagProtoResponse>& context) {
        EXPECT_EQ(context.result, SuccessExecutionResult());
        EXPECT_EQ(context.response->value(), kTagValue);
        condition = true;
      });

  EXPECT_EQ(client_->GetTag(context), SuccessExecutionResult());
  WaitUntil([&]() { return condition.load(); });
}

TEST_F(ConfigClientProviderTest, TagNotFound) {
  EXPECT_EQ(client_->Run(), SuccessExecutionResult());

  atomic<bool> condition = false;
  auto request = make_shared<GetTagProtoRequest>();
  request->set_tag_name("tag_2");
  AsyncContext<GetTagProtoRequest, GetTagProtoResponse> context(
      move(request),
      [&](AsyncContext<GetTagProtoRequest, GetTagProtoResponse>& context) {
        EXPECT_EQ(context.result, FailureExecutionResult(
                                      SC_CONFIG_CLIENT_PROVIDER_TAG_NOT_FOUND));
        condition = true;
      });

  EXPECT_EQ(client_->GetTag(context), SuccessExecutionResult());
  WaitUntil([&]() { return condition.load(); });
}

TEST_F(ConfigClientProviderTest, FailedToSubscribeGetTag) {
  GetTagProtoRequest get_env_name_request;
  Any any_request;
  any_request.PackFrom(get_env_name_request);
  message_router_->Subscribe(any_request.type_url(),
                             [](AsyncContext<Any, Any> context) {});

  EXPECT_EQ(client_->Init(), FailureExecutionResult(
                                 SC_MESSAGE_ROUTER_REQUEST_ALREADY_SUBSCRIBED));
}

TEST_F(ConfigClientProviderTest, FailedToSubscribeGetInstanceId) {
  GetInstanceIdProtoRequest get_instance_id_request;
  Any any_request;
  any_request.PackFrom(get_instance_id_request);
  message_router_->Subscribe(any_request.type_url(),
                             [](AsyncContext<Any, Any> context) {});

  EXPECT_EQ(client_->Init(), FailureExecutionResult(
                                 SC_MESSAGE_ROUTER_REQUEST_ALREADY_SUBSCRIBED));
}

TEST_F(ConfigClientProviderTest, FailedToSubscribeGetParameter) {
  GetParameterProtoRequest get_parameter_request;
  Any any_request;
  any_request.PackFrom(get_parameter_request);
  message_router_->Subscribe(any_request.type_url(),
                             [](AsyncContext<Any, Any> context) {});

  EXPECT_EQ(client_->Init(), FailureExecutionResult(
                                 SC_MESSAGE_ROUTER_REQUEST_ALREADY_SUBSCRIBED));
}
}  // namespace google::scp::cpio::client_providers::test
