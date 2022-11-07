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
#include <vector>

#include "core/interface/async_context.h"
#include "core/message_router/src/message_router.h"
#include "cpio/client_providers/private_key_client_provider/src/private_key_client_provider.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"

namespace google::scp::cpio::client_providers::mock {
class MockPrivateKeyClientProviderWithOverrides
    : public PrivateKeyClientProvider {
 public:
  explicit MockPrivateKeyClientProviderWithOverrides(
      const std::shared_ptr<PrivateKeyClientOptions>&
          private_key_client_options,
      const std::shared_ptr<core::MessageRouterInterface<
          google::protobuf::Any, google::protobuf::Any>>& message_router =
          nullptr)
      : PrivateKeyClientProvider(private_key_client_options, message_router) {}

  std::function<core::ExecutionResult(
      core::AsyncContext<
          private_key_client::ListPrivateKeysByIdsProtoRequest,
          private_key_client::ListPrivateKeysByIdsProtoResponse>&)>
      list_private_keys_by_ids_mock;

  core::ExecutionResult list_private_keys_by_ids_result_mock;

  core::ExecutionResult ListPrivateKeysByIds(
      core::AsyncContext<private_key_client::ListPrivateKeysByIdsProtoRequest,
                         private_key_client::ListPrivateKeysByIdsProtoResponse>&
          context) noexcept override {
    if (list_private_keys_by_ids_mock) {
      return list_private_keys_by_ids_mock(context);
    }
    if (list_private_keys_by_ids_result_mock) {
      context.result = list_private_keys_by_ids_result_mock;
      if (list_private_keys_by_ids_result_mock ==
          core::SuccessExecutionResult()) {
        context.response = std::make_shared<
            private_key_client::ListPrivateKeysByIdsProtoResponse>();
      }
      context.Finish();
      return list_private_keys_by_ids_result_mock;
    }

    return PrivateKeyClientProvider::ListPrivateKeysByIds(context);
  }
};
}  // namespace google::scp::cpio::client_providers::mock
