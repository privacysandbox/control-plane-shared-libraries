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

#include "async_context.h"
#include "http_types.h"
#include "service_interface.h"

namespace google::scp::core {
/**
 * @brief Helper class to build http headers and parse http response body
 */
class AuthorizationHTTPRequestHelperInterface {
 public:
  virtual ~AuthorizationHTTPRequestHelperInterface() = default;

  /**
   * @brief Add authorization related headers to the headers map
   *
   * @return ExecutionResult
   */
  virtual ExecutionResult AddHeadersToRequest(const AuthorizationMetadata&,
                                              const HttpRequest&,
                                              HttpHeaders&) = 0;

  /**
   * @brief Parse response to obtain authorization related data
   *
   * @return ExecutionResult
   */
  virtual ExecutionResult ObtainAuthorizedMetadataFromResponse(
      const HttpResponse&, AuthorizedMetadata&) = 0;
};
}  // namespace google::scp::core
