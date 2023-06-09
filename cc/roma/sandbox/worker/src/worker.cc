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

#include "worker.h"

#include <unistd.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "roma/sandbox/constants/constants.h"

#include "error_codes.h"

using std::string;
using std::unordered_map;
using std::vector;

using google::scp::core::ExecutionResult;
using google::scp::core::ExecutionResultOr;
using google::scp::core::FailureExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::errors::SC_ROMA_WORKER_MISSING_CONTEXT_WHEN_EXECUTING;
using google::scp::core::errors::SC_ROMA_WORKER_MISSING_METADATA_ITEM;
using google::scp::core::errors::SC_ROMA_WORKER_WASM_NOT_SUPPORTED;
using google::scp::roma::sandbox::constants::kCodeVersion;
using google::scp::roma::sandbox::constants::kHandlerName;
using google::scp::roma::sandbox::constants::kRequestAction;
using google::scp::roma::sandbox::constants::kRequestActionExecute;
using google::scp::roma::sandbox::constants::kRequestActionLoad;
using google::scp::roma::sandbox::constants::kRequestType;
using google::scp::roma::sandbox::constants::kRequestTypeJavascript;
using google::scp::roma::sandbox::constants::kRequestTypeWasm;
using google::scp::roma::sandbox::js_engine::RomaJsEngineCompilationContext;

namespace google::scp::roma::sandbox::worker {
ExecutionResult Worker::Init() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult Worker::Run() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult Worker::Stop() noexcept {
  return SuccessExecutionResult();
}

static ExecutionResultOr<string> GetValueFromMetadata(
    const unordered_map<string, string>& metadata, const string& key) {
  if (metadata.find(key) == metadata.end()) {
    return FailureExecutionResult(SC_ROMA_WORKER_MISSING_METADATA_ITEM);
  }

  return metadata.at(key);
}

ExecutionResultOr<string> Worker::RunCode(
    const string& code, const vector<string>& input,
    const unordered_map<string, string>& metadata) {
  auto request_type_or = GetValueFromMetadata(metadata, kRequestType);
  RETURN_IF_FAILURE(request_type_or.result());

  auto code_version_or = GetValueFromMetadata(metadata, kCodeVersion);
  RETURN_IF_FAILURE(code_version_or.result());

  auto action_or = GetValueFromMetadata(metadata, kRequestAction);
  RETURN_IF_FAILURE(action_or.result());

  if (*request_type_or == kRequestTypeJavascript) {
    string handler_name = "";
    auto handler_name_or = GetValueFromMetadata(metadata, kHandlerName);
    // If we read the handler name successfully, let's store it.
    // Else if we didn't read it and the request is not a load request,
    // then return the failure.
    if (handler_name_or.result().Successful()) {
      handler_name = *handler_name_or;
    } else if (*action_or != kRequestActionLoad) {
      return handler_name_or.result();
    }

    RomaJsEngineCompilationContext context;
    if (compilation_contexts_.Contains(*code_version_or)) {
      context = compilation_contexts_.Get(*code_version_or);
    } else if (require_preload_ && *action_or != kRequestActionLoad) {
      // If we require preloads and we couldn't find a context and this is not a
      // load request, then bail out. This is an execution without a previous
      // load.
      return FailureExecutionResult(
          SC_ROMA_WORKER_MISSING_CONTEXT_WHEN_EXECUTING);
    }

    auto response_or =
        js_engine_->CompileAndRunJs(code, handler_name, input, context);
    RETURN_IF_FAILURE(response_or.result());

    compilation_contexts_.Set(*code_version_or,
                              response_or->compilation_context);

    return response_or->response;
  }

  // TODO: Handle WASM
  return FailureExecutionResult(SC_ROMA_WORKER_WASM_NOT_SUPPORTED);
}
}  // namespace google::scp::roma::sandbox::worker