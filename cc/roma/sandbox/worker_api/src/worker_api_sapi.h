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

#include <mutex>

#include "roma/sandbox/worker_api/sapi/src/worker_sandbox_api.h"
#include "roma/sandbox/worker_factory/src/worker_factory.h"

#include "worker_api.h"

namespace google::scp::roma::sandbox::worker_api {
class WorkerApiSapi : public WorkerApi {
 public:
  WorkerApiSapi(const worker::WorkerFactory::WorkerEngine& engine =
                    worker::WorkerFactory::WorkerEngine::v8,
                bool require_preload = true)
      : sandbox_api_(engine, require_preload) {}

  core::ExecutionResult Init() noexcept override;

  core::ExecutionResult Run() noexcept override;

  core::ExecutionResult Stop() noexcept override;

  core::ExecutionResultOr<WorkerApi::RunCodeResponse> RunCode(
      const WorkerApi::RunCodeRequest& request) noexcept override;

 private:
  WorkerSandboxApi sandbox_api_;
  std::mutex run_code_mutex_;
};
}  // namespace google::scp::roma::sandbox::worker_api
