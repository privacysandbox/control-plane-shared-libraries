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

#include "core/interface/async_context.h"
#include "core/interface/service_interface.h"

namespace google::scp::core {

/**
 * @brief Request containing token query metadata
 *
 */
struct QueryTokenRequest {};

/**
 * @brief Response containing token and associated metadata
 *
 */
struct QueryTokenResponse {
  Token token;
  Timestamp token_expiration_timestamp = UINT64_MAX;
};

/**
 * @brief An interface for callers to query token from another
 * component/service.
 */
class TokenQueryInterface {
 public:
  /**
   * @brief Get the token asynchronously
   * @return ExecutionResult
   */
  virtual ExecutionResult QueryToken(
      AsyncContext<QueryTokenRequest, QueryTokenResponse>) noexcept = 0;

  virtual ~TokenQueryInterface() = default;
};

}  // namespace google::scp::core
