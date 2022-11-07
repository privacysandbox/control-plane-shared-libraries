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
#include "core/interface/async_executor_interface.h"
#include "core/interface/message_router_interface.h"
#include "core/interface/service_interface.h"
#include "core/message_router/src/message_router.h"
#include "cpio/client_providers/interface/cpio_provider_interface.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"

using google::scp::core::AsyncExecutor;
using google::scp::core::AsyncExecutorInterface;
using google::scp::core::ExecutionResult;
using google::scp::core::MessageRouter;
using google::scp::core::MessageRouterInterface;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::common::kZeroUuid;
using std::make_shared;
using std::make_unique;
using std::unique_ptr;

static constexpr char kLibCpioProvider[] = "LibCpioProvider";
static const size_t kCpuThreadPoolThreadCount = 2;
static const size_t kCpuThreadPoolQueueSize = 100000;

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
  if (cpu_async_executor_) {
    auto execution_result = cpu_async_executor_->Stop();
    if (!execution_result.Successful()) {
      ERROR(kLibCpioProvider, kZeroUuid, kZeroUuid, execution_result,
            "Failed to stop cpu-bound asynce executor.");
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

ExecutionResult LibCpioProvider::GetCpuAsyncExecutor(
    std::shared_ptr<AsyncExecutorInterface>& cpu_async_executor) noexcept {
  if (cpu_async_executor_) {
    cpu_async_executor = cpu_async_executor_;
    return SuccessExecutionResult();
  }

  cpu_async_executor_ = make_shared<AsyncExecutor>(kCpuThreadPoolThreadCount,
                                                   kCpuThreadPoolQueueSize);
  auto execution_result = cpu_async_executor_->Init();
  if (!execution_result.Successful()) {
    ERROR(kLibCpioProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to initialize cpu-bound asynce executor.");
    return execution_result;
  }

  execution_result = cpu_async_executor_->Run();
  if (!execution_result.Successful()) {
    ERROR(kLibCpioProvider, kZeroUuid, kZeroUuid, execution_result,
          "Failed to run cpu-bound asynce executor.");
    return execution_result;
  }
  cpu_async_executor = cpu_async_executor_;
  return SuccessExecutionResult();
}

std::shared_ptr<InstanceClientProviderInterface>
LibCpioProvider::GetInstanceClientProvider() noexcept {
  return instance_client_provider_;
}

#ifndef CPIO_TESTING
unique_ptr<CpioProviderInterface> CpioProviderFactory::Create() {
  return make_unique<LibCpioProvider>();
}
#endif
}  // namespace google::scp::cpio::client_providers
