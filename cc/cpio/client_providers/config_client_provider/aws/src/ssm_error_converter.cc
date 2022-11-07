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

#include "ssm_error_converter.h"

#include "cpio/common/aws/src/error_codes.h"

#include "error_codes.h"

using Aws::SSM::SSMErrors;
using google::scp::core::FailureExecutionResult;
using google::scp::core::errors::
    SC_AWS_CONFIG_CLIENT_PROVIDER_PARAMETER_NOT_FOUND;
using google::scp::core::errors::SC_AWS_INTERNAL_SERVICE_ERROR;
using google::scp::core::errors::SC_AWS_INVALID_CREDENTIALS;
using google::scp::core::errors::SC_AWS_INVALID_REQUEST;
using google::scp::core::errors::SC_AWS_REQUEST_LIMIT_REACHED;
using google::scp::core::errors::SC_AWS_SERVICE_UNAVAILABLE;
using google::scp::core::errors::SC_AWS_VALIDATION_FAILED;

namespace google::scp::cpio::client_providers {
FailureExecutionResult SSMErrorConverter::ConvertSSMError(
    const SSMErrors& error) {
  switch (error) {
    case SSMErrors::VALIDATION:
      return FailureExecutionResult(SC_AWS_VALIDATION_FAILED);
    case SSMErrors::ACCESS_DENIED:
      return FailureExecutionResult(SC_AWS_INVALID_CREDENTIALS);
    case SSMErrors::INVALID_PARAMETER_COMBINATION:
    case SSMErrors::INVALID_QUERY_PARAMETER:
    case SSMErrors::INVALID_PARAMETER_VALUE:
      return FailureExecutionResult(SC_AWS_INVALID_REQUEST);
    case SSMErrors::PARAMETER_NOT_FOUND:
      return FailureExecutionResult(
          SC_AWS_CONFIG_CLIENT_PROVIDER_PARAMETER_NOT_FOUND);
    case SSMErrors::SERVICE_UNAVAILABLE:
    case SSMErrors::NETWORK_CONNECTION:
      return FailureExecutionResult(SC_AWS_SERVICE_UNAVAILABLE);
    case SSMErrors::THROTTLING:
      return FailureExecutionResult(SC_AWS_REQUEST_LIMIT_REACHED);
    case SSMErrors::INTERNAL_FAILURE:
    default:
      return FailureExecutionResult(SC_AWS_INTERNAL_SERVICE_ERROR);
  }
}
}  // namespace google::scp::cpio::client_providers
