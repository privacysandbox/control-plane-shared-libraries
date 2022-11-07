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

#include <google/protobuf/util/message_differencer.h>

#include "core/interface/async_context.h"
#include "core/message_router/src/message_router.h"
#include "cpio/client_providers/interface/metric_client_provider_interface.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"

namespace google::scp::cpio::client_providers::mock {
class MockMetricClientProvider : public MetricClientProviderInterface {
 public:
  core::ExecutionResult init_result_mock = core::SuccessExecutionResult();

  core::ExecutionResult Init() noexcept override { return init_result_mock; }

  core::ExecutionResult run_result_mock = core::SuccessExecutionResult();

  core::ExecutionResult Run() noexcept override { return run_result_mock; }

  core::ExecutionResult stop_result_mock = core::SuccessExecutionResult();

  core::ExecutionResult Stop() noexcept override { return stop_result_mock; }

  std::function<core::ExecutionResult(
      core::AsyncContext<metric_client::RecordMetricsProtoRequest,
                         metric_client::RecordMetricsProtoResponse>&)>
      record_metric_mock;

  core::ExecutionResult record_metric_result_mock;
  metric_client::RecordMetricsProtoRequest record_metrics_request_mock;

  core::ExecutionResult MetricsBatchPush(
      const std::shared_ptr<std::vector<
          core::AsyncContext<metric_client::RecordMetricsProtoRequest,
                             metric_client::RecordMetricsProtoResponse>>>&
          metric_requests_vector) noexcept {
    return core::SuccessExecutionResult();
  }

  core::ExecutionResult RecordMetrics(
      core::AsyncContext<metric_client::RecordMetricsProtoRequest,
                         metric_client::RecordMetricsProtoResponse>&
          context) noexcept override {
    if (record_metric_mock) {
      return record_metric_mock(context);
    }
    google::protobuf::util::MessageDifferencer differencer;
    differencer.set_repeated_field_comparison(
        google::protobuf::util::MessageDifferencer::AS_SET);
    if (differencer.Equals(record_metrics_request_mock,
                           metric_client::RecordMetricsProtoRequest()) ||
        differencer.Equals(record_metrics_request_mock,
                           ZeroTimestampe(context.request))) {
      context.result = record_metric_result_mock;
      if (record_metric_result_mock == core::SuccessExecutionResult()) {
        context.response =
            std::make_shared<metric_client::RecordMetricsProtoResponse>();
      }
      context.Finish();
    }
    return record_metric_result_mock;
  }

  // TODO(b/253115895): figure out why IgnoreField doesn't work for
  // MessageDifferencer.
  metric_client::RecordMetricsProtoRequest ZeroTimestampe(
      std::shared_ptr<metric_client::RecordMetricsProtoRequest>& request) {
    metric_client::RecordMetricsProtoRequest output;
    output.CopyFrom(*request);
    for (auto metric = output.mutable_metrics()->begin();
         metric < output.mutable_metrics()->end(); metric++) {
      metric->set_timestamp_in_ms(0);
    }
    return output;
  }
};
}  // namespace google::scp::cpio::client_providers::mock
