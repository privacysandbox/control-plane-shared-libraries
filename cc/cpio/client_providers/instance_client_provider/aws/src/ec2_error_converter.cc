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

#include "ec2_error_converter.h"

#include <aws/ec2/EC2Client.h>

#include "cpio/common/aws/src/error_codes.h"

#include "error_codes.h"

using Aws::EC2::EC2Errors;
using google::scp::core::FailureExecutionResult;
using google::scp::core::errors::SC_AWS_INTERNAL_SERVICE_ERROR;
using google::scp::core::errors::SC_AWS_INVALID_CREDENTIALS;
using google::scp::core::errors::SC_AWS_INVALID_REQUEST;
using google::scp::core::errors::SC_AWS_REQUEST_LIMIT_REACHED;
using google::scp::core::errors::SC_AWS_SERVICE_UNAVAILABLE;
using google::scp::core::errors::SC_AWS_VALIDATION_FAILED;

namespace google::scp::cpio::client_providers {
FailureExecutionResult EC2ErrorConverter::ConvertEC2Error(
    const EC2Errors& error) {
  switch (error) {
    case EC2Errors::VALIDATION:
      return FailureExecutionResult(SC_AWS_VALIDATION_FAILED);
    case EC2Errors::ACCESS_DENIED:
      return FailureExecutionResult(SC_AWS_INVALID_CREDENTIALS);
    case EC2Errors::INVALID_PARAMETER_COMBINATION:
    case EC2Errors::INVALID_QUERY_PARAMETER:
    case EC2Errors::INVALID_PARAMETER_VALUE:
      return FailureExecutionResult(SC_AWS_INVALID_REQUEST);
    case EC2Errors::SERVICE_UNAVAILABLE:
    case EC2Errors::NETWORK_CONNECTION:
      return FailureExecutionResult(SC_AWS_SERVICE_UNAVAILABLE);
    case EC2Errors::THROTTLING:
      return FailureExecutionResult(SC_AWS_REQUEST_LIMIT_REACHED);
    case EC2Errors::INTERNAL_FAILURE:
    default:
      return FailureExecutionResult(SC_AWS_INTERNAL_SERVICE_ERROR);
  }
}
}  // namespace google::scp::cpio::client_providers
