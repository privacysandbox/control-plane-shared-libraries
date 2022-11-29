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

#include "aws_metric_client_provider.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <aws/monitoring/CloudWatchClient.h>
#include <aws/monitoring/CloudWatchErrors.h>
#include <aws/monitoring/model/PutMetricDataRequest.h>

#include "core/common/uuid/src/uuid.h"
#include "core/interface/async_context.h"
#include "core/interface/async_executor_interface.h"
#include "cpio/client_providers/global_cpio/src/global_cpio.h"
#include "cpio/client_providers/interface/metric_client_provider_interface.h"
#include "cpio/common/aws/src/aws_utils.h"
#include "cpio/proto/metric_client.pb.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/interface/metric_client/type_def.h"

#include "aws_metric_client_utils.h"
#include "cloud_watch_error_converter.h"
#include "error_codes.h"

using Aws::Client::AsyncCallerContext;
using Aws::Client::ClientConfiguration;
using Aws::CloudWatch::CloudWatchClient;
using Aws::CloudWatch::CloudWatchErrors;
using Aws::CloudWatch::Model::MetricDatum;
using Aws::CloudWatch::Model::PutMetricDataOutcome;
using Aws::CloudWatch::Model::PutMetricDataRequest;
using google::protobuf::Any;
using google::scp::core::AsyncContext;
using google::scp::core::AsyncExecutorInterface;
using google::scp::core::ExecutionResult;
using google::scp::core::FailureExecutionResult;
using google::scp::core::MessageRouterInterface;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::common::kZeroUuid;
using google::scp::core::errors::
    SC_AWS_METRIC_CLIENT_PROVIDER_REQUEST_PAYLOAD_OVERSIZE;
using google::scp::cpio::common::CreateClientConfiguration;
using google::scp::cpio::metric_client::RecordMetricsProtoRequest;
using google::scp::cpio::metric_client::RecordMetricsProtoResponse;
using std::bind;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

// Specifies the maximum number of HTTP connections to a single server.
static constexpr size_t kCloudwatchMaxConcurrentConnections = 50;
// The limit of AWS PutMetricDataRequest metric datum is 1000.
static constexpr size_t kAwsMetricDatumSizeLimit = 1000;
// The Aws PutMetricDataRequest payload size limit is about 560000B.
static constexpr size_t kAwsPayloadSizeLimit = 560000;
static constexpr char kAwsMetricClientProvider[] = "AwsMetricClientProvider";

