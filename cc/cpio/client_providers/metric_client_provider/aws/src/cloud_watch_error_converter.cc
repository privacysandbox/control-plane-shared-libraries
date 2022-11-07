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

#include "cloud_watch_error_converter.h"

#include "cpio/common/aws/src/error_codes.h"

#include "error_codes.h"

using Aws::CloudWatch::CloudWatchErrors;
using google::scp::core::ExecutionResult;
using google::scp::core::FailureExecutionResult;
using google::scp::core::errors::SC_AWS_INTERNAL_SERVICE_ERROR;
using google::scp::core::errors::SC_AWS_INVALID_CREDENTIALS;
using google::scp::core::errors::SC_AWS_INVALID_REQUEST;
using google::scp::core::errors::
    SC_AWS_METRIC_CLIENT_PROVIDER_METRIC_LIMIT_REACHED_PER_REQUEST;
using google::scp::core::errors::SC_AWS_REQUEST_LIMIT_REACHED;
using google::scp::core::errors::SC_AWS_SERVICE_UNAVAILABLE;
using google::scp::core::errors::SC_AWS_VALIDATION_FAILED;

namespace google::scp::cpio::client_providers {
ExecutionResult CloudWatchErrorConverter::ConvertCloudWatchError(
    CloudWatchErrors cloud_watch_error) {
  switch (cloud_watch_error) {
    case CloudWatchErrors::ACCESS_DENIED:
    // Gets this error when no credential supplied.
    case CloudWatchErrors::MISSING_AUTHENTICATION_TOKEN:
      return FailureExecutionResult(SC_AWS_INVALID_CREDENTIALS);
    case CloudWatchErrors::MISSING_REQUIRED_PARAMETER:
    case CloudWatchErrors::INVALID_PARAMETER_COMBINATION:
    case CloudWatchErrors::INVALID_PARAMETER_VALUE:
      return core::FailureExecutionResult(core::errors::SC_AWS_INVALID_REQUEST);
    case CloudWatchErrors::SERVICE_UNAVAILABLE:
    case CloudWatchErrors::NETWORK_CONNECTION:
      return FailureExecutionResult(SC_AWS_SERVICE_UNAVAILABLE);
    case CloudWatchErrors::LIMIT_EXCEEDED:
      return core::FailureExecutionResult(
          core::errors::
              SC_AWS_METRIC_CLIENT_PROVIDER_METRIC_LIMIT_REACHED_PER_REQUEST);
    case CloudWatchErrors::THROTTLING:
      return FailureExecutionResult(SC_AWS_REQUEST_LIMIT_REACHED);
    case CloudWatchErrors::INTERNAL_FAILURE:
    default:
      return FailureExecutionResult(SC_AWS_INTERNAL_SERVICE_ERROR);
  }
}
}  // namespace google::scp::cpio::client_providers
