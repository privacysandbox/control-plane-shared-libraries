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
#pragma once

#include "core/interface/errors.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/interface/error_codes.h"

namespace google::scp::core::errors {
/// Registers component code as 0x0211 for AwsConfigClientProvider.
REGISTER_COMPONENT_CODE(SC_AWS_CONFIG_CLIENT_PROVIDER, 0x0211)

DEFINE_ERROR_CODE(SC_AWS_CONFIG_CLIENT_PROVIDER_PARAMETER_NOT_FOUND,
                  SC_AWS_CONFIG_CLIENT_PROVIDER, 0x0001, "Parameter not found",
                  HttpStatusCode::BAD_REQUEST)
DEFINE_ERROR_CODE(SC_AWS_CONFIG_CLIENT_PROVIDER_NOT_ALL_PARAMETERS_FOUND,
                  SC_AWS_CONFIG_CLIENT_PROVIDER, 0x0002,
                  "Not all parameters found", HttpStatusCode::BAD_REQUEST)
DEFINE_ERROR_CODE(SC_AWS_CONFIG_CLIENT_PROVIDER_INVALID_PARAMETER_NAME,
                  SC_AWS_CONFIG_CLIENT_PROVIDER, 0x0003,
                  "Parameter name is invalid", HttpStatusCode::BAD_REQUEST)

MAP_TO_PUBLIC_ERROR_CODE(SC_AWS_CONFIG_CLIENT_PROVIDER_PARAMETER_NOT_FOUND,
                         SC_CPIO_ENTITY_NOT_FOUND)
MAP_TO_PUBLIC_ERROR_CODE(SC_AWS_CONFIG_CLIENT_PROVIDER_NOT_ALL_PARAMETERS_FOUND,
                         SC_CPIO_ENTITY_NOT_FOUND)
MAP_TO_PUBLIC_ERROR_CODE(SC_AWS_CONFIG_CLIENT_PROVIDER_INVALID_PARAMETER_NAME,
                         SC_CPIO_INVALID_REQUEST)
}  // namespace google::scp::core::errors
