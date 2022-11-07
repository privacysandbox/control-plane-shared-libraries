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

#include "cpio/client_providers/config_client_provider/aws/src/aws_config_client_provider.h"

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
#include "core/test/utils/conditional_wait.h"
#include "cpio/client_providers/config_client_provider/aws/mock/mock_aws_config_client_provider_with_overrides.h"
#include "cpio/client_providers/instance_client_provider/mock/mock_instance_client_provider.h"
#include "cpio/common/aws/src/error_codes.h"
#include "cpio/proto/config_client.pb.h"
#include "public/core/interface/execution_result.h"

using Aws::InitAPI;
using Aws::SDKOptions;
using Aws::ShutdownAPI;
using Aws::Client::AWSError;
using Aws::SSM::SSMErrors;
using Aws::SSM::Model::GetParametersOutcome;
using Aws::SSM::Model::GetParametersRequest;
using Aws::SSM::Model::GetParametersResult;
using Aws::SSM::Model::Parameter;
using google::scp::core::AsyncContext;
using google::scp::core::ExecutionStatus;
using google::scp::core::FailureExecutionResult;
using google::scp::core::MessageRouter;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::errors::
    SC_AWS_CONFIG_CLIENT_PROVIDER_PARAMETER_NOT_FOUND;
using google::scp::core::errors::SC_AWS_INTERNAL_SERVICE_ERROR;
using google::scp::core::test::WaitUntil;
using google::scp::cpio::client_providers::mock::
    MockAwsConfigClientProviderWithOverrides;
using google::scp::cpio::config_client::GetEnvironmentNameProtoRequest;
using google::scp::cpio::config_client::GetEnvironmentNameProtoResponse;
using google::scp::cpio::config_client::GetInstanceIdProtoRequest;
using google::scp::cpio::config_client::GetInstanceIdProtoResponse;
using google::scp::cpio::config_client::GetParameterProtoRequest;
using google::scp::cpio::config_client::GetParameterProtoResponse;
using std::atomic;
using std::make_shared;
using std::make_unique;
using std::map;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;

static constexpr char kRegion[] = "us-east-1";
static constexpr char kParameterName1[] = "/service/parameter_name_1";
static constexpr char kParameterName2[] = "/service/parameter_name_2";
static const vector<string> kParameterNames({string(kParameterName1),
                                             string(kParameterName2)});
static constexpr char kValue1[] = "value1";
static constexpr char kValue2[] = "value2";

namespace google::scp::cpio::client_providers::test {
class AwsConfigClientProviderTest : public ::testing::Test {
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
    auto config_client_options = make_shared<ConfigClientOptions>();
    config_client_options->parameter_names = kParameterNames;
    client_ = make_unique<MockAwsConfigClientProviderWithOverrides>(
        config_client_options);

    client_->GetInstanceClientProvider()->region_mock = kRegion;
  }

  void MockParameters() {
    // Mocks GetParametersRequest.
    GetParametersRequest get_parameters_request;
    get_parameters_request.AddNames(kParameterName1);
    get_parameters_request.AddNames(kParameterName2);
    client_->GetSSMClient()->get_parameters_request_mock =
        get_parameters_request;

    // Mocks successs GetParametersOutcome
    GetParametersResult result;
    Parameter parameter1;
    parameter1.SetName(kParameterName1);
    parameter1.SetValue(kValue1);
    result.AddParameters(parameter1);
    Parameter parameter2;
    parameter2.SetName(kParameterName2);
    parameter2.SetValue(kValue2);
    result.AddParameters(parameter2);
    GetParametersOutcome get_parameters_outcome(result);
    client_->GetSSMClient()->get_parameters_outcome_mock =
        get_parameters_outcome;
  }

  void TearDown() override {
    EXPECT_EQ(client_->Stop(), SuccessExecutionResult());
  }

  unique_ptr<MockAwsConfigClientProviderWithOverrides> client_;
};

TEST_F(AwsConfigClientProviderTest, FailedToFetchRegion) {
  auto failure = FailureExecutionResult(SC_AWS_INTERNAL_SERVICE_ERROR);
  client_->GetInstanceClientProvider()->get_region_result_mock = failure;

  EXPECT_EQ(client_->Init(), failure);
}

