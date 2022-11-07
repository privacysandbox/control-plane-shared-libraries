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

#include "aws_metric_client_utils.h"

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include <aws/monitoring/CloudWatchClient.h>
#include <aws/monitoring/CloudWatchErrors.h>
#include <aws/monitoring/model/PutMetricDataRequest.h>

#include "core/interface/async_context.h"
#include "cpio/proto/metric_client.pb.h"
#include "public/core/interface/execution_result.h"

#include "error_codes.h"

using Aws::CloudWatch::Model::Dimension;
using Aws::CloudWatch::Model::MetricDatum;
using Aws::CloudWatch::Model::StandardUnit;
using google::scp::core::AsyncContext;
using google::scp::core::ExecutionResult;
using google::scp::core::FailureExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::errors::
    SC_AWS_METRIC_CLIENT_PROVIDER_INVALID_METRIC_UNIT;
using google::scp::core::errors::
    SC_AWS_METRIC_CLIENT_PROVIDER_INVALID_METRIC_VALUE;
using google::scp::core::errors::
    SC_AWS_METRIC_CLIENT_PROVIDER_INVALID_TIMESTAMP;
using google::scp::core::errors::
    SC_AWS_METRIC_CLIENT_PROVIDER_METRIC_LIMIT_REACHED_PER_REQUEST;
using google::scp::core::errors::
    SC_AWS_METRIC_CLIENT_PROVIDER_OVERSIZE_DATUM_DIMENSIONS;
using google::scp::cpio::metric_client::MetricUnitProto;
using google::scp::cpio::metric_client::RecordMetricsProtoRequest;
using google::scp::cpio::metric_client::RecordMetricsProtoResponse;
using std::map;
using std::vector;
using std::chrono::duration_cast;
using std::chrono::hours;
using std::chrono::seconds;
using std::chrono::system_clock;

static constexpr int kTwoWeeksSecondsCount =
    duration_cast<seconds>(hours(24 * 14)).count();
static constexpr int kTwoHoursSecondsCount =
    duration_cast<seconds>(hours(2)).count();

static const map<MetricUnitProto, StandardUnit> kAwsMetricUnitMap = {
    {MetricUnitProto::METRIC_UNIT_UNKNOWN, StandardUnit::NOT_SET},
    {MetricUnitProto::METRIC_UNIT_SECONDS, StandardUnit::Seconds},
    {MetricUnitProto::METRIC_UNIT_MICROSECONDS, StandardUnit::Microseconds},
    {MetricUnitProto::METRIC_UNIT_MILLISECONDS, StandardUnit::Milliseconds},
    {MetricUnitProto::METRIC_UNIT_BITS, StandardUnit::Bits},
    {MetricUnitProto::METRIC_UNIT_KILOBITS, StandardUnit::Kilobits},
    {MetricUnitProto::METRIC_UNIT_MEGABITS, StandardUnit::Megabits},
    {MetricUnitProto::METRIC_UNIT_GIGABITS, StandardUnit::Gigabits},
    {MetricUnitProto::METRIC_UNIT_TERABITS, StandardUnit::Terabits},
    {MetricUnitProto::METRIC_UNIT_BYTES, StandardUnit::Bytes},
    {MetricUnitProto::METRIC_UNIT_KILOBYTES, StandardUnit::Kilobytes},
    {MetricUnitProto::METRIC_UNIT_MEGABYTES, StandardUnit::Megabytes},
    {MetricUnitProto::METRIC_UNIT_GIGABYTES, StandardUnit::Gigabytes},
    {MetricUnitProto::METRIC_UNIT_TERABYTES, StandardUnit::Terabytes},
    {MetricUnitProto::METRIC_UNIT_COUNT, StandardUnit::Count},
    {MetricUnitProto::METRIC_UNIT_PERCENT, StandardUnit::Percent},
    {MetricUnitProto::METRIC_UNIT_BITS_PER_SECOND, StandardUnit::Bits_Second},
    {MetricUnitProto::METRIC_UNIT_KILOBITS_PER_SECOND,
     StandardUnit::Kilobits_Second},
    {MetricUnitProto::METRIC_UNIT_MEGABITS_PER_SECOND,
     StandardUnit::Megabits_Second},
    {MetricUnitProto::METRIC_UNIT_GIGABITS_PER_SECOND,
     StandardUnit::Gigabits_Second},
    {MetricUnitProto::METRIC_UNIT_TERABITS_PER_SECOND,
     StandardUnit::Terabits_Second},
    {MetricUnitProto::METRIC_UNIT_BYTES_PER_SECOND, StandardUnit::Bytes_Second},
    {MetricUnitProto::METRIC_UNIT_KILOBYTES_PER_SECOND,
     StandardUnit::Kilobytes_Second},
    {MetricUnitProto::METRIC_UNIT_MEGABYTES_PER_SECOND,
     StandardUnit::Megabytes_Second},
    {MetricUnitProto::METRIC_UNIT_GIGABYTES_PER_SECOND,
     StandardUnit::Gigabytes_Second},
    {MetricUnitProto::METRIC_UNIT_TERABYTES_PER_SECOND,
     StandardUnit::Terabytes_Second},
    {MetricUnitProto::METRIC_UNIT_COUNT_PER_SECOND,
     StandardUnit::Count_Second}};

namespace google::scp::cpio::client_providers {

size_t AwsMetricClientUtils::CalculateRequestSize(
    vector<MetricDatum>& datum_list) noexcept {
  size_t payload_size = 0;
  for (const auto& datum : datum_list) {
    payload_size += sizeof(datum.GetMetricName());
    payload_size += sizeof(datum.GetTimestamp());
    payload_size += sizeof(datum.GetValue());
    payload_size += sizeof(datum.GetUnit());
    for (const auto& dim : datum.GetDimensions()) {
      payload_size += sizeof(dim.GetValue());
      payload_size += sizeof(dim.GetName());
    }
  }
  return payload_size;
}

ExecutionResult AwsMetricClientUtils::ParseRequestToDatum(
    AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>&
        record_metric_context,
    vector<MetricDatum>& datum_list, int request_metric_limit) noexcept {
  if (record_metric_context.request->metrics().size() > request_metric_limit) {
    record_metric_context.result = FailureExecutionResult(
        SC_AWS_METRIC_CLIENT_PROVIDER_METRIC_LIMIT_REACHED_PER_REQUEST);
    record_metric_context.Finish();
    return record_metric_context.result;
  }

  for (auto metric : record_metric_context.request->metrics()) {
    if (metric.labels().size() > 30) {
      record_metric_context.result = FailureExecutionResult(
          SC_AWS_METRIC_CLIENT_PROVIDER_OVERSIZE_DATUM_DIMENSIONS);
      record_metric_context.Finish();
      return record_metric_context.result;
    }

    if (metric.timestamp_in_ms() < 0) {
      record_metric_context.result = FailureExecutionResult(
          SC_AWS_METRIC_CLIENT_PROVIDER_INVALID_TIMESTAMP);
      record_metric_context.Finish();
      return record_metric_context.result;
    }

    MetricDatum datum;
    // The default value of the timestamp is the current time.
    auto metric_timestamp = Aws::Utils::DateTime(system_clock::now());
    if (metric.timestamp_in_ms() > 0) {
      auto current_time = Aws::Utils::DateTime().Now();
      metric_timestamp = Aws::Utils::DateTime(metric.timestamp_in_ms());
      auto difference =
          duration_cast<seconds>(current_time - metric_timestamp).count();
      // The valid timestamp of metric cannot be earlier than two weeks or
      // later than two hours.
      if (difference > kTwoWeeksSecondsCount ||
          difference < -kTwoHoursSecondsCount) {
        record_metric_context.result = FailureExecutionResult(
            SC_AWS_METRIC_CLIENT_PROVIDER_INVALID_TIMESTAMP);
        record_metric_context.Finish();
        return record_metric_context.result;
      }
    }
    datum.SetTimestamp(metric_timestamp);

    datum.SetMetricName(metric.name().c_str());
    try {
      auto value = std::stod(metric.value());
      datum.SetValue(value);
    } catch (...) {
      record_metric_context.result = FailureExecutionResult(
          SC_AWS_METRIC_CLIENT_PROVIDER_INVALID_METRIC_VALUE);
      record_metric_context.Finish();
      return record_metric_context.result;
    }

    auto unit = StandardUnit::NOT_SET;
    if (kAwsMetricUnitMap.find(metric.unit()) != kAwsMetricUnitMap.end()) {
      unit = kAwsMetricUnitMap.at(metric.unit());
    }
    if (unit == StandardUnit::NOT_SET) {
      record_metric_context.result = FailureExecutionResult(
          SC_AWS_METRIC_CLIENT_PROVIDER_INVALID_METRIC_UNIT);
      record_metric_context.Finish();
      return record_metric_context.result;
    }
    datum.SetUnit(unit);

    for (const auto& label : metric.labels()) {
      Dimension dimension;
      dimension.SetName(label.first.c_str());
      dimension.SetValue(label.second.c_str());
      datum.AddDimensions(dimension);
    }

    datum_list.emplace_back(datum);
  }

  return SuccessExecutionResult();
}

}  // namespace google::scp::cpio::client_providers
