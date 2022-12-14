
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

#include "cpio/client_providers/metric_client_provider/src/metric_client_provider.h"

#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <string>

#include <aws/core/Aws.h>

#include "core/async_executor/mock/mock_async_executor.h"
#include "core/interface/async_context.h"
#include "core/message_router/src/error_codes.h"
#include "core/message_router/src/message_router.h"
#include "core/test/utils/conditional_wait.h"
#include "cpio/client_providers/metric_client_provider/mock/mock_metric_client_provider_with_overrides.h"
#include "cpio/client_providers/metric_client_provider/src/error_codes.h"
#include "cpio/common/aws/src/error_codes.h"
#include "cpio/proto/metric_client.pb.h"
#include "public/core/interface/execution_result.h"

using Aws::InitAPI;
using Aws::SDKOptions;
using Aws::ShutdownAPI;
using google::protobuf::Any;
using google::scp::core::AsyncContext;
using google::scp::core ::AsyncExecutorInterface;
using google::scp::core::AsyncOperation;
using google::scp::core::FailureExecutionResult;
using google::scp::core::MessageRouter;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::Timestamp;
using google::scp::core::async_executor::mock::MockAsyncExecutor;
using google::scp::core::errors::GetErrorMessage;
using google::scp::core::errors::SC_MESSAGE_ROUTER_REQUEST_ALREADY_SUBSCRIBED;
using google::scp::core::errors::
    SC_METRIC_CLIENT_PROVIDER_EXECUTOR_NOT_AVAILABLE;
using google::scp::core::errors::SC_METRIC_CLIENT_PROVIDER_IS_NOT_RUNNING;
using google::scp::core::errors::SC_METRIC_CLIENT_PROVIDER_METRIC_NOT_SET;
using google::scp::core::errors::SC_METRIC_CLIENT_PROVIDER_NAMESPACE_NOT_SET;
using google::scp::core::test::WaitUntil;
using google::scp::cpio::client_providers::mock::
    MockMetricClientProviderWithOverrides;
using google::scp::cpio::metric_client::RecordMetricsProtoRequest;
using google::scp::cpio::metric_client::RecordMetricsProtoResponse;
using std::atomic;
using std::function;
using std::make_shared;
using std::make_unique;
using std::move;
using std::shared_ptr;
using std::static_pointer_cast;
using std::string;
using std::unique_ptr;

static constexpr size_t kMetricsBatchSize = 1000;

namespace google::scp::cpio::client_providers::test {
class MetricClientProviderTest : public ::testing::Test {
 protected:
  static void SetUpTestSuite() {
    SDKOptions options;
    InitAPI(options);
  }

  static void TearDownTestSuite() {
    SDKOptions options;
    ShutdownAPI(options);
  }

  shared_ptr<MetricClientOptions> CreateMetricClientOptions(
      bool enable_batch_recording, string metric_namespace = "Test") {
    auto metric_client_options = make_shared<MetricClientOptions>();
    metric_client_options->metric_namespace = metric_namespace;
    metric_client_options->enable_batch_recording = enable_batch_recording;
    return metric_client_options;
  }

