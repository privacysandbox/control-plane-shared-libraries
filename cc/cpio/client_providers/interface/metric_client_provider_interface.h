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

#include "core/interface/async_context.h"
#include "core/interface/service_interface.h"
#include "cpio/proto/metric_client.pb.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/interface/metric_client/type_def.h"

namespace google::scp::cpio::client_providers {
/**
 * @brief Responsible to record custom metrics for clients.
 */
class MetricClientProviderInterface : public core::ServiceInterface {
 public:
  virtual ~MetricClientProviderInterface() = default;

  /**
   * @brief Records the custom Metric for Clients.
   *
   * @param record_metric_context the context of custom metric.
   * @return core::ExecutionResult the execution result of the operation.
   */
  virtual core::ExecutionResult RecordMetrics(
      core::AsyncContext<metric_client::RecordMetricsProtoRequest,
                         metric_client::RecordMetricsProtoResponse>&
          record_metric_context) noexcept = 0;
};

class MetricClientProviderFactory {
 public:
  /**
   * @brief Factory to create MetricClientProvider.
   *
   * @return std::shared_ptr<MetricClientProviderInterface> created
   * MetricClientProvider.
   */
  static std::shared_ptr<MetricClientProviderInterface> Create(
      const std::shared_ptr<MetricClientOptions>& options);
};
}  // namespace google::scp::cpio::client_providers
