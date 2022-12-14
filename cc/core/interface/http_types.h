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

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "core/common/concurrent_map/src/concurrent_map.h"

#include "type_def.h"

namespace google::scp::core {
/// Http Methods enumerator.
enum class HttpMethod {
  GET = 0,
  POST = 1,
  UNKNOWN = 1000,
};

typedef std::string Uri;

/// Keeps http headers key value pairs.
typedef std::multimap<std::string, std::string> HttpHeaders;

struct AuthContext {
  std::shared_ptr<std::string> authorized_domain;
};

/// Http request object.
struct HttpRequest {
  virtual ~HttpRequest() = default;

  /// Represents the http method.
  HttpMethod method;
  /// Represents the HTTP URI path.
  std::shared_ptr<Uri> path;
  /// Represents the query parameters, e.g.
  /// https://example.com/user?id=123&org=456, "/user" would be the path, and
  /// "id=123&org=456" would be the query parameters.
  std::shared_ptr<std::string> query;
  /// Represents the collection of all the request headers.
  std::shared_ptr<HttpHeaders> headers;
  /// Represents the body of the request.
  BytesBuffer body;
  /// Represents the context of authentication and/or authorization.
  AuthContext auth_context;
};

/// Http response object.
struct HttpResponse {
  virtual ~HttpResponse() = default;

  /// Represents the collection of all the response headers.
  std::shared_ptr<HttpHeaders> headers;
  /// Represents the body of the response.
  BytesBuffer body;
  /// Represents the http status code.
  errors::HttpStatusCode code;
};

}  // namespace google::scp::core
