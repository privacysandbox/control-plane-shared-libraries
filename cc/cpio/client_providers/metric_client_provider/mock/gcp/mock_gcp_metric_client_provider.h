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

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gmock/gmock.h>

#include "core/interface/async_context.h"
#include "cpio/client_providers/interface/metric_client_provider_interface.h"
#include "google/cloud/future.h"
#include "google/cloud/monitoring/metric_client.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/proto/metric_service/v1/metric_service.pb.h"

namespace google::scp::cpio::client_providers::mock {

class MockGcpMetricClientProvider : public MetricClientProviderInterface {
 public:
  MOCK_METHOD(core::ExecutionResult, Init, (), (noexcept, override));

  MOCK_METHOD(core::ExecutionResult, Run, (), (noexcept, override));

  MOCK_METHOD(core::ExecutionResult, Stop, (), (noexcept, override));

  MOCK_METHOD(core::ExecutionResult, PutMetrics,
              ((core::AsyncContext<
                  cmrt::sdk::metric_service::v1::PutMetricsRequest,
                  cmrt::sdk::metric_service::v1::PutMetricsResponse>&)),
              (noexcept, override));
};
}  // namespace google::scp::cpio::client_providers::mock