TEST_F(AwsConfigClientProviderTest, SucceededToFetchInstanceId) {
  EXPECT_EQ(client_->Init(), SuccessExecutionResult());
  string instance_id = "instance_id";
  client_->GetInstanceClientProvider()->instance_id_mock = instance_id;
  MockParameters();

  EXPECT_EQ(client_->Run(), SuccessExecutionResult());

  atomic<bool> condition = false;
  AsyncContext<GetInstanceIdProtoRequest, GetInstanceIdProtoResponse> context(
      make_shared<GetInstanceIdProtoRequest>(),
      [&](AsyncContext<GetInstanceIdProtoRequest, GetInstanceIdProtoResponse>&
              context) {
        EXPECT_EQ(context.result, SuccessExecutionResult());
        EXPECT_EQ(context.response->instance_id(), instance_id);
        condition = true;
      });

  EXPECT_EQ(client_->GetInstanceId(context), SuccessExecutionResult());
  WaitUntil([&]() { return condition.load(); });
}

TEST_F(AwsConfigClientProviderTest, SucceededToFetchEnvName) {
  EXPECT_EQ(client_->Init(), SuccessExecutionResult());
  string name = "env_name";
  client_->GetInstanceClientProvider()->environment_name_mock = name;
  MockParameters();

  EXPECT_EQ(client_->Run(), SuccessExecutionResult());

  atomic<bool> condition = false;
  AsyncContext<GetEnvironmentNameProtoRequest, GetEnvironmentNameProtoResponse>
      context(make_shared<GetEnvironmentNameProtoRequest>(),
              [&](AsyncContext<GetEnvironmentNameProtoRequest,
                               GetEnvironmentNameProtoResponse>& context) {
                EXPECT_EQ(context.result, SuccessExecutionResult());
                EXPECT_EQ(context.response->environment_name(), name);
                condition = true;
              });

  EXPECT_EQ(client_->GetEnvironmentName(context), SuccessExecutionResult());
  WaitUntil([&]() { return condition.load(); });
}

TEST_F(AwsConfigClientProviderTest, FailedToFetchParameter) {
  EXPECT_EQ(client_->Init(), SuccessExecutionResult());
  MockParameters();
  AWSError<SSMErrors> error(SSMErrors::INTERNAL_FAILURE, false);
  GetParametersOutcome outcome(error);
  client_->GetSSMClient()->get_parameters_outcome_mock = outcome;

  EXPECT_EQ(client_->Run(),
            FailureExecutionResult(SC_AWS_INTERNAL_SERVICE_ERROR));
}

TEST_F(AwsConfigClientProviderTest, ParameterNotFound) {
  EXPECT_EQ(client_->Init(), SuccessExecutionResult());
  MockParameters();
  EXPECT_EQ(client_->Run(), SuccessExecutionResult());

  atomic<bool> condition = false;
  auto request = make_shared<GetParameterProtoRequest>();
  request->set_parameter_name("invalid_parameter");
  AsyncContext<GetParameterProtoRequest, GetParameterProtoResponse> context(
      move(request), [&](AsyncContext<GetParameterProtoRequest,
                                      GetParameterProtoResponse>& context) {
        EXPECT_EQ(context.result,
                  FailureExecutionResult(
                      SC_AWS_CONFIG_CLIENT_PROVIDER_PARAMETER_NOT_FOUND));
        condition = true;
      });
  EXPECT_EQ(client_->GetParameter(context), SuccessExecutionResult());
  WaitUntil([&]() { return condition.load(); });
}

TEST_F(AwsConfigClientProviderTest, SucceedToFetchParameter) {
  EXPECT_EQ(client_->Init(), SuccessExecutionResult());
  MockParameters();
  EXPECT_EQ(client_->Run(), SuccessExecutionResult());

  atomic<bool> condition1 = false;
  auto request = make_shared<GetParameterProtoRequest>();
  request->set_parameter_name(kParameterName1);
  AsyncContext<GetParameterProtoRequest, GetParameterProtoResponse> context1(
      move(request), [&](AsyncContext<GetParameterProtoRequest,
                                      GetParameterProtoResponse>& context) {
        EXPECT_EQ(context.result, SuccessExecutionResult());
        EXPECT_EQ(context.response->value(), kValue1);
        condition1 = true;
      });
  EXPECT_EQ(client_->GetParameter(context1), SuccessExecutionResult());
  WaitUntil([&]() { return condition1.load(); });

  atomic<bool> condition2 = false;
  request->set_parameter_name(kParameterName2);
  AsyncContext<GetParameterProtoRequest, GetParameterProtoResponse> context2(
      move(request), [&](AsyncContext<GetParameterProtoRequest,
                                      GetParameterProtoResponse>& context) {
        EXPECT_EQ(context.result, SuccessExecutionResult());
        EXPECT_EQ(context.response->value(), kValue2);
        condition2 = true;
      });
  EXPECT_EQ(client_->GetParameter(context2), SuccessExecutionResult());
  WaitUntil([&]() { return condition2.load(); });
}
}  // namespace google::scp::cpio::client_providers::test
