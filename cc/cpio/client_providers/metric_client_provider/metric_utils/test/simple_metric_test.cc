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

#include "cpio/client_providers/metric_client_provider/metric_utils/src/simple_metric.h"

#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "core/async_executor/mock/mock_async_executor.h"
#include "core/interface/async_context.h"
#include "core/message_router/src/error_codes.h"
#include "core/message_router/src/message_router.h"
#include "core/test/utils/conditional_wait.h"
#include "cpio/client_providers/metric_client_provider/metric_utils/interface/type_def.h"
#include "cpio/client_providers/metric_client_provider/metric_utils/mock/mock_simple_metric_with_overrides.h"
#include "cpio/client_providers/metric_client_provider/mock/mock_metric_client_provider.h"
#include "cpio/proto/metric_client.pb.h"
#include "public/core/interface/execution_result.h"

using google::scp::core::AsyncContext;
using google::scp::core::AsyncExecutorInterface;
using google::scp::core::AsyncOperation;
using google::scp::core::ExecutionResult;
using google::scp::core::FailureExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::async_executor::mock::MockAsyncExecutor;
using google::scp::core::test::WaitUntil;
using google::scp::cpio::client_providers::mock::MockMetricClientProvider;
using google::scp::cpio::client_providers::mock::MockSimpleMetricOverrides;
using google::scp::cpio::metric_client::MetricProto;
using google::scp::cpio::metric_client::MetricUnitProto;
using google::scp::cpio::metric_client::RecordMetricsProtoRequest;
using google::scp::cpio::metric_client::RecordMetricsProtoResponse;
using std::make_shared;
using std::shared_ptr;
using std::static_pointer_cast;
using std::string;
using std::vector;

namespace google::scp::cpio::client_providers::test {
TEST(SimpleMetricTest, Push) {
  auto mock_metric_client = make_shared<MockMetricClientProvider>();
  auto metric_name = make_shared<MetricName>("FrontEndRequestCount");
  auto metric_unit = make_shared<MetricUnit>(MetricUnit::kCount);
  auto metric_info = make_shared<MetricDefinition>(metric_name, metric_unit);
  metric_info->name_space = make_shared<MetricNamespace>("PBS");

  auto mock_async_executor = make_shared<MockAsyncExecutor>();

  shared_ptr<AsyncExecutorInterface> async_executor =
      static_pointer_cast<AsyncExecutorInterface>(mock_async_executor);

  mock_async_executor->schedule_mock = [&](const AsyncOperation& work) {
    work();
    return SuccessExecutionResult();
  };

  auto simple_metric = MockSimpleMetricOverrides(
      async_executor, mock_metric_client, metric_info);

  EXPECT_EQ(simple_metric.Init(), SuccessExecutionResult());
  EXPECT_EQ(simple_metric.Run(), SuccessExecutionResult());
  EXPECT_EQ(simple_metric.Stop(), SuccessExecutionResult());

  MetricProto metric_received;
  bool schedule_is_called = false;
  simple_metric.run_metric_push_mock =
      [&](shared_ptr<RecordMetricsProtoRequest> request) {
        schedule_is_called = true;
        metric_received.CopyFrom(request->metrics()[0]);
        return;
      };

  auto metric_value = make_shared<MetricValue>("12345");
  simple_metric.Push(metric_value);
  WaitUntil([&]() { return schedule_is_called; });

  EXPECT_EQ(metric_received.name(), *metric_name);
  EXPECT_EQ(metric_received.unit(), MetricUnitProto::METRIC_UNIT_COUNT);
  EXPECT_EQ(metric_received.value(), *metric_value);
}

TEST(SimpleMetricTest, RunMetricPush) {
  auto mock_metric_client = make_shared<MockMetricClientProvider>();
  auto metric_name = make_shared<MetricName>("FrontEndRequestCount");
  auto metric_unit = make_shared<MetricUnit>(MetricUnit::kCount);
  auto metric_info = make_shared<MetricDefinition>(metric_name, metric_unit);
  metric_info->name_space = make_shared<MetricNamespace>("PBS");

  auto mock_async_executor = make_shared<MockAsyncExecutor>();

  shared_ptr<AsyncExecutorInterface> async_executor =
      static_pointer_cast<AsyncExecutorInterface>(mock_async_executor);

  mock_async_executor->schedule_mock = [&](const AsyncOperation& work) {
    work();
    return SuccessExecutionResult();
  };

  auto simple_metric = MockSimpleMetricOverrides(
      async_executor, mock_metric_client, metric_info);

  MetricProto metric_received;
  bool record_metric_is_called = false;

  mock_metric_client->record_metric_mock =
      [&](AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>&
              context) {
        record_metric_is_called = true;
        metric_received.CopyFrom(context.request->metrics()[0]);
        context.result = FailureExecutionResult(123);
        context.Finish();
        return context.result;
      };

  auto metric_value = make_shared<MetricValue>("12345");
  simple_metric.Push(metric_value);
  WaitUntil([&]() { return record_metric_is_called; });

  EXPECT_EQ(metric_received.name(), *metric_name);
  EXPECT_EQ(metric_received.unit(), MetricUnitProto::METRIC_UNIT_COUNT);
  EXPECT_EQ(metric_received.value(), *metric_value);
}

}  // namespace google::scp::cpio::client_providers::test
