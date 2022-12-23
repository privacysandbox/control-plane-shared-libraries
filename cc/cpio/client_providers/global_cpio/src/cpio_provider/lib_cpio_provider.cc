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

#include "lib_cpio_provider.h"

#include <memory>

#include "core/async_executor/src/async_executor.h"
#include "core/common/global_logger/src/global_logger.h"
#include "core/common/uuid/src/uuid.h"
#include "core/http2_client/src/http2_client.h"
#include "core/interface/async_executor_interface.h"
#include "core/interface/http_client_interface.h"
#include "core/interface/message_router_interface.h"
#include "core/interface/service_interface.h"
#include "core/message_router/src/message_router.h"
#include "cpio/client_providers/interface/cpio_provider_interface.h"
#include "cpio/client_providers/interface/role_credentials_provider_interface.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"

using google::scp::core::AsyncExecutor;
using google::scp::core::AsyncExecutorInterface;
using google::scp::core::ExecutionResult;
using google::scp::core::HttpClient;
using google::scp::core::HttpClientInterface;
using google::scp::core::MessageRouter;
using google::scp::core::MessageRouterInterface;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::common::kZeroUuid;
using std::make_shared;
using std::make_unique;
using std::shared_ptr;
using std::unique_ptr;

static constexpr char kLibCpioProvider[] = "LibCpioProvider";
static const size_t kThreadPoolThreadCount = 2;
static const size_t kThreadPoolQueueSize = 100000;

namespace google::scp::cpio::client_providers {
// MessageRouter is not needed for native library mode.
LibCpioProvider::LibCpioProvider() : message_router_(nullptr) {
  instance_client_provider_ = InstanceClientProviderFactory::Create();
}

core::ExecutionResult LibCpioProvider::Init() noexcept {
  return instance_client_provider_->Init();
}

core::ExecutionResult LibCpioProvider::Run() noexcept {
  return instance_client_provider_->Run();
}

core::ExecutionResult LibCpioProvider::Stop() noexcept {
  if (async_executor_) {
    auto execution_result = async_executor_->Stop();
    if (!execution_result.Successful()) {
      ERROR(kLibCpioProvider, kZeroUuid, kZeroUuid, execution_result,
            "Failed to stop async executor.");
      return execution_result;
    }
  }

  if (http_client_) {
    auto execution_result = http_client_->Stop();
    if (!execution_result.Successful()) {
      ERROR(kLibCpioProvider, kZeroUuid, kZeroUuid, execution_result,
            "Failed to stop http client.");
      return execution_result;
    }
  }

  return instance_client_provider_->Stop();
}

std::shared_ptr<
    MessageRouterInterface<google::protobuf::Any, google::protobuf::Any>>
LibCpioProvider::GetMessageRouter() noexcept {
  return message_router_;
}

ExecutionResult LibCpioProvider::GetHttpClient(
    std::shared_ptr<HttpClientInterface>& http_client) noexcept {
  if (http_client_) {
    http_client = http_client_;
    return SuccessExecutionResult();
  }

  shared_ptr<AsyncExecutorInterface> async_executor;
  auto execution_result = LibCpioProvider::GetAsyncExecutor(async_executor);
  if (!execution_result.Successful()) {
    ERROR(kLibCpioProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to get asynce executor.");
    return execution_result;
  }

  http_client_ = make_shared<HttpClient>(async_executor);
  execution_result = http_client_->Init();
  if (!execution_result.Successful()) {
    ERROR(kLibCpioProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to initialize http client.");
    return execution_result;
  }

  execution_result = http_client_->Run();
  if (!execution_result.Successful()) {
    ERROR(kLibCpioProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to run http client.");
    return execution_result;
  }
  http_client = http_client_;
  return SuccessExecutionResult();
}

ExecutionResult LibCpioProvider::GetAsyncExecutor(
    std::shared_ptr<AsyncExecutorInterface>& async_executor) noexcept {
  if (async_executor_) {
    async_executor = async_executor_;
    return SuccessExecutionResult();
  }

  async_executor_ =
      make_shared<AsyncExecutor>(kThreadPoolThreadCount, kThreadPoolQueueSize);
  auto execution_result = async_executor_->Init();
  if (!execution_result.Successful()) {
    ERROR(kLibCpioProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to initialize asynce executor.");
    return execution_result;
  }

  execution_result = async_executor_->Run();
  if (!execution_result.Successful()) {
    ERROR(kLibCpioProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to run asynce executor.");
    return execution_result;
  }
  async_executor = async_executor_;
  return SuccessExecutionResult();
}

std::shared_ptr<InstanceClientProviderInterface>
LibCpioProvider::GetInstanceClientProvider() noexcept {
  return instance_client_provider_;
}

ExecutionResult LibCpioProvider::GetRoleCredentialsProvider(
    shared_ptr<RoleCredentialsProviderInterface>&
        role_credentials_provider) noexcept {
  if (role_credentials_provider) {
    role_credentials_provider = role_credentials_provider_;
    return SuccessExecutionResult();
  }

  shared_ptr<AsyncExecutorInterface> async_executor;
  auto execution_result = LibCpioProvider::GetAsyncExecutor(async_executor);
  if (!execution_result.Successful()) {
    ERROR(kLibCpioProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to get asynce executor.");
    return execution_result;
  }

  role_credentials_provider_ = RoleCredentialsProviderFactory::Create(
      GetInstanceClientProvider(), async_executor);
  execution_result = role_credentials_provider_->Init();
  if (!execution_result.Successful()) {
    ERROR(kLibCpioProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to initialize role credential provider.");
    return execution_result;
  }

  execution_result = role_credentials_provider_->Run();
  if (!execution_result.Successful()) {
    ERROR(kLibCpioProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to run role credential provider.");
    return execution_result;
  }
  role_credentials_provider = role_credentials_provider_;
  return SuccessExecutionResult();
}

#ifdef TEST_CPIO
#elif LOCAL_CPIO
#else
unique_ptr<CpioProviderInterface> CpioProviderFactory::Create(
    const shared_ptr<CpioOptions>& options) {
  return make_unique<LibCpioProvider>();
}
#endif
}  // namespace google::scp::cpio::client_providers
