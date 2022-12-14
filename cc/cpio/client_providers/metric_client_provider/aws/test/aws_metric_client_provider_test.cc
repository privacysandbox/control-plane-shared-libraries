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

#include "cpio/client_providers/metric_client_provider/aws/src/aws_metric_client_provider.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include <aws/core/Aws.h>
#include <aws/monitoring/CloudWatchClient.h>
#include <aws/monitoring/CloudWatchErrors.h>
#include <aws/monitoring/model/PutMetricDataRequest.h>

#include "core/async_executor/mock/mock_async_executor.h"
#include "core/interface/async_context.h"
#include "core/test/utils/conditional_wait.h"
#include "cpio/client_providers/metric_client_provider/aws/mock/mock_aws_metric_client_provider_with_overrides.h"
#include "cpio/client_providers/metric_client_provider/aws/mock/mock_cloud_watch_client.h"
#include "cpio/client_providers/metric_client_provider/aws/src/aws_metric_client_utils.h"
#include "cpio/client_providers/metric_client_provider/aws/src/error_codes.h"
#include "cpio/common/aws/src/error_codes.h"
#include "cpio/proto/metric_client.pb.h"
#include "public/core/interface/execution_result.h"

using Aws::InitAPI;
using Aws::SDKOptions;
using Aws::ShutdownAPI;
using Aws::Client::AWSError;
using Aws::CloudWatch::CloudWatchErrors;
using Aws::CloudWatch::Model::MetricDatum;
using Aws::CloudWatch::Model::PutMetricDataOutcome;
using Aws::CloudWatch::Model::PutMetricDataRequest;
using google::scp::core::AsyncContext;
using google::scp::core::ExecutionResult;
using google::scp::core::FailureExecutionResult;
using google::scp::core::MessageRouter;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::Timestamp;
using google::scp::core::async_executor::mock::MockAsyncExecutor;
using google::scp::core::errors::SC_AWS_INTERNAL_SERVICE_ERROR;
using google::scp::core::test::WaitUntil;
using google::scp::cpio::client_providers::AwsMetricClientUtils;
using google::scp::cpio::client_providers::mock::
    MockAwsMetricClientProviderOverrides;
using google::scp::cpio::client_providers::mock::MockCloudWatchClient;
using google::scp::cpio::metric_client::MetricUnitProto;
using google::scp::cpio::metric_client::RecordMetricsProtoRequest;
using google::scp::cpio::metric_client::RecordMetricsProtoResponse;
using std::atomic;
using std::make_shared;
using std::make_unique;
using std::map;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

static constexpr char kName[] = "test_name";
static constexpr char kValue[] = "12346";
static constexpr MetricUnitProto kUnit = MetricUnitProto::METRIC_UNIT_COUNT;
static constexpr char kNamespace[] = "aws_name_space";

namespace google::scp::cpio::client_providers::test {
class AwsMetricClientProviderTest : public ::testing::Test {
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
    auto mock_async_executor = make_shared<MockAsyncExecutor>();
    auto message_router = make_shared<MessageRouter>();
    auto metric_client_options = make_shared<MetricClientOptions>();
    metric_client_options->metric_namespace = kNamespace;

    client_ = make_unique<MockAwsMetricClientProviderOverrides>(
        metric_client_options, mock_async_executor, message_router);
  }

  void SetRecordMetricsProtoRequest(
      RecordMetricsProtoRequest& record_metric_request,
      const string& value = kValue, int metrics_num = 1,
      const int64_t& timestamp =
          duration_cast<milliseconds>(system_clock::now().time_since_epoch())
              .count()) {
    for (auto i = 0; i < metrics_num; i++) {
      auto metric = record_metric_request.add_metrics();
      metric->set_name(kName);
      metric->set_value(value);
      metric->set_unit(kUnit);
      metric->set_timestamp_in_ms(timestamp);
    }
  }

  unique_ptr<MockAwsMetricClientProviderOverrides> client_;
};

TEST_F(AwsMetricClientProviderTest, InitSuccess) {
  client_->GetInstanceClientProvider()->region_mock = "us-east-1";
  EXPECT_EQ(client_->Init(), SuccessExecutionResult());
  EXPECT_EQ(client_->Run(), SuccessExecutionResult());
  EXPECT_EQ(client_->Stop(), SuccessExecutionResult());
}

TEST_F(AwsMetricClientProviderTest, InitFailure) {
  auto failure = FailureExecutionResult(SC_AWS_INTERNAL_SERVICE_ERROR);
  client_->GetInstanceClientProvider()->get_region_result_mock = failure;
  EXPECT_EQ(client_->Init(), failure);
}

TEST_F(AwsMetricClientProviderTest, SplitsOversizeRequestsVector) {
  client_->GetInstanceClientProvider()->region_mock = "us-east-1";
  EXPECT_EQ(client_->Init(), SuccessExecutionResult());
  EXPECT_EQ(client_->Run(), SuccessExecutionResult());

  Aws::NoResult result;
  client_->GetCloudWatchClient()->put_metric_data_outcome_mock =
      PutMetricDataOutcome(result);

  size_t put_metric_data_request_count = 0;
  client_->GetCloudWatchClient()->put_metric_data_async_mock =
      [&](const Aws::CloudWatch::Model::PutMetricDataRequest& request,
          const Aws::CloudWatch::PutMetricDataResponseReceivedHandler& handler,
          const std::shared_ptr<const Aws::Client::AsyncCallerContext>&
              context) {
        EXPECT_EQ(request.GetNamespace(), kNamespace);
        put_metric_data_request_count += 1;
        return;
      };

  RecordMetricsProtoRequest record_metric_request;
  SetRecordMetricsProtoRequest(record_metric_request);
  AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse> context(
      make_shared<RecordMetricsProtoRequest>(record_metric_request),
      [&](AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>&
              context) {});
  auto requests_vector = make_shared<vector<
      AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>>>();
  PutMetricDataRequest request_mock;
  for (auto i = 0; i < 10000; i++) {
    requests_vector->emplace_back(context);
  }

  EXPECT_EQ(client_->MetricsBatchPush(requests_vector),
            SuccessExecutionResult());
  WaitUntil([&]() { return put_metric_data_request_count == 10; });

  // Cannot stop the client because the AWS callback is mocked.
}

