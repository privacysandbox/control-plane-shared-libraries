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

#include "cpio/client_providers/global_cpio/src/cpio_provider/lib_cpio_provider.h"
#include "cpio/client_providers/instance_client_provider/mock/mock_instance_client_provider.h"

namespace google::scp::cpio::client_providers::mock {
class MockLibCpioProviderWithOverrides : public LibCpioProvider {
 public:
  MockLibCpioProviderWithOverrides() : LibCpioProvider() {
    instance_client_provider_ = std::make_shared<MockInstanceClientProvider>();
  }

  std::shared_ptr<core::AsyncExecutorInterface> GetAsyncExecutorMember() {
    return async_executor_;
  }

  std::shared_ptr<core::HttpClientInterface> GetHttpClientMember() {
    return http_client_;
  }
};
}  // namespace google::scp::cpio::client_providers::mock
