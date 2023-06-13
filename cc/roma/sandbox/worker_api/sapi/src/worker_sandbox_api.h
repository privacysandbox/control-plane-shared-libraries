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

#pragma once

#include <sys/syscall.h>

#include <linux/audit.h>

#include <memory>
#include <string>
#include <vector>

#include "core/interface/service_interface.h"
#include "roma/sandbox/worker_api/sapi/src/roma_worker_wrapper_lib-sapi.sapi.h"
#include "roma/sandbox/worker_api/sapi/src/worker_params.pb.h"
#include "roma/sandbox/worker_factory/src/worker_factory.h"
#include "sandboxed_api/sandbox2/policy.h"
#include "sandboxed_api/sandbox2/policybuilder.h"

namespace google::scp::roma::sandbox::worker_api {

static constexpr int kBadFd = -1;

/**
 * @brief Class used as the API from the parent/controlling process to call into
 * a SAPI sandbox containing a roma worker.
 *
 */
class WorkerSandboxApi : public core::ServiceInterface {
 public:
  /**
   * @brief Construct a new Worker Sandbox Api object.
   *
   * @param worker_engine The JS engine type used to build the worker.
   * @param require_preload Whether code preloading is required for this engine.
   * @param native_js_function_comms_fd Filed descriptor to be used for native
   * function calls through the sandbox.
   * @param native_js_function_names The names of the functions that should be
   * registered to be available in JS.
   */
  WorkerSandboxApi(const worker::WorkerFactory::WorkerEngine& worker_engine,
                   bool require_preload, int native_js_function_comms_fd,
                   const std::vector<std::string>& native_js_function_names) {
    worker_sapi_sandbox_ = std::make_unique<WorkerSapiSandbox>();
    worker_wrapper_api_ =
        std::make_unique<WorkerWrapperApi>(worker_sapi_sandbox_.get());
    worker_engine_ = worker_engine;
    require_preload_ = require_preload;
    native_js_function_comms_fd_ = native_js_function_comms_fd;
    native_js_function_names_ = native_js_function_names;
    if (native_js_function_comms_fd_ != kBadFd) {
      sapi_native_js_function_comms_fd_ =
          std::make_unique<sapi::v::Fd>(native_js_function_comms_fd_);
    }
  }

  core::ExecutionResult Init() noexcept override;

  core::ExecutionResult Run() noexcept override;

  core::ExecutionResult Stop() noexcept override;

  /**
   * @brief Send a request to run code to a worker running within a sandbox.
   *
   * @param params Proto representing a request to the worker.
   * @return core::ExecutionResult
   */
  core::ExecutionResult RunCode(
      ::worker_api::WorkerParamsProto& params) noexcept;

 protected:
  /**
   * @brief Class to allow overwriting the policy for the SAPI sandbox.
   *
   */
  class WorkerSapiSandbox : public WorkerWrapperSandbox {
   public:
    // Build a custom sandbox policy needed proper worker operation
    std::unique_ptr<sandbox2::Policy> ModifyPolicy(
        sandbox2::PolicyBuilder*) override {
      return sandbox2::PolicyBuilder()
          .AllowRead()
          .AllowWrite()
          .AllowOpen()
          .AllowSystemMalloc()
          .AllowHandleSignals()
          .AllowExit()
          .AllowStat()
          .AllowTime()
          .AllowGetIDs()
          .AllowGetPIDs()
          .AllowReadlink()
          .AllowMmap()
          .AllowFork()
          .AllowSyscalls({__NR_tgkill, __NR_recvmsg, __NR_sendmsg, __NR_lseek,
                          __NR_nanosleep, __NR_futex, __NR_close,
                          __NR_sched_getaffinity, __NR_mprotect, __NR_clone3,
                          __NR_rseq, __NR_set_robust_list, __NR_prctl,
                          __NR_uname, __NR_pkey_alloc, __NR_madvise})
          .BuildOrDie();
    }
  };

  std::unique_ptr<WorkerSapiSandbox> worker_sapi_sandbox_;
  std::unique_ptr<WorkerWrapperApi> worker_wrapper_api_;
  worker::WorkerFactory::WorkerEngine worker_engine_;
  bool require_preload_;
  int native_js_function_comms_fd_;
  std::vector<std::string> native_js_function_names_;
  std::unique_ptr<sapi::v::Fd> sapi_native_js_function_comms_fd_;
};
}  // namespace google::scp::roma::sandbox::worker_api