TEST_F(AwsMetricClientProviderTest, KeepMetricsInTheSameRequest) {
  client_->GetInstanceClientProvider()->region_mock = "us-east-1";
  EXPECT_EQ(client_->Init(), SuccessExecutionResult());
  EXPECT_EQ(client_->Run(), SuccessExecutionResult());

  Aws::NoResult result;
  client_->GetCloudWatchClient()->put_metric_data_outcome_mock =
      PutMetricDataOutcome(result);

  atomic<int> put_metric_data_request_count = 0;
  atomic<int> number_datums_received = 0;
  client_->GetCloudWatchClient()->put_metric_data_async_mock =
      [&](const Aws::CloudWatch::Model::PutMetricDataRequest& request,
          const Aws::CloudWatch::PutMetricDataResponseReceivedHandler& handler,
          const std::shared_ptr<const Aws::Client::AsyncCallerContext>&
              context) {
        EXPECT_EQ(request.GetNamespace(), kNamespace);
        put_metric_data_request_count += 1;
        number_datums_received.fetch_add((request.GetMetricData().size()));
        return;
      };

  auto requests_vector = make_shared<vector<
      AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>>>();
  for (auto metric_num : {100, 500, 600, 800}) {
    RecordMetricsProtoRequest record_metric_request;
    SetRecordMetricsProtoRequest(record_metric_request, kValue, metric_num);

    AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse> context(
        make_shared<RecordMetricsProtoRequest>(record_metric_request),
        [&](AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>&
                context) {});
    requests_vector->push_back(context);
  }
  EXPECT_EQ(client_->MetricsBatchPush(requests_vector),
            SuccessExecutionResult());
  WaitUntil([&]() { return put_metric_data_request_count.load() == 3; });
  WaitUntil([&]() { return number_datums_received.load() == 2000; });

  // Cannot stop the client because the AWS callback is mocked.
}

TEST_F(AwsMetricClientProviderTest, OnPutMetricDataAsyncCallbackWithError) {
  client_->GetInstanceClientProvider()->region_mock = "us-east-1";
  EXPECT_EQ(client_->Init(), SuccessExecutionResult());
  EXPECT_EQ(client_->Run(), SuccessExecutionResult());

  AWSError<CloudWatchErrors> error(CloudWatchErrors::UNKNOWN, false);
  client_->GetCloudWatchClient()->put_metric_data_outcome_mock =
      PutMetricDataOutcome(error);

  RecordMetricsProtoRequest record_metric_request;
  SetRecordMetricsProtoRequest(record_metric_request);
  atomic<int64_t> context_finish_count = 0;
  AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse> context(
      make_shared<RecordMetricsProtoRequest>(record_metric_request),
      [&](AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>&
              context) {
        context_finish_count += 1;
        EXPECT_EQ(context.result,
                  FailureExecutionResult(SC_AWS_INTERNAL_SERVICE_ERROR));
      });
  auto requests_vector = make_shared<vector<
      AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>>>();
  requests_vector->push_back(context);
  requests_vector->push_back(context);
  requests_vector->push_back(context);
  EXPECT_EQ(client_->MetricsBatchPush(requests_vector),
            SuccessExecutionResult());
  WaitUntil([&]() { return context_finish_count == 3; });

  EXPECT_EQ(client_->Stop(), SuccessExecutionResult());
}

TEST_F(AwsMetricClientProviderTest, OnPutMetricDataAsyncCallbackWithSuccess) {
  client_->GetInstanceClientProvider()->region_mock = "us-east-1";
  EXPECT_EQ(client_->Init(), SuccessExecutionResult());
  EXPECT_EQ(client_->Run(), SuccessExecutionResult());

  Aws::NoResult result;
  client_->GetCloudWatchClient()->put_metric_data_outcome_mock =
      PutMetricDataOutcome(result);

  RecordMetricsProtoRequest record_metric_request;
  SetRecordMetricsProtoRequest(record_metric_request);
  atomic<int64_t> context_finish_count = 0;
  AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse> context(
      make_shared<RecordMetricsProtoRequest>(record_metric_request),
      [&](AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>&
              context) {
        context_finish_count += 1;
        EXPECT_EQ(context.result, SuccessExecutionResult());
      });
  auto requests_vector = make_shared<vector<
      AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>>>();
  requests_vector->push_back(context);
  requests_vector->push_back(context);
  requests_vector->push_back(context);
  EXPECT_EQ(client_->MetricsBatchPush(requests_vector),
            SuccessExecutionResult());
  WaitUntil([&]() { return context_finish_count == 3; });

  EXPECT_EQ(client_->Stop(), SuccessExecutionResult());
}
}  // namespace google::scp::cpio::client_providers::test
