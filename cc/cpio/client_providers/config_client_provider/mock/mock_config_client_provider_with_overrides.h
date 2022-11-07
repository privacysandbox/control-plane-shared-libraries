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

#include <functional>
#include <memory>

#include <google/protobuf/util/message_differencer.h>

#include "cpio/client_providers/config_client_provider/src/config_client_provider.h"
#include "cpio/client_providers/instance_client_provider/mock/mock_instance_client_provider.h"
#include "google/protobuf/any.pb.h"

namespace google::scp::cpio::client_providers::mock {
class MockConfigClientProviderWithOverrides : public ConfigClientProvider {
 public:
  explicit MockConfigClientProviderWithOverrides(
      const std::shared_ptr<ConfigClientOptions>& config_client_options,
      const std::shared_ptr<core::MessageRouterInterface<
          google::protobuf::Any, google::protobuf::Any>>& message_router)
      : ConfigClientProvider(config_client_options,
                             std::make_shared<MockInstanceClientProvider>(),
                             message_router) {}

  std::shared_ptr<MockInstanceClientProvider> GetInstanceClientProvider() {
    return std::dynamic_pointer_cast<MockInstanceClientProvider>(
        instance_client_provider_);
  }

  core::ExecutionResult GetParameter(
      core::AsyncContext<config_client::GetParameterProtoRequest,
                         config_client::GetParameterProtoResponse>&
          context) noexcept override {
    context.Finish();
    return core::SuccessExecutionResult();
  }
};
}  // namespace google::scp::cpio::client_providers::mock
