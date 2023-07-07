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

#include "execution_manager.h"

#include <string>

#include "core/interface/type_def.h"
#include "include/v8.h"
#include "roma/config/src/type_converter.h"
#include "roma/ipc/src/ipc_message.h"

#include "error_codes.h"
#include "execution_utils.h"

using google::scp::core::ExecutionResult;
using google::scp::core::FailureExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::errors::GetErrorMessage;
using google::scp::core::errors::SC_ROMA_V8_WORKER_BAD_INPUT_ARGS;
using google::scp::core::errors::SC_ROMA_V8_WORKER_BIND_UNBOUND_SCRIPT_FAILED;
using google::scp::core::errors::SC_ROMA_V8_WORKER_CODE_EXECUTION_FAILURE;
using google::scp::core::errors::SC_ROMA_V8_WORKER_FAILED_TO_PARSE_TIMEOUT_TAG;
using google::scp::core::errors::SC_ROMA_V8_WORKER_RESULT_PARSE_FAILURE;
using google::scp::core::errors::SC_ROMA_V8_WORKER_UNKNOWN_CODE_TYPE;
using google::scp::core::errors::SC_ROMA_V8_WORKER_UNKNOWN_WASM_RETURN_TYPE;
using google::scp::core::errors::SC_ROMA_V8_WORKER_UNMATCHED_CODE_VERSION_NUM;
using google::scp::core::errors::
    SC_ROMA_V8_WORKER_UNSET_ISOLATE_WITH_PRELOADED_CODE;
using google::scp::roma::kDefaultExecutionTimeoutMs;
using google::scp::roma::kTimeoutMsTag;
using google::scp::roma::common::RomaString;
using google::scp::roma::ipc::RomaCodeObj;
using google::scp::roma::worker::ExecutionUtils;
using std::make_unique;
using std::move;
using std::shared_ptr;
using std::stoi;
using std::string;
using std::vector;
using v8::Array;
using v8::Context;
using v8::External;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::Global;
using v8::HandleScope;
using v8::Int32;
using v8::Isolate;
using v8::JSON;
using v8::Local;
using v8::Message;
using v8::ObjectTemplate;
using v8::Promise;
using v8::SnapshotCreator;
using v8::StartupData;
using v8::String;
using v8::TryCatch;
using v8::UnboundScript;
using v8::Undefined;
using v8::Value;

namespace {

/**
 * @brief Get the Timeout Value object from code_obj tags.
 *
 * @param code_obj
 * @param timeout_ms
 * @return ExecutionResult
 */
ExecutionResult GetTimeoutValue(const RomaCodeObj& code_obj,
                                int& timeout_ms) noexcept {
  // Sets timeout_ms to default value. v8_execute_manager uses the
  // default value when no valid TimeoutMs tag found in code object.
  RomaString timeout_ms_value;
  RomaString timeout_ms_tag(kTimeoutMsTag);
  auto execution_result =
      code_obj.GetCodeObjTag(timeout_ms_tag, timeout_ms_value);
  RETURN_IF_FAILURE(execution_result);

  try {
    timeout_ms = stoi(timeout_ms_value.c_str());
    return SuccessExecutionResult();
  } catch (...) {
#if defined(_SCP_ROMA_LOG_ERRORS)
    std::cout << "Error GetTimeoutValue: "
              << GetErrorMessage(execution_result.status_code) << "\n"
              << std::endl;
#endif
    return FailureExecutionResult(
        SC_ROMA_V8_WORKER_FAILED_TO_PARSE_TIMEOUT_TAG);
  }
}

}  // namespace

