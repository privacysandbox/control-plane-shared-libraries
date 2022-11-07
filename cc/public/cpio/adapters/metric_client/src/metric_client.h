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

#include "cpio/client_providers/interface/metric_client_provider_interface.h"
#include "cpio/proto/metric_client.pb.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/interface/metric_client/metric_client_interface.h"

namespace google::scp::cpio {
/*! @copydoc MetricClientInterface
 */
class MetricClient : public MetricClientInterface {
 public:
  explicit MetricClient(const std::shared_ptr<MetricClientOptions>& options);

  core::ExecutionResult Init() noexcept override;

  core::ExecutionResult Run() noexcept override;

  core::ExecutionResult Stop() noexcept override;

  core::ExecutionResult RecordMetrics(
      RecordMetricsRequest request,
      Callback<RecordMetricsResponse> callback) noexcept override;

 protected:
  /**
   * @brief Callback when OnRecordMetric results are returned.
   *
   * @param request caller's request.
   * @param callback caller's callback
   * @param record_metrics_context execution context.
   */
  void OnRecordMetricsCallback(
      const RecordMetricsRequest& request,
      Callback<RecordMetricsResponse>& callback,
      core::AsyncContext<metric_client::RecordMetricsProtoRequest,
                         metric_client::RecordMetricsProtoResponse>&
          record_metrics_context) noexcept;

  std::shared_ptr<client_providers::MetricClientProviderInterface>
      metric_client_provider_;
};
}  // namespace google::scp::cpio