  shared_ptr<MockAsyncExecutor> mock_async_executor_ =
      make_shared<MockAsyncExecutor>();
  shared_ptr<MessageRouter> message_router_ = make_shared<MessageRouter>();
};

TEST_F(MetricClientProviderTest, EmptyMessageRouterWithBatchRecording) {
  auto client = make_unique<MockMetricClientProviderWithOverrides>(
      mock_async_executor_, CreateMetricClientOptions(true), nullptr);
  client->schedule_metric_push_mock = [&]() {
    return SuccessExecutionResult();
  };
  EXPECT_EQ(client->Init(), SuccessExecutionResult());
  EXPECT_EQ(client->Run(), SuccessExecutionResult());
}

TEST_F(MetricClientProviderTest, EmptyMessageRouterWithoutBatchRecording) {
  auto client = make_unique<MockMetricClientProviderWithOverrides>(
      mock_async_executor_, CreateMetricClientOptions(false), nullptr);
  client->schedule_metric_push_mock = [&]() {
    return SuccessExecutionResult();
  };
  EXPECT_EQ(client->Init(), SuccessExecutionResult());
  EXPECT_EQ(client->Run(), SuccessExecutionResult());
  EXPECT_EQ(client->Stop(), SuccessExecutionResult());
}

TEST_F(MetricClientProviderTest, EmptyAsyncExecutorIsNotOKWithBatchRecording) {
  auto client = make_unique<MockMetricClientProviderWithOverrides>(
      nullptr, CreateMetricClientOptions(true), nullptr);
  EXPECT_EQ(
      client->Init(),
      FailureExecutionResult(SC_METRIC_CLIENT_PROVIDER_EXECUTOR_NOT_AVAILABLE));
}

TEST_F(MetricClientProviderTest, EmptyAsyncExecutorIsOKWithoutBatchRecording) {
  auto client = make_unique<MockMetricClientProviderWithOverrides>(
      nullptr, CreateMetricClientOptions(false), message_router_);

  EXPECT_EQ(client->Init(), SuccessExecutionResult());
  EXPECT_EQ(client->Run(), SuccessExecutionResult());

  client->record_metric_result_mock = SuccessExecutionResult();

  RecordMetricsProtoRequest record_metric_request;
  auto any_request = make_shared<Any>();
  any_request->PackFrom(record_metric_request);
  atomic<bool> condition = false;
  auto any_context = make_shared<AsyncContext<Any, Any>>(
      move(any_request), [&](AsyncContext<Any, Any>& any_context) {
        EXPECT_EQ(any_context.result, SuccessExecutionResult());
        condition = true;
      });

  message_router_->OnMessageReceived(any_context);
  WaitUntil([&]() { return condition.load(); });
}

TEST_F(MetricClientProviderTest, EmptyNamespaceFailsInit) {
  auto client = make_unique<MockMetricClientProviderWithOverrides>(
      mock_async_executor_, CreateMetricClientOptions(false, ""),
      message_router_);
  auto result = client->Init();
  EXPECT_EQ(result, FailureExecutionResult(
                        SC_METRIC_CLIENT_PROVIDER_NAMESPACE_NOT_SET));
}

TEST_F(MetricClientProviderTest, InvalidMetric) {
  auto client = make_unique<MockMetricClientProviderWithOverrides>(
      mock_async_executor_, CreateMetricClientOptions(false), message_router_);

  AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse> context(
      make_shared<RecordMetricsProtoRequest>(),
      [&](AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>&
              context) {});

  EXPECT_EQ(client->Init(), SuccessExecutionResult());
  EXPECT_EQ(client->Run(), SuccessExecutionResult());
  EXPECT_EQ(client->RecordMetrics(context),
            FailureExecutionResult(SC_METRIC_CLIENT_PROVIDER_METRIC_NOT_SET));
  EXPECT_EQ(client->Stop(), SuccessExecutionResult());
}

TEST_F(MetricClientProviderTest, FailedWithoutRunning) {
  auto client = make_unique<MockMetricClientProviderWithOverrides>(
      mock_async_executor_, CreateMetricClientOptions(true), message_router_);

  AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse> context(
      make_shared<RecordMetricsProtoRequest>(),
      [&](AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>&
              context) {});

  EXPECT_EQ(client->Init(), SuccessExecutionResult());
  EXPECT_EQ(client->ScheduleMetricsBatchPush(),
            FailureExecutionResult(SC_METRIC_CLIENT_PROVIDER_IS_NOT_RUNNING));
  EXPECT_EQ(client->RecordMetrics(context),
            FailureExecutionResult(SC_METRIC_CLIENT_PROVIDER_IS_NOT_RUNNING));
}

TEST_F(MetricClientProviderTest, LaunchScheduleMetricsBatchPushWithRun) {
  auto client = make_unique<MockMetricClientProviderWithOverrides>(
      mock_async_executor_, CreateMetricClientOptions(true), message_router_);

  bool schedule_for_is_called = false;
  mock_async_executor_->schedule_for_mock =
      [&](const AsyncOperation& work, Timestamp timestamp,
          function<bool()>& cancellation_callback) {
        schedule_for_is_called = true;
        return FailureExecutionResult(SC_UNKNOWN);
      };

  EXPECT_EQ(client->Init(), SuccessExecutionResult());
  EXPECT_EQ(client->Run(), FailureExecutionResult(SC_UNKNOWN));
  WaitUntil([&]() { return schedule_for_is_called; });
}

TEST_F(MetricClientProviderTest, RecordMetricWithoutBatch) {
  auto client = make_unique<MockMetricClientProviderWithOverrides>(
      mock_async_executor_, CreateMetricClientOptions(false), message_router_);

  auto request = make_shared<RecordMetricsProtoRequest>();
  auto metric = request->add_metrics();
  metric->set_name("metric1");
  metric->set_value("123");

  AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse> context(
      request, [&](AsyncContext<RecordMetricsProtoRequest,
                                RecordMetricsProtoResponse>& context) {});

  int64_t batch_push_called_count = 0;
  client->metrics_batch_push_mock =
      [&](const std::shared_ptr<std::vector<
              core::AsyncContext<metric_client::RecordMetricsProtoRequest,
                                 metric_client::RecordMetricsProtoResponse>>>&
              metric_requests_vector) noexcept {
        batch_push_called_count += 1;
        EXPECT_EQ(metric_requests_vector->size(), 1);
        return SuccessExecutionResult();
      };

  EXPECT_EQ(client->Init(), SuccessExecutionResult());
  EXPECT_EQ(client->Run(), SuccessExecutionResult());
  EXPECT_EQ(client->RecordMetrics(context), SuccessExecutionResult());
  EXPECT_EQ(client->GetSizeMetricRequestsVector(), 0);
  EXPECT_EQ(client->RecordMetrics(context), SuccessExecutionResult());
  EXPECT_EQ(client->GetSizeMetricRequestsVector(), 0);
  WaitUntil([&]() { return batch_push_called_count == 2; });
}

TEST_F(MetricClientProviderTest, RecordMetricWithBatch) {
  auto client = make_unique<MockMetricClientProviderWithOverrides>(
      mock_async_executor_, CreateMetricClientOptions(true), message_router_);

  atomic<bool> schedule_for_is_called = false;
  mock_async_executor_->schedule_for_mock =
      [&](const AsyncOperation& work, Timestamp timestamp,
          function<bool()>& cancellation_callback) {
        schedule_for_is_called = true;
        return SuccessExecutionResult();
      };

  auto record_metric_request = make_shared<RecordMetricsProtoRequest>();
  auto metric = record_metric_request->add_metrics();
  metric->set_name("metric1");
  metric->set_value("123");
  AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse> context(
      record_metric_request,
      [&](AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>&
              context) {});

  atomic<bool> batch_push_called = false;
  client->metrics_batch_push_mock =
      [&](const std::shared_ptr<std::vector<
              core::AsyncContext<metric_client::RecordMetricsProtoRequest,
                                 metric_client::RecordMetricsProtoResponse>>>&
              metric_requests_vector) noexcept {
        batch_push_called.store(true);
        EXPECT_EQ(metric_requests_vector->size(), kMetricsBatchSize);
        return SuccessExecutionResult();
      };

  EXPECT_EQ(client->Init(), SuccessExecutionResult());
  EXPECT_EQ(client->Run(), SuccessExecutionResult());

  for (int i = 0; i <= 2001; i++) {
    EXPECT_EQ(client->RecordMetrics(context), SuccessExecutionResult());
  }

  WaitUntil([&]() { return schedule_for_is_called.load(); });

  WaitUntil([&]() { return batch_push_called.load(); });
}

TEST_F(MetricClientProviderTest, RunMetricsBatchPush) {
  auto client = make_unique<MockMetricClientProviderWithOverrides>(
      mock_async_executor_, CreateMetricClientOptions(true), message_router_);

  auto record_metric_request = make_shared<RecordMetricsProtoRequest>();
  auto metric = record_metric_request->add_metrics();
  metric->set_name("metric1");
  metric->set_value("123");
  AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse> context(
      record_metric_request,
      [&](AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>&
              context) {});

  int64_t schedule_metric_push_count = 0;
  client->schedule_metric_push_mock = [&]() {
    schedule_metric_push_count += 1;
    return SuccessExecutionResult();
  };

  int64_t batch_push_called_count = 0;
  client->metrics_batch_push_mock =
      [&](const std::shared_ptr<std::vector<
              core::AsyncContext<metric_client::RecordMetricsProtoRequest,
                                 metric_client::RecordMetricsProtoResponse>>>&
              metric_requests_vector) noexcept {
        batch_push_called_count += 1;
        EXPECT_EQ(metric_requests_vector->size(), 2);
        return SuccessExecutionResult();
      };

  EXPECT_EQ(client->Init(), SuccessExecutionResult());
  EXPECT_EQ(client->Run(), SuccessExecutionResult());

  EXPECT_EQ(client->RecordMetrics(context), SuccessExecutionResult());
  EXPECT_EQ(client->RecordMetrics(context), SuccessExecutionResult());
  EXPECT_EQ(client->GetSizeMetricRequestsVector(), 2);
  client->RunMetricsBatchPush();
  EXPECT_EQ(client->GetSizeMetricRequestsVector(), 0);
  WaitUntil([&]() { return batch_push_called_count == 1; });
  WaitUntil([&]() { return schedule_metric_push_count == 1; });
}
}  // namespace google::scp::cpio::client_providers::test
