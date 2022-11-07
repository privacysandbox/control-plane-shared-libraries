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

#include "metric_client_utils.h"

#include <map>

#include "cpio/proto/metric_client.pb.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/interface/metric_client/type_def.h"

#include "error_codes.h"

using google::scp::core::ExecutionResult;
using google::scp::core::FailureExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::errors::SC_METRIC_CLIENT_PROVIDER_METRIC_NAME_NOT_SET;
using google::scp::core::errors::SC_METRIC_CLIENT_PROVIDER_METRIC_NOT_SET;
using google::scp::core::errors::SC_METRIC_CLIENT_PROVIDER_METRIC_VALUE_NOT_SET;
using google::scp::cpio::MetricUnit;
using google::scp::cpio::metric_client::MetricUnitProto;
using google::scp::cpio::metric_client::RecordMetricsProtoRequest;

static const std::map<MetricUnit, MetricUnitProto> kMetricUnitMap = {
    {MetricUnit::kSeconds, MetricUnitProto::METRIC_UNIT_SECONDS},
    {MetricUnit::kMicroseconds, MetricUnitProto::METRIC_UNIT_MICROSECONDS},
    {MetricUnit::kMilliseconds, MetricUnitProto::METRIC_UNIT_MILLISECONDS},
    {MetricUnit::kBits, MetricUnitProto::METRIC_UNIT_BITS},
    {MetricUnit::kKilobits, MetricUnitProto::METRIC_UNIT_KILOBITS},
    {MetricUnit::kMegabits, MetricUnitProto::METRIC_UNIT_MEGABITS},
    {MetricUnit::kGigabits, MetricUnitProto::METRIC_UNIT_GIGABITS},
    {MetricUnit::kTerabits, MetricUnitProto::METRIC_UNIT_TERABITS},
    {MetricUnit::kBytes, MetricUnitProto::METRIC_UNIT_BYTES},
    {MetricUnit::kKilobytes, MetricUnitProto::METRIC_UNIT_KILOBYTES},
    {MetricUnit::kMegabytes, MetricUnitProto::METRIC_UNIT_MEGABYTES},
    {MetricUnit::kGigabytes, MetricUnitProto::METRIC_UNIT_GIGABYTES},
    {MetricUnit::kTerabytes, MetricUnitProto::METRIC_UNIT_TERABYTES},
    {MetricUnit::kCount, MetricUnitProto::METRIC_UNIT_COUNT},
    {MetricUnit::kPercent, MetricUnitProto::METRIC_UNIT_PERCENT},
    {MetricUnit::kBitsPerSecond, MetricUnitProto::METRIC_UNIT_BITS_PER_SECOND},
    {MetricUnit::kKilobitsPerSecond,
     MetricUnitProto::METRIC_UNIT_KILOBITS_PER_SECOND},
    {MetricUnit::kMegabitsPerSecond,
     MetricUnitProto::METRIC_UNIT_MEGABITS_PER_SECOND},
    {MetricUnit::kGigabitsPerSecond,
     MetricUnitProto::METRIC_UNIT_GIGABITS_PER_SECOND},
    {MetricUnit::kTerabitsPerSecond,
     MetricUnitProto::METRIC_UNIT_TERABITS_PER_SECOND},
    {MetricUnit::kBytesPerSecond,
     MetricUnitProto::METRIC_UNIT_BYTES_PER_SECOND},
    {MetricUnit::kKilobytesPerSecond,
     MetricUnitProto::METRIC_UNIT_KILOBYTES_PER_SECOND},
    {MetricUnit::kMegabytesPerSecond,
     MetricUnitProto::METRIC_UNIT_MEGABYTES_PER_SECOND},
    {MetricUnit::kGigabytesPerSecond,
     MetricUnitProto::METRIC_UNIT_GIGABYTES_PER_SECOND},
    {MetricUnit::kTerabytesPerSecond,
     MetricUnitProto::METRIC_UNIT_TERABYTES_PER_SECOND},
    {MetricUnit::kCountPerSecond,
     MetricUnitProto::METRIC_UNIT_COUNT_PER_SECOND}};

namespace google::scp::cpio::client_providers {
MetricUnitProto MetricClientUtils::ConvertToMetricUnitProto(
    MetricUnit metric_unit) {
  if (kMetricUnitMap.find(metric_unit) != kMetricUnitMap.end()) {
    return kMetricUnitMap.at(metric_unit);
  }
  return MetricUnitProto::METRIC_UNIT_UNKNOWN;
}

ExecutionResult MetricClientUtils::ValidateRequest(
    const RecordMetricsProtoRequest& request) {
  if (request.metrics().empty()) {
    return FailureExecutionResult(SC_METRIC_CLIENT_PROVIDER_METRIC_NOT_SET);
  }
  for (auto metric : request.metrics()) {
    if (metric.name().empty()) {
      return FailureExecutionResult(
          SC_METRIC_CLIENT_PROVIDER_METRIC_NAME_NOT_SET);
    }
    if (metric.value().empty()) {
      return FailureExecutionResult(
          SC_METRIC_CLIENT_PROVIDER_METRIC_VALUE_NOT_SET);
    }
  }
  return SuccessExecutionResult();
}
}  // namespace google::scp::cpio::client_providers