namespace google::scp::roma::worker {

ExecutionManager::ExecutionManager(
    const JsEngineResourceConstraints& v8_resource_constraints,
    const std::vector<std::shared_ptr<FunctionBindingObjectBase>>&
        function_bindings)
    : v8_resource_constraints_(v8_resource_constraints),
      function_bindings_(function_bindings) {
  // Must add pointers that are not within the v8 heap to external_references_
  // so that the snapshot serialization works.
  external_references_.push_back(
      reinterpret_cast<intptr_t>(ExecutionUtils::GlobalV8FunctionCallback));
  for (auto& func : function_bindings) {
    external_references_.push_back(reinterpret_cast<intptr_t>(func.get()));
  }
  // Must be null terminated
  external_references_.push_back(0);
}

ExecutionResult ExecutionManager::Init() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult ExecutionManager::Run() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult ExecutionManager::Stop() noexcept {
  DisposeV8Isolate();
  return SuccessExecutionResult();
}

ExecutionResult ExecutionManager::Create(const RomaCodeObj& code_obj,
                                         RomaString& err_msg) noexcept {
  // If there's any previous data, deallocate it.
  if (startup_data_.data) {
    delete[] startup_data_.data;
  }
  startup_data_ = {nullptr, 0};
  unbound_script_.Reset();
  code_version_num_ = 0;

  // Dispose current v8_isolate_ if it's defined.
  DisposeV8Isolate();

  // SnapshotCreator doesn't work with wasm code. Current temp solution
  // doesn't create a blob for wasm and just return SuccessExecutionResult().
  if (code_obj.JsIsEmpty() && !code_obj.WasmIsEmpty()) {
    code_type_ = CodeType::kWasm;
    wasm_code_ = code_obj.wasm;
    CreateV8Isolate(external_references_.data());
    code_version_num_ = code_obj.version_num;
    return SuccessExecutionResult();
  }

  auto execution_result = ExecutionUtils::CreateSnapshot(
      startup_data_, code_obj.js, err_msg, function_bindings_,
      external_references_.data());
  if (!execution_result.Successful() &&
      !ExecutionUtils::CheckErrorWithWebAssembly(err_msg)) {
#if defined(_SCP_ROMA_LOG_ERRORS)
    std::cout << "Error CreateSnapshot:" << err_msg << "\n" << std::endl;
#endif
    return execution_result;
  }

  code_type_ = CodeType::kJs;

  // Re-create v8_isolate_. UnboundScript needs to be created in v8_isolate_.
  CreateV8Isolate(external_references_.data());

  if (!execution_result.Successful() &&
      ExecutionUtils::CheckErrorWithWebAssembly(err_msg)) {
    execution_result = ExecutionUtils::CreateUnboundScript(
        unbound_script_, v8_isolate_, code_obj.js, err_msg);
    if (!execution_result.Successful()) {
#if defined(_SCP_ROMA_LOG_ERRORS)
      std::cout << "Error CreateUnboundScript:" << err_msg << "\n" << std::endl;
#endif
      return execution_result;
    }

    code_type_ = CodeType::kJsWasmMixed;
  }

  code_version_num_ = code_obj.version_num;
  return SuccessExecutionResult();
}

ExecutionResult ExecutionManager::SetUpContextAndGetHandler(
    const RomaCodeObj& code_obj, Local<Value>& handler,
    RomaString& err_msg) noexcept {
  auto isolate = Isolate::GetCurrent();
  Local<Context> context(isolate->GetCurrentContext());
  Context::Scope context_scope(context);

  ExecutionResult execution_result;
  switch (code_type_) {
    case CodeType::kJs:
      execution_result =
          ExecutionUtils::GetJsHandler(code_obj.handler_name, handler, err_msg);
      RETURN_IF_FAILURE(execution_result);
      break;

    case CodeType::kJsWasmMixed:
      execution_result =
          ExecutionUtils::BindUnboundScript(unbound_script_, err_msg);
      RETURN_IF_FAILURE(execution_result);

      execution_result =
          ExecutionUtils::GetJsHandler(code_obj.handler_name, handler, err_msg);
      RETURN_IF_FAILURE(execution_result);
      break;

    case CodeType::kWasm:
      if (code_obj.wasm_return_type != WasmDataType::kUint32 &&
          code_obj.wasm_return_type != WasmDataType::kString &&
          code_obj.wasm_return_type != WasmDataType::kListOfString) {
        return FailureExecutionResult(
            SC_ROMA_V8_WORKER_UNKNOWN_WASM_RETURN_TYPE);
      }

      // Needs to run wasm code during the request process.
      execution_result =
          ExecutionUtils::CompileRunWASM(RomaString(wasm_code_), err_msg);
      RETURN_IF_FAILURE(execution_result);

      // Get handler value from compiled JS code.
      execution_result = ExecutionUtils::GetWasmHandler(code_obj.handler_name,
                                                        handler, err_msg);
      RETURN_IF_FAILURE(execution_result);
      break;
    default:
      return FailureExecutionResult(SC_ROMA_V8_WORKER_UNKNOWN_CODE_TYPE);
  }

  return SuccessExecutionResult();
}

ExecutionResult ExecutionManager::Process(const RomaCodeObj& code_obj,
                                          RomaString& output,
                                          RomaString& err_msg) noexcept {
  if (code_obj.version_num != code_version_num_) {
    return FailureExecutionResult(SC_ROMA_V8_WORKER_UNMATCHED_CODE_VERSION_NUM);
  }

  if (!v8_isolate_ || (!startup_data_.data && unbound_script_.IsEmpty() &&
                       wasm_code_.empty())) {
    return FailureExecutionResult(
        SC_ROMA_V8_WORKER_UNSET_ISOLATE_WITH_PRELOADED_CODE);
  }

  // Start execution_watchdog_ right before code object process.
  int timeout_ms;
  auto execution_result = GetTimeoutValue(code_obj, timeout_ms);
  if (!execution_result.Successful()) {
    timeout_ms = kDefaultExecutionTimeoutMs;
  }
  execution_watchdog_->StartTimer(v8_isolate_, timeout_ms);

  Isolate::Scope isolate_scope(v8_isolate_);
  // Create a handle scope to keep the temporary object references.
  HandleScope handle_scope(v8_isolate_);

  // Set up an exception handler before calling the Process function
  TryCatch try_catch(v8_isolate_);

  Local<Context> context = Context::New(v8_isolate_);
  Context::Scope context_scope(context);

  Local<Value> handler;
  execution_result = SetUpContextAndGetHandler(code_obj, handler, err_msg);
  if (!execution_result.Successful()) {
#if defined(_SCP_ROMA_LOG_ERRORS)
    std::cout << "Error SetUpContextAndGetHandler:" << err_msg << "\n"
              << std::endl;
#endif
    return execution_result;
  }

  // We need to do the input parsing after the modules have been compiled,
  // since in the case of WASM, the global context gets populated with the
  // WASM memory object during compilation. And we need the WASM memory for
  // proper argument handling.
  auto& input = code_obj.input;
  auto argc = input.size();
  auto argv_array =
      ExecutionUtils::InputToLocalArgv(input, code_type_ == CodeType::kWasm);
  // If argv_array size doesn't match with input. Input conversion failed.
  if (argv_array.IsEmpty() || argv_array->Length() != argc) {
    auto exception_result =
        ExecutionUtils::ReportException(&try_catch, err_msg);
    return ExecutionUtils::GetExecutionResult(exception_result,
                                              SC_ROMA_V8_WORKER_BAD_INPUT_ARGS);
  }
  Local<Value> argv[argc];
  for (size_t i = 0; i < argc; ++i) {
    argv[i] = argv_array->Get(context, i).ToLocalChecked();
  }

  Local<Function> handler_func = handler.As<Function>();
  // Invoke the process function
  Local<Value> result;
  if (!handler_func->Call(context, context->Global(), argc, argv)
           .ToLocal(&result)) {
    execution_result = ExecutionUtils::ReportException(&try_catch, err_msg);
#if defined(_SCP_ROMA_LOG_ERRORS)
    std::cout << "Error Handler Call:" << err_msg << "\n" << std::endl;
#endif

    return ExecutionUtils::GetExecutionResult(
        execution_result, SC_ROMA_V8_WORKER_CODE_EXECUTION_FAILURE);
  }

  // If this is a WASM run then we need to deserialize the returned data
  if (code_type_ == CodeType::kWasm) {
    auto offset = result.As<Int32>()->Value();
    result = ExecutionUtils::ReadFromWasmMemory(v8_isolate_, context, offset,
                                                code_obj.wasm_return_type);
  }

  if (result->IsPromise()) {
    execution_result =
        ExecutionUtils::V8PromiseHandler(v8_isolate_, result, err_msg);
#if defined(_SCP_ROMA_LOG_ERRORS)
    std::cout << "Error from V8 Promise execution: " << err_msg << "\n"
              << std::endl;
#endif
    RETURN_IF_FAILURE(execution_result);
  }

  // Fetch execution result and handle exceptions.
  auto result_json_maybe = JSON::Stringify(context, result);
  Local<String> result_json;
  if (!result_json_maybe.ToLocal(&result_json)) {
    auto result = ExecutionUtils::ReportException(&try_catch, err_msg);
    return ExecutionUtils::GetExecutionResult(
        result, SC_ROMA_V8_WORKER_RESULT_PARSE_FAILURE);
  }

  String::Utf8Value result_utf8(v8_isolate_, result_json);
  output = RomaString(*result_utf8, result_utf8.length());

  // End execution_watchdog_ in case it terminate the standby isolate.
  execution_watchdog_->EndTimer();
  return SuccessExecutionResult();
}

void ExecutionManager::GetV8HeapStatistics(
    v8::HeapStatistics& v8_heap_stats) noexcept {
  v8_isolate_->GetHeapStatistics(&v8_heap_stats);
}

void ExecutionManager::CreateV8Isolate(
    const intptr_t* external_references) noexcept {
  Isolate::CreateParams create_params;
  create_params.external_references = external_references;

  // Configure v8 resource constraints if initial_heap_size_in_mb or
  // maximum_heap_size_in_mb is nonzero.
  if (v8_resource_constraints_.initial_heap_size_in_mb > 0 ||
      v8_resource_constraints_.maximum_heap_size_in_mb > 0) {
    create_params.constraints.ConfigureDefaultsFromHeapSize(
        v8_resource_constraints_.initial_heap_size_in_mb * kMB,
        v8_resource_constraints_.maximum_heap_size_in_mb * kMB);
  }

  // Configure create_params with startup_data_ if startup_data_ is
  // available.
  if (startup_data_.raw_size > 0 && startup_data_.data != nullptr) {
    create_params.snapshot_blob = &startup_data_;
  }
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8_isolate_ = Isolate::New(create_params);

  // Start execution_watchdog_ thread to monitor the execution time for each
  // code object in the isolate.
  execution_watchdog_ = make_unique<ExecutionWatchDog>();
  execution_watchdog_->Run();
}

void ExecutionManager::DisposeV8Isolate() noexcept {
  if (execution_watchdog_) {
    execution_watchdog_->Stop();
  }

  unbound_script_.Reset();

  if (v8_isolate_) {
    v8_isolate_->Dispose();
  }
}

}  // namespace google::scp::roma::worker
