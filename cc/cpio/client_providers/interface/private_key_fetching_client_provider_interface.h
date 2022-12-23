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
#include "core/interface/type_def.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/interface/type_def.h"

namespace google::scp::cpio::client_providers {

/// Request for fetching private key.
struct PrivateKeyFetchingRequest {
  /// The account identity authorized to fetch the keys.
  std::shared_ptr<AccountIdentity> account_identity;
  /// The base URI of private key vending service.
  std::shared_ptr<core::Uri> private_key_service_base_uri;
  /// The region of the private key vending service.
  std::shared_ptr<std::string> service_region;

  /// The list of identifiers of the public and private key pair.
  std::shared_ptr<std::string> key_id;
};

/// Type of encryption key and how it is split.
enum class EncryptionKeyType {
  /// Single-coordinator managed key using a Tink hybrid key.
  kSinglePartyHybridKey = 0,
  /// Multi-coordinator using a Tink hybrid key, split using XOR with random
  /// data.
  kMultiPartyHybridEvenKeysplit = 1,
};

/// Represents key material and metadata associated with the key.
struct KeyData {
  /**
   * @brief Cryptographic signature of the public key material from the
   * coordinator identified by keyEncryptionKeyUri
   */
  std::shared_ptr<std::string> public_key_signature;

  /**
   * @brief URI of the cloud KMS key used to encrypt the keyMaterial (also used
   * to identify who owns the key material, and the signer of
   * publicKeySignature)
   *
   * e.g. aws-kms://arn:aws:kms:us-east-1:012345678901:key/abcd
   */
  std::shared_ptr<std::string> key_encryption_key_uri;

  /**
   * @brief The encrypted key material, of type defined by EncryptionKeyType of
   * the EncryptionKey
   */
  std::shared_ptr<std::string> key_material;
};

/// Response for fetching private key.
struct PrivateKeyFetchingResponse {
  /**
   * @brief Resource name (see <a href="https://google.aip.dev/122">AIP-122</a>)
   * representing the encryptedPrivateKey. E.g. "privateKeys/{keyid}"
   *
   */
  std::shared_ptr<std::string> resource_name;

  /// The type of key, and how it is split.
  EncryptionKeyType encryption_key_type;

  /// Tink keyset handle containing the public key material.
  std::shared_ptr<std::string> public_keyset_handle;

  /// The raw public key material, base 64 encoded.
  std::shared_ptr<std::string> public_key_material;

  /// Key expiration time in Unix Epoch milliseconds.
  core::Timestamp expiration_time_ms;

  /// List of key data. The size of key_data is matched with split parts of
  /// the private key.
  std::vector<std::shared_ptr<KeyData>> key_data;
};

/**
 * @brief Interface responsible for fetching private key.
 */
class PrivateKeyFetchingClientProviderInterface
    : public core::ServiceInterface {
 public:
  virtual ~PrivateKeyFetchingClientProviderInterface() = default;
  /**
   * @brief Fetches private key.
   *
   * @param context context of the operation.
   * @return core::ExecutionResult execution result.
   */
  virtual core::ExecutionResult FetchPrivateKey(
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
