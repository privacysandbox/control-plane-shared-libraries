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

#include "worker_sandbox_api.h"

#include <sys/syscall.h>

#include <linux/audit.h>

#include <chrono>

#include "roma/sandbox/worker_api/sapi/src/worker_params.pb.h"
#include "sandboxed_api/sandbox2/policy.h"
#include "sandboxed_api/sandbox2/policybuilder.h"

#include "error_codes.h"

using google::scp::core::ExecutionResult;
using google::scp::core::FailureExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::errors::SC_ROMA_WORKER_API_COULD_NOT_CREATE_IPC_PROTO;
using google::scp::core::errors::
    SC_ROMA_WORKER_API_COULD_NOT_GET_PROTO_MESSAGE_AFTER_EXECUTION;
using google::scp::core::errors::
    SC_ROMA_WORKER_API_COULD_NOT_INITIALIZE_SANDBOX;
using google::scp::core::errors::
    SC_ROMA_WORKER_API_COULD_NOT_INITIALIZE_WRAPPER_API;
using google::scp::core::errors ::
    SC_ROMA_WORKER_API_COULD_NOT_RUN_CODE_THROUGH_WRAPPER_API;
using google::scp::core::errors::SC_ROMA_WORKER_API_COULD_NOT_RUN_WRAPPER_API;
using google::scp::core::errors::SC_ROMA_WORKER_API_COULD_NOT_STOP_WRAPPER_API;

namespace google::scp::roma::sandbox::worker_api {

ExecutionResult WorkerSandboxApi::Init() noexcept {
  auto status = worker_sapi_sandbox_->Init();
  if (!status.ok()) {
    return FailureExecutionResult(
        SC_ROMA_WORKER_API_COULD_NOT_INITIALIZE_SANDBOX);
  }
  auto status_or = worker_wrapper_api_->Init(static_cast<int>(worker_engine_),
                                             require_preload_);
  if (!status_or.ok()) {
    return FailureExecutionResult(
        SC_ROMA_WORKER_API_COULD_NOT_INITIALIZE_WRAPPER_API);
  } else if (*status_or != SC_OK) {
    return FailureExecutionResult(*status_or);
  }

  return SuccessExecutionResult();
}

ExecutionResult WorkerSandboxApi::Run() noexcept {
  auto status_or = worker_wrapper_api_->Run();
  if (!status_or.ok()) {
    return FailureExecutionResult(SC_ROMA_WORKER_API_COULD_NOT_RUN_WRAPPER_API);
  } else if (*status_or != SC_OK) {
    return FailureExecutionResult(*status_or);
  }

  return SuccessExecutionResult();
}

ExecutionResult WorkerSandboxApi::Stop() noexcept {
  auto status_or = worker_wrapper_api_->Stop();
  if (!status_or.ok()) {
    return FailureExecutionResult(
        SC_ROMA_WORKER_API_COULD_NOT_STOP_WRAPPER_API);
  } else if (*status_or != SC_OK) {
    return FailureExecutionResult(*status_or);
  }

  return SuccessExecutionResult();
}

ExecutionResult WorkerSandboxApi::RunCode(
    ::worker_api::WorkerParamsProto& params) noexcept {
  auto sapi_proto =
      sapi::v::Proto<::worker_api::WorkerParamsProto>::FromMessage(params);
  if (!sapi_proto.ok()) {
    return FailureExecutionResult(
        SC_ROMA_WORKER_API_COULD_NOT_CREATE_IPC_PROTO);
  }

  auto status_or = worker_wrapper_api_->RunCode(sapi_proto->PtrBoth());
  if (!status_or.ok()) {
    return FailureExecutionResult(
        SC_ROMA_WORKER_API_COULD_NOT_RUN_CODE_THROUGH_WRAPPER_API);
  } else if (*status_or != SC_OK) {
    return FailureExecutionResult(*status_or);
  }

  auto message_or = sapi_proto->GetMessage();
  if (!message_or.ok()) {
    return FailureExecutionResult(
        SC_ROMA_WORKER_API_COULD_NOT_GET_PROTO_MESSAGE_AFTER_EXECUTION);
  }

  params = *message_or;

  return SuccessExecutionResult();
}
}  // namespace google::scp::roma::sandbox::worker_api
