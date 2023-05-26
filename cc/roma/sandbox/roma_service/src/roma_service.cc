/*
 * Copyright 2023 Google LLC
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

#include "roma_service.h"

#include <thread>

#include "roma/sandbox/worker_api/src/worker_api_sapi.h"
#include "roma/sandbox/worker_pool/src/worker_pool_implementation.h"

using google::scp::core::AsyncExecutor;
using google::scp::core::ExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::roma::sandbox::dispatcher::Dispatcher;
using google::scp::roma::sandbox::worker_api::WorkerApiSapi;
using google::scp::roma::sandbox::worker_pool::WorkerPoolImplementation;
using std::make_shared;
using std::thread;

namespace google::scp::roma::sandbox::roma_service {

RomaService* RomaService::instance_ = nullptr;

ExecutionResult RomaService::Init() noexcept {
  size_t concurrency = config_.NumberOfWorkers;
  if (concurrency == 0) {
    concurrency = thread::hardware_concurrency();
  }

  worker_pool_ =
      make_shared<WorkerPoolImplementation<WorkerApiSapi>>(concurrency);
  auto result = worker_pool_->Init();
  RETURN_IF_FAILURE(result);

  // TODO: Make queue_cap configurable
  async_executor_ = make_shared<AsyncExecutor>(concurrency, 100 /*queue_cap*/);
  result = async_executor_->Init();
  RETURN_IF_FAILURE(result);

  // TODO: Make max_pending_requests configurable
  dispatcher_ = make_shared<class Dispatcher>(async_executor_, worker_pool_,
                                              100 /*max_pending_requests*/);
  result = dispatcher_->Init();
  RETURN_IF_FAILURE(result);

  return SuccessExecutionResult();
}

ExecutionResult RomaService::Run() noexcept {
  auto result = async_executor_->Run();
  RETURN_IF_FAILURE(result);

  result = worker_pool_->Run();
  RETURN_IF_FAILURE(result);

  result = dispatcher_->Run();
  RETURN_IF_FAILURE(result);

  return SuccessExecutionResult();
}

ExecutionResult RomaService::Stop() noexcept {
  auto result = dispatcher_->Stop();
  RETURN_IF_FAILURE(result);

  result = worker_pool_->Stop();
  RETURN_IF_FAILURE(result);

  result = async_executor_->Stop();
  RETURN_IF_FAILURE(result);

  return SuccessExecutionResult();
}

}  // namespace google::scp::roma::sandbox::roma_service
