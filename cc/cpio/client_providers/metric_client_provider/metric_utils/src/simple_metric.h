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

#include "core/interface/async_context.h"
#include "core/interface/async_executor_interface.h"
#include "cpio/client_providers/interface/metric_client_provider_interface.h"
#include "cpio/client_providers/metric_client_provider/metric_utils/interface/simple_metric_interface.h"
#include "cpio/client_providers/metric_client_provider/metric_utils/interface/type_def.h"
#include "cpio/proto/metric_client.pb.h"
#include "public/core/interface/execution_result.h"

namespace google::scp::cpio::client_providers {
/*! @copydoc SimpleMetricInterface
 */
class SimpleMetric : public SimpleMetricInterface {
 public:
  explicit SimpleMetric(
      const std::shared_ptr<core::AsyncExecutorInterface>& async_executor,
      const std::shared_ptr<MetricClientProviderInterface>& metric_client,
      const std::shared_ptr<MetricDefinition>& metric_info)
      : async_executor_(async_executor),
        metric_client_(metric_client),
        metric_info_(metric_info) {}

  core::ExecutionResult Init() noexcept override;

  core::ExecutionResult Run() noexcept override;

  core::ExecutionResult Stop() noexcept override;

  void Push(const std::shared_ptr<MetricValue>& metric_value,
            const std::shared_ptr<MetricTag>& metric_tag) noexcept override;

 protected:
  /**
   * @brief Runs the actual metric push logic.
   *
   * @param record_metric_request The pointer to an RecordMetricsProtoRequest
   * object.
   */
  virtual void RunMetricPush(
      const std::shared_ptr<metric_client::RecordMetricsProtoRequest>
          record_metric_request) noexcept;

  /// An instance to the async executor.
  std::shared_ptr<core::AsyncExecutorInterface> async_executor_;
  /// Metric client instance.
  std::shared_ptr<MetricClientProviderInterface> metric_client_;
  /// Metric general information.
  std::shared_ptr<MetricDefinition> metric_info_;
};

}  // namespace google::scp::cpio::client_providers
