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

#include "worker_api_sapi.h"

#include <memory>
#include <mutex>
#include <string>

#include "public/core/interface/execution_result.h"

using google::scp::core::ExecutionResult;
using google::scp::core::ExecutionResultOr;
using std::lock_guard;
using std::make_shared;
using std::mutex;
using std::string;

namespace google::scp::roma::sandbox::worker_api {
ExecutionResult WorkerApiSapi::Init() noexcept {
  return sandbox_api_->Init();
}

ExecutionResult WorkerApiSapi::Run() noexcept {
  return sandbox_api_->Run();
}

ExecutionResult WorkerApiSapi::Stop() noexcept {
  return sandbox_api_->Stop();
}

ExecutionResultOr<WorkerApi::RunCodeResponse> WorkerApiSapi::RunCode(
    const WorkerApi::RunCodeRequest& request) noexcept {
  lock_guard<mutex> lock(run_code_mutex_);

  ::worker_api::WorkerParamsProto params_proto;
  params_proto.set_code(string(request.code));
  params_proto.mutable_input()->Add(request.input.begin(), request.input.end());
  for (auto&& kv : request.metadata) {
    (*params_proto.mutable_metadata())[kv.first] = kv.second;
  }

  auto result = sandbox_api_->RunCode(params_proto);
  if (!result.Successful()) {
    return result;
  }

  WorkerApi::RunCodeResponse code_response;
  code_response.response = make_shared<string>(params_proto.response());
  return code_response;
}
}  // namespace google::scp::roma::sandbox::worker_api
