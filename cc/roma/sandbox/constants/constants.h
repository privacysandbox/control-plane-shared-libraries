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

namespace google::scp::roma::sandbox::constants {
static constexpr char kRequestType[] = "RequestType";
static constexpr char kRequestTypeJavascript[] = "JS";
static constexpr char kRequestTypeWasm[] = "WASM";

static constexpr char kHandlerName[] = "HandlerName";

static constexpr char kRequestId[] = "RequestId";
static constexpr char kCodeVersion[] = "CodeVersion";
static constexpr char kRequestAction[] = "RequestAction";
static constexpr char kRequestActionLoad[] = "Load";
static constexpr char kRequestActionExecute[] = "Execute";
static constexpr char kJsEngineOneTimeSetupWasmPagesKey[] =
    "MaxWasmNumberOfPages";

static constexpr char kFuctionBindingMetadataFunctionName[] =
    "roma.js_function_binding_name";

static constexpr char kMetadataRomaRequestId[] = "roma.request_id";

static constexpr int kCodeVersionCacheSize = 5;

static constexpr char kWasmMemPagesV8PlatformFlag[] = "--wasm_max_mem_pages=";
static constexpr size_t kMaxNumberOfWasm32BitMemPages = 65536;

// Metrics information constants

// Label for time taken to run code in the sandbox, called from outside the
// sandbox, meaning this includes serialization overhead. In nanoseconds.
static constexpr char kExecutionMetricSandboxedJsEngineCallNs[] =
    "roma.metric.sandboxed_code_run_ns";
// Label for time taken to run code inside of the sandbox, meaning we skip the
// overhead for serializing data. In nanoseconds.
static constexpr char kExecutionMetricJsEngineCallNs[] =
    "roma.metric.code_run_ns";
}  // namespace google::scp::roma::sandbox::constants
