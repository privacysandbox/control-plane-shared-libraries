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

#include "roma/sandbox/worker_api/sapi/src/worker_init_params.pb.h"
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
using google::scp::core::errors::SC_ROMA_WORKER_API_COULD_NOT_RESTART_SANDBOX;
using google::scp::core::errors ::
    SC_ROMA_WORKER_API_COULD_NOT_RUN_CODE_THROUGH_WRAPPER_API;
using google::scp::core::errors::SC_ROMA_WORKER_API_COULD_NOT_RUN_WRAPPER_API;
using google::scp::core::errors::SC_ROMA_WORKER_API_COULD_NOT_STOP_WRAPPER_API;
using google::scp::core::errors::
    SC_ROMA_WORKER_API_COULD_NOT_TRANSFER_FUNCTION_FD_TO_SANDBOX;

namespace google::scp::roma::sandbox::worker_api {

ExecutionResult WorkerSandboxApi::Init() noexcept {
  auto status = worker_sapi_sandbox_->Init();
  if (!status.ok()) {
    return FailureExecutionResult(
        SC_ROMA_WORKER_API_COULD_NOT_INITIALIZE_SANDBOX);
  }

  int remote_fd = kBadFd;
  if (sapi_native_js_function_comms_fd_) {
    auto transferred = worker_sapi_sandbox_->TransferToSandboxee(
        sapi_native_js_function_comms_fd_.get());
    if (!transferred.ok()) {
      return FailureExecutionResult(
          SC_ROMA_WORKER_API_COULD_NOT_TRANSFER_FUNCTION_FD_TO_SANDBOX);
    }

    remote_fd = sapi_native_js_function_comms_fd_->GetRemoteFd();
  }

  ::worker_api::WorkerInitParamsProto worker_init_params;
  worker_init_params.set_worker_factory_js_engine(
      static_cast<int>(worker_engine_));
  worker_init_params.set_require_code_preload_for_execution(require_preload_);
  worker_init_params.set_native_js_function_comms_fd(remote_fd);
  worker_init_params.mutable_native_js_function_names()->Assign(
      native_js_function_names_.begin(), native_js_function_names_.end());
  auto sapi_proto =
      sapi::v::Proto<::worker_api::WorkerInitParamsProto>::FromMessage(
          worker_init_params);
  if (!sapi_proto.ok()) {
    return FailureExecutionResult(
        SC_ROMA_WORKER_API_COULD_NOT_CREATE_IPC_PROTO);
  }

  auto status_or = worker_wrapper_api_->Init(sapi_proto->PtrBefore());
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

  worker_sapi_sandbox_->Terminate();

  return SuccessExecutionResult();
}

ExecutionResult WorkerSandboxApi::RunCode(
    ::worker_api::WorkerParamsProto& params) noexcept {
  auto sapi_proto =
      ::sapi::v::Proto<::worker_api::WorkerParamsProto>::FromMessage(params);

  if (!sapi_proto.ok()) {
    return FailureExecutionResult(
        SC_ROMA_WORKER_API_COULD_NOT_CREATE_IPC_PROTO);
  }

  auto status_or = worker_wrapper_api_->RunCode(sapi_proto->PtrBoth());
  if (!status_or.ok()) {
    // This means that the sandbox died so we need to restart it
    if (!worker_sapi_sandbox_->is_active()) {
      auto restarted =
          worker_sapi_sandbox_->Restart(true /*attempt_graceful_exit*/);
      if (!restarted.ok()) {
        return FailureExecutionResult(
            SC_ROMA_WORKER_API_COULD_NOT_RESTART_SANDBOX);
      }
      auto result = Init();
      RETURN_IF_FAILURE(result);
      result = Run();
      RETURN_IF_FAILURE(result);
    }

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
