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
#include <string>
#include <vector>

#include "core/interface/async_context.h"
#include "cpio/client_providers/instance_client_provider/mock/mock_instance_client_provider.h"
#include "cpio/client_providers/metric_client_provider/mock/aws/mock_cloud_watch_client.h"
#include "cpio/client_providers/metric_client_provider/src/aws/aws_metric_client_provider.h"
#include "cpio/client_providers/metric_client_provider/src/metric_client_provider.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/interface/metric_client/type_def.h"

namespace google::scp::cpio::client_providers::mock {

class MockAwsMetricClientProviderOverrides : public AwsMetricClientProvider {
 public:
  explicit MockAwsMetricClientProviderOverrides(
      const std::shared_ptr<MetricClientOptions>& metric_client_options,
      const std::shared_ptr<core::AsyncExecutorInterface>& async_executor =
          nullptr)
      : AwsMetricClientProvider(metric_client_options,
                                std::make_shared<MockInstanceClientProvider>(),
                                nullptr) {}

  std::shared_ptr<MockCloudWatchClient> GetCloudWatchClient() {
    return std::dynamic_pointer_cast<MockCloudWatchClient>(cloud_watch_client_);
  }

  std::shared_ptr<MockInstanceClientProvider> GetInstanceClientProvider() {
    return std::dynamic_pointer_cast<MockInstanceClientProvider>(
        instance_client_provider_);
  }

  std::function<core::ExecutionResult(
      const std::shared_ptr<std::vector<core::AsyncContext<
          cmrt::sdk::metric_service::v1::PutMetricsRequest,
          cmrt::sdk::metric_service::v1::PutMetricsResponse>>>&)>
      metric_batch_push_mock;

  std::function<void(
      const std::shared_ptr<std::vector<core::AsyncContext<
          cmrt::sdk::metric_service::v1::PutMetricsRequest,
          cmrt::sdk::metric_service::v1::PutMetricsResponse>>>&,
      const Aws::CloudWatch::CloudWatchClient*,
      const Aws::CloudWatch::Model::PutMetricDataRequest&,
      const Aws::CloudWatch::Model::PutMetricDataOutcome&,
      const std::shared_ptr<const Aws::Client::AsyncCallerContext>&)>
      put_metric_data_Async_callback_mock;

  core::ExecutionResult MetricsBatchPush(
      const std::shared_ptr<std::vector<core::AsyncContext<
          cmrt::sdk::metric_service::v1::PutMetricsRequest,
          cmrt::sdk::metric_service::v1::PutMetricsResponse>>>&
          metric_requests_vector) noexcept {
    if (metric_batch_push_mock) {
      return metric_batch_push_mock(metric_requests_vector);
    }
    return AwsMetricClientProvider::MetricsBatchPush(metric_requests_vector);
  }

  void OnPutMetricDataAsyncCallback(
      const std::shared_ptr<std::vector<core::AsyncContext<
          cmrt::sdk::metric_service::v1::PutMetricsRequest,
          cmrt::sdk::metric_service::v1::PutMetricsResponse>>>&
          metric_requests_vector,
      const Aws::CloudWatch::CloudWatchClient* client,
      const Aws::CloudWatch::Model::PutMetricDataRequest& put_request,
      const Aws::CloudWatch::Model::PutMetricDataOutcome& outcome,
      const std::shared_ptr<const Aws::Client::AsyncCallerContext>&
          aws_context) noexcept {
    if (put_metric_data_Async_callback_mock) {
      put_metric_data_Async_callback_mock(metric_requests_vector, client,
                                          put_request, outcome, aws_context);
      return;
    }

    return AwsMetricClientProvider::OnPutMetricDataAsyncCallback(
        metric_requests_vector, client, put_request, outcome, aws_context);
  }

  core::ExecutionResult Init() noexcept override {
    auto execution_result = AwsMetricClientProvider::Init();
    if (execution_result != core::SuccessExecutionResult()) {
      return execution_result;
    }

    cloud_watch_client_ = std::make_shared<MockCloudWatchClient>();
    return core::SuccessExecutionResult();
  }
};
}  // namespace google::scp::cpio::client_providers::mock
