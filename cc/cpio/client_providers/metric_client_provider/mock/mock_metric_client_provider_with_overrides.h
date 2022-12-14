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
#include <vector>

#include "core/interface/async_context.h"
#include "core/message_router/src/message_router.h"
#include "cpio/client_providers/instance_client_provider/mock/mock_instance_client_provider.h"
#include "cpio/client_providers/metric_client_provider/src/metric_client_provider.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"

namespace google::scp::cpio::client_providers::mock {
class MockMetricClientProviderWithOverrides : public MetricClientProvider {
 public:
  explicit MockMetricClientProviderWithOverrides(
      const std::shared_ptr<core::AsyncExecutorInterface>& async_executor,
      const std::shared_ptr<MetricClientOptions>& metric_client_options,
      const std::shared_ptr<core::MessageRouterInterface<
          google::protobuf::Any, google::protobuf::Any>>& message_router =
          nullptr)
      : MetricClientProvider(async_executor, metric_client_options,
                             std::make_shared<MockInstanceClientProvider>(),
                             message_router) {}

  std::function<core::ExecutionResult(
      core::AsyncContext<metric_client::RecordMetricsProtoRequest,
                         metric_client::RecordMetricsProtoResponse>&)>
      record_metric_mock;

  std::function<core::ExecutionResult()> schedule_metric_push_mock;
  std::function<core::ExecutionResult(
      const std::shared_ptr<std::vector<
          core::AsyncContext<metric_client::RecordMetricsProtoRequest,
                             metric_client::RecordMetricsProtoResponse>>>&)>
      metrics_batch_push_mock;

  std::function<void()> schedule_metrics_helper_mock;

  core::ExecutionResult record_metric_result_mock;

  void RunMetricsBatchPush() noexcept override {
    if (schedule_metrics_helper_mock) {
      return schedule_metrics_helper_mock();
    }
    return MetricClientProvider::RunMetricsBatchPush();
  }

  core::ExecutionResult RecordMetrics(
      core::AsyncContext<metric_client::RecordMetricsProtoRequest,
                         metric_client::RecordMetricsProtoResponse>&
          context) noexcept override {
    if (record_metric_mock) {
      return record_metric_mock(context);
    }
    if (record_metric_result_mock) {
      context.result = record_metric_result_mock;
      if (record_metric_result_mock == core::SuccessExecutionResult()) {
        context.response =
            std::make_shared<metric_client::RecordMetricsProtoResponse>();
      }
      context.Finish();
      return record_metric_result_mock;
    }

    return MetricClientProvider::RecordMetrics(context);
  }

  int GetSizeMetricRequestsVector() {
    return MetricClientProvider::metric_requests_vector_.size();
  }

  core::ExecutionResult ScheduleMetricsBatchPush() noexcept override {
    if (schedule_metric_push_mock) {
      return schedule_metric_push_mock();
    }
    return MetricClientProvider::ScheduleMetricsBatchPush();
  }

  core::ExecutionResult MetricsBatchPush(
      const std::shared_ptr<std::vector<
          core::AsyncContext<metric_client::RecordMetricsProtoRequest,
                             metric_client::RecordMetricsProtoResponse>>>&
          metric_requests_vector) noexcept override {
    if (metrics_batch_push_mock) {
      return metrics_batch_push_mock(metric_requests_vector);
    }
    return core::SuccessExecutionResult();
  }
};
}  // namespace google::scp::cpio::client_providers::mock
