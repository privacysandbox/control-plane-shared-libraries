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

#include <aws/monitoring/CloudWatchClient.h>
#include <aws/monitoring/model/PutMetricDataRequest.h>

#include "core/interface/async_context.h"
#include "core/interface/async_executor_interface.h"
#include "core/interface/message_router_interface.h"
#include "cpio/client_providers/interface/instance_client_provider_interface.h"
#include "cpio/client_providers/metric_client_provider/src/metric_client_provider.h"
#include "cpio/proto/metric_client.pb.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/interface/metric_client/type_def.h"

#include "error_codes.h"

namespace google::scp::cpio::client_providers {
/*! @copydoc MetricClientProvider
 */
class AwsMetricClientProvider : public MetricClientProvider {
 public:
  /**
   * @brief Constructs a new Aws Metric Client Provider.
   *
   * @param metric_client_options the configurations for Metric Client.
   * @param instance_client_provider the Instance Client Provider.
   * @param region the optional region input. A temporary solution for PBS.
   * @param async_executor the thread pool for batch recording.
   * @param message_router the message router for server mode.
   */
  explicit AwsMetricClientProvider(
      const std::shared_ptr<MetricClientOptions>& metric_client_options,
      const std::shared_ptr<InstanceClientProviderInterface>&
          instance_client_provider,
      const std::shared_ptr<core::AsyncExecutorInterface>& async_executor =
          nullptr,
      const std::shared_ptr<core::MessageRouterInterface<
          google::protobuf::Any, google::protobuf::Any>>& message_router =
          nullptr)
      : MetricClientProvider(async_executor, metric_client_options,
                             instance_client_provider, message_router) {}

  AwsMetricClientProvider() = delete;

  core::ExecutionResult Init() noexcept override;

 protected:
  core::ExecutionResult MetricsBatchPush(
      const std::shared_ptr<std::vector<
          core::AsyncContext<metric_client::RecordMetricsProtoRequest,
                             metric_client::RecordMetricsProtoResponse>>>&
          metric_requests_vector) noexcept override;

  /**
   * @brief Creates a Client Configuration object.
   *
   * @param client_config returned Client Configuration.
   * @return core::ExecutionResult creation result.
   */
  virtual core::ExecutionResult CreateClientConfiguration(
      std::shared_ptr<Aws::Client::ClientConfiguration>&
          client_config) noexcept;

  /**
   * @brief Is called after AWS PutMetricDataAsync is completed.
   *
   * @param record_metric_context the record custom metric operation
   * context.
   * @param outcome the operation outcome of AWS PutMetricDataAsync.
   */
  virtual void OnPutMetricDataAsyncCallback(
      const std::shared_ptr<std::vector<core::AsyncContext<
          metric_client::RecordMetricsProtoRequest,
          metric_client::RecordMetricsProtoResponse>>>& metric_requests_vector,
      const Aws::CloudWatch::CloudWatchClient*,
      const Aws::CloudWatch::Model::PutMetricDataRequest&,
      const Aws::CloudWatch::Model::PutMetricDataOutcome& outcome,
      const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) noexcept;

  /// CloudWatchClient.
  std::shared_ptr<Aws::CloudWatch::CloudWatchClient> cloud_watch_client_;
};
}  // namespace google::scp::cpio::client_providers
