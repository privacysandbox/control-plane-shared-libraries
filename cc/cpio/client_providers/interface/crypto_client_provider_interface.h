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
#include "cpio/proto/crypto_client.pb.h"
#include "public/core/interface/execution_result.h"

namespace google::scp::cpio {
/**
 * @brief Interface responsible for retrieving encrypter and decrypter, or
 * encrypting and decrypting payload.
 */
class CryptoClientProviderInterface : public core::ServiceInterface {
 public:
  virtual ~CryptoClientProviderInterface() = default;

  /**
   * @brief Encrypts payload using HPKE.
   *
   * @param context context of the operation.
   * @return ExecutionResult result of the operation.
   */
  virtual core::ExecutionResult HpkeEncrypt(
      core::AsyncContext<crypto_client::HpkeEncryptProtoRequest,
                         crypto_client::HpkeEncryptProtoResponse>&
          context) noexcept = 0;

  /**
   * @brief Decrypts payload using HPKE.
   *
   * @param context context of the operation.
   * @return ExecutionResult result of the operation.
   */
  virtual core::ExecutionResult HpkeDecrypt(
      core::AsyncContext<crypto_client::HpkeDecryptProtoRequest,
                         crypto_client::HpkeDecryptProtoResponse>&
          context) noexcept = 0;

  /**
   * @brief Encrypts payload using AEAD.
   *
   * @param context context of the operation.
   * @return ExecutionResult result of the operation.
   */
  virtual core::ExecutionResult AeadEncrypt(
      core::AsyncContext<crypto_client::AeadEncryptProtoRequest,
                         crypto_client::AeadEncryptProtoResponse>&
          context) noexcept = 0;

  /**
   * @brief Decrypts payload using AEAD.
   *
   * @param context context of the operation.
   * @return ExecutionResult result of the operation.
   */
  virtual core::ExecutionResult AeadDecrypt(
      core::AsyncContext<crypto_client::AeadDecryptProtoRequest,
                         crypto_client::AeadDecryptProtoResponse>&
          context) noexcept = 0;
};
}  // namespace google::scp::cpio
