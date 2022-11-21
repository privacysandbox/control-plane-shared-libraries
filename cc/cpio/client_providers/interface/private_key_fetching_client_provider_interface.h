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
#include <vector>

#include "core/interface/async_context.h"
#include "core/interface/service_interface.h"
#include "public/core/interface/execution_result.h"

namespace google::scp::cpio::client_providers {

/// Request for fetching private key.
struct PrivateKeyFetchingRequest {
  /// The list of identifiers of the public and private key pair.
  std::shared_ptr<std::string> key_id;
};

/// Response for fetching private key.
struct PrivateKeyFetchingResponse {
    /**
   * @brief Resource name (see <a href="https://google.aip.dev/122">AIP-122</a>)
   * representing the encryptedPrivateKey. E.g. "privateKeys/{keyid}"
   *
   */
  std::shared_ptr<std::string> resource_name;

  /// Tink-provided JSON-encoded KeysetHandle representing this private key.
  std::shared_ptr<std::string> json_encoded_key_set;
};

/**
 * @brief Interface responsible for fetching private key.
 */
class PrivateKeyFetchingClientProviderInterface
    : public core::ServiceInterface {
 public:
  virtual ~PrivateKeyFetchingClientProviderInterface() = default;
  /**
   * @brief Fetches private keys.
   *
   * @param context context of the operation.
   * @return core::ExecutionResult execution result.
   */
  virtual core::ExecutionResult FetchPrivateKeys(
      core::AsyncContext<PrivateKeyFetchingRequest, PrivateKeyFetchingResponse>&
          context) noexcept = 0;
};

class PrivateKeyFetchingClientProviderFactory {
 public:
  /**
   * @brief Factory to create PrivateKeyFetchingClientProvider.
   *
   * @return std::shared_ptr<PrivateKeyFetchingClientProviderInterface> created
   * PrivateKeyFetchingClientProvider.
   */
  static std::shared_ptr<PrivateKeyFetchingClientProviderInterface> Create();
};
}  // namespace google::scp::cpio::client_providers