namespace google::scp::cpio::client_providers {
ExecutionResult AwsMetricClientProvider::CreateClientConfiguration(
    shared_ptr<ClientConfiguration>& client_config) noexcept {
  auto region = make_shared<string>();
  auto execution_result = instance_client_provider_->GetRegion(*region);
  if (!execution_result.Successful()) {
    ERROR(kAwsMetricClientProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to get region");
    return execution_result;
  }

  client_config = common::CreateClientConfiguration(region);
  client_config->maxConnections = kCloudwatchMaxConcurrentConnections;
  return SuccessExecutionResult();
}

ExecutionResult AwsMetricClientProvider::Init() noexcept {
  auto execution_result = MetricClientProvider::Init();
  if (!execution_result.Successful()) {
    ERROR(kAwsMetricClientProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to initialize MetricClientProvider");
    return execution_result;
  }

  shared_ptr<ClientConfiguration> client_config;
  execution_result = CreateClientConfiguration(client_config);
  if (!execution_result.Successful()) {
    ERROR(kAwsMetricClientProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to create ClientConfiguration");
    return execution_result;
  }

  cloud_watch_client_ = make_shared<CloudWatchClient>(*client_config);

  return SuccessExecutionResult();
}

ExecutionResult AwsMetricClientProvider::MetricsBatchPush(
    const shared_ptr<vector<
        AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>>>&
        metric_requests_vector) noexcept {
  vector<AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>>
      context_chunk;

  PutMetricDataRequest request_chunk;
  string name_space = metric_client_options_->metric_namespace;
  request_chunk.SetNamespace(name_space.c_str());
  auto ns_base_payload = sizeof(request_chunk.GetNamespace());
  size_t chunk_payload = ns_base_payload;
  size_t chunk_size = 0;

  auto context_size = metric_requests_vector->size();
  for (size_t i = 0; i < context_size; i++) {
    auto context = metric_requests_vector->at(i);
    vector<MetricDatum> datum_list;
    auto result = AwsMetricClientUtils::ParseRequestToDatum(
        context, datum_list, kAwsMetricDatumSizeLimit);

    // Skips the context that failed in ParseRequestToDatum().
    if (!result.Successful()) {
      ERROR_CONTEXT(kAwsMetricClientProvider, context, result,
                    "Invalid metric.");
      continue;
    }

    // Single request payload size cannot be greater than kAwsPayloadSizeLimit.
    auto datums_payload =
        AwsMetricClientUtils::CalculateRequestSize(datum_list);
    if (datums_payload + ns_base_payload > kAwsPayloadSizeLimit) {
      context.result = FailureExecutionResult(
          SC_AWS_METRIC_CLIENT_PROVIDER_REQUEST_PAYLOAD_OVERSIZE);
      ERROR_CONTEXT(kAwsMetricClientProvider, context, context.result,
                    "Invalid metric.");
      context.Finish();
      continue;
    }

    // Pushes the request chunk before chunk's size or payload exceeds the
    // thresholds.
    auto datums_size = datum_list.size();
    if (chunk_size + datums_size > kAwsMetricDatumSizeLimit ||
        chunk_payload + datums_payload > kAwsPayloadSizeLimit) {
      cloud_watch_client_->PutMetricDataAsync(
          request_chunk,
          bind(&AwsMetricClientProvider::OnPutMetricDataAsyncCallback, this,
               make_shared<vector<AsyncContext<RecordMetricsProtoRequest,
                                               RecordMetricsProtoResponse>>>(
                   context_chunk),
               _1, _2, _3, _4));
      active_push_count_++;

      // Resets all chunks.
      chunk_size = 0;
      chunk_payload = ns_base_payload;
      request_chunk.SetMetricData({});
      context_chunk.clear();
    }

    chunk_size += datums_size;
    chunk_payload += datums_payload;
    for (auto& datum : datum_list) {
      request_chunk.AddMetricData(datum);
    }
    context_chunk.push_back(context);
  }

  // Pushes the remaining metrics in the chunk.
  if (!context_chunk.empty()) {
    cloud_watch_client_->PutMetricDataAsync(
        request_chunk,
        bind(&AwsMetricClientProvider::OnPutMetricDataAsyncCallback, this,
             make_shared<vector<AsyncContext<RecordMetricsProtoRequest,
                                             RecordMetricsProtoResponse>>>(
                 context_chunk),
             _1, _2, _3, _4));
    active_push_count_++;
  }

  return SuccessExecutionResult();
}

void AwsMetricClientProvider::OnPutMetricDataAsyncCallback(
    const shared_ptr<vector<
        AsyncContext<RecordMetricsProtoRequest, RecordMetricsProtoResponse>>>&
        metric_requests_vector,
    const CloudWatchClient*, const PutMetricDataRequest&,
    const PutMetricDataOutcome& outcome,
    const shared_ptr<const AsyncCallerContext>&) noexcept {
  active_push_count_--;
  if (outcome.IsSuccess()) {
    for (auto& record_metric_context : *metric_requests_vector) {
      record_metric_context.result = SuccessExecutionResult();
      record_metric_context.Finish();
    }
    return;
  }

  // TODO(b/240477800): map HttpErrorCodes to local errors. For cloudwatch,
  // watch out HttpResponseCode::REQUEST_ENTITY_TOO_LARGE.
  auto result = CloudWatchErrorConverter::ConvertCloudWatchError(
      outcome.GetError().GetErrorType(), outcome.GetError().GetMessage());
  ERROR_CONTEXT(kAwsMetricClientProvider, metric_requests_vector->back(),
                result, "The error is %s",
                outcome.GetError().GetMessage().c_str());
  for (auto& record_metric_context : *metric_requests_vector) {
    record_metric_context.result = result;
    record_metric_context.Finish();
  }
  return;
}

#ifndef TEST_CPIO
std::shared_ptr<MetricClientProviderInterface>
MetricClientProviderFactory::Create(
    const std::shared_ptr<MetricClientOptions>& options) {
  shared_ptr<AsyncExecutorInterface> async_executor;
  if (options->enable_batch_recording) {
    GlobalCpio::GetGlobalCpio()->GetAsyncExecutor(async_executor);
  }
  return make_shared<AwsMetricClientProvider>(
      options, GlobalCpio::GetGlobalCpio()->GetInstanceClientProvider(),
      async_executor, GlobalCpio::GetGlobalCpio()->GetMessageRouter());
}
#endif
}  // namespace google::scp::cpio::client_providers
