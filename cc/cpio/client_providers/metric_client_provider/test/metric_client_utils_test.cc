
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

#include "cpio/client_providers/metric_client_provider/src/metric_client_utils.h"

#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <string>

#include "cpio/client_providers/metric_client_provider/src/error_codes.h"
#include "cpio/proto/metric_client.pb.h"
#include "public/core/interface/execution_result.h"

using google::scp::core::FailureExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::errors::SC_METRIC_CLIENT_PROVIDER_METRIC_NAME_NOT_SET;
using google::scp::core::errors::SC_METRIC_CLIENT_PROVIDER_METRIC_NOT_SET;
using google::scp::core::errors::SC_METRIC_CLIENT_PROVIDER_METRIC_VALUE_NOT_SET;
using google::scp::cpio::MetricUnit;
using google::scp::cpio::client_providers::MetricClientUtils;
using google::scp::cpio::metric_client::MetricProto;
using google::scp::cpio::metric_client::MetricUnitProto;
using google::scp::cpio::metric_client::RecordMetricsProtoRequest;

namespace google::scp::cpio::client_providers::test {
TEST(MetricClientUtilsTest, ConvertMetricUnit) {
  EXPECT_EQ(MetricClientUtils::ConvertToMetricUnitProto(MetricUnit::kBits),
            MetricUnitProto::METRIC_UNIT_BITS);
  EXPECT_EQ(MetricClientUtils::ConvertToMetricUnitProto(MetricUnit::kCount),
            MetricUnitProto::METRIC_UNIT_COUNT);
  EXPECT_EQ(
      MetricClientUtils::ConvertToMetricUnitProto(MetricUnit::kCountPerSecond),
      MetricUnitProto::METRIC_UNIT_COUNT_PER_SECOND);
}

TEST(MetricClientUtilsTest, NoMetric) {
  RecordMetricsProtoRequest request;
  EXPECT_EQ(MetricClientUtils::ValidateRequest(RecordMetricsProtoRequest()),
            FailureExecutionResult(SC_METRIC_CLIENT_PROVIDER_METRIC_NOT_SET));
}

TEST(MetricClientUtilsTest, NoMetricName) {
  RecordMetricsProtoRequest request;
  request.add_metrics();

  EXPECT_EQ(
      MetricClientUtils::ValidateRequest(request),
      FailureExecutionResult(SC_METRIC_CLIENT_PROVIDER_METRIC_NAME_NOT_SET));
}

TEST(MetricClientUtilsTest, NoMetricValue) {
  RecordMetricsProtoRequest request;
  auto metric = request.add_metrics();
  metric->set_name("metric1");
  EXPECT_EQ(
      MetricClientUtils::ValidateRequest(request),
      FailureExecutionResult(SC_METRIC_CLIENT_PROVIDER_METRIC_VALUE_NOT_SET));
}

TEST(MetricClientUtilsTest, OneMetricWithoutName) {
  RecordMetricsProtoRequest request;
  auto metric = request.add_metrics();
  metric->set_name("metric1");
  metric->set_value("123");
  request.add_metrics();

  EXPECT_EQ(
      MetricClientUtils::ValidateRequest(request),
      FailureExecutionResult(SC_METRIC_CLIENT_PROVIDER_METRIC_NAME_NOT_SET));
}

TEST(MetricClientUtilsTest, ValidMetric) {
  RecordMetricsProtoRequest request;
  auto metric = request.add_metrics();
  metric->set_name("metric1");
  metric->set_value("123");
  EXPECT_EQ(MetricClientUtils::ValidateRequest(request),
            SuccessExecutionResult());
}
}  // namespace google::scp::cpio::client_providers::test
