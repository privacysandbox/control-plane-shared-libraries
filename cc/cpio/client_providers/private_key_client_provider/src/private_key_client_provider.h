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

#include "core/interface/async_context.h"
#include "core/interface/message_router_interface.h"
#include "core/message_router/src/message_router.h"
#include "cpio/client_providers/interface/private_key_client_provider_interface.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"

namespace google::scp::cpio::client_providers {
/*! @copydoc PrivateKeyClientProviderInterface
 */
class PrivateKeyClientProvider : public PrivateKeyClientProviderInterface {
 public:
  virtual ~PrivateKeyClientProvider() = default;

  explicit PrivateKeyClientProvider(
      const std::shared_ptr<PrivateKeyClientOptions>&
          private_key_client_options,
      const std::shared_ptr<core::MessageRouterInterface<
          google::protobuf::Any, google::protobuf::Any>>& message_router)
      : private_key_client_options_(private_key_client_options),
        message_router_(message_router) {}

  core::ExecutionResult Init() noexcept override;

  core::ExecutionResult Run() noexcept override;

  core::ExecutionResult Stop() noexcept override;

  core::ExecutionResult ListPrivateKeysByIds(
      core::AsyncContext<private_key_client::ListPrivateKeysByIdsProtoRequest,
                         private_key_client::ListPrivateKeysByIdsProtoResponse>&
          context) noexcept override;

 protected:
  /**
   * @brief Triggered when ListPrivateKeysByIdsProtoRequest arrives.
   *
   * @param context async execution context.
   */
  virtual void OnListPrivateKeysByIds(
      core::AsyncContext<google::protobuf::Any, google::protobuf::Any>
          context) noexcept;

  /// Configurations for PrivateKeyClient.
  std::shared_ptr<PrivateKeyClientOptions> private_key_client_options_;

  /// The message router where the private key client subscribes actions.
  std::shared_ptr<core::MessageRouterInterface<google::protobuf::Any,
                                               google::protobuf::Any>>
      message_router_;
};
}  // namespace google::scp::cpio::client_providers
