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

#include "v8_js_engine.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "public/core/interface/execution_result.h"
#include "roma/config/src/type_converter.h"
#include "roma/sandbox/logging/src/logging.h"
#include "roma/sandbox/worker/src/worker_utils.h"
#include "roma/worker/src/execution_utils.h"

#include "error_codes.h"

using google::scp::core::ExecutionResult;
using google::scp::core::ExecutionResultOr;
using google::scp::core::FailureExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::errors::GetErrorMessage;
using google::scp::core::errors::
    SC_ROMA_V8_ENGINE_COULD_NOT_CONVERT_OUTPUT_TO_JSON;
using google::scp::core::errors::
    SC_ROMA_V8_ENGINE_COULD_NOT_CONVERT_OUTPUT_TO_STRING;
using google::scp::core::errors::SC_ROMA_V8_ENGINE_COULD_NOT_CREATE_ISOLATE;
using google::scp::core::errors::
    SC_ROMA_V8_ENGINE_COULD_NOT_FIND_HANDLER_BY_NAME;
using google::scp::core::errors::SC_ROMA_V8_ENGINE_COULD_NOT_PARSE_SCRIPT_INPUT;
using google::scp::core::errors::SC_ROMA_V8_ENGINE_ERROR_COMPILING_SCRIPT;
using google::scp::core::errors::SC_ROMA_V8_ENGINE_ERROR_INVOKING_HANDLER;
using google::scp::core::errors::SC_ROMA_V8_ENGINE_ERROR_RUNNING_SCRIPT;
using google::scp::core::errors::SC_ROMA_V8_ENGINE_ISOLATE_ALREADY_INITIALIZED;
using google::scp::core::errors::SC_ROMA_V8_ENGINE_ISOLATE_NOT_INITIALIZED;
using google::scp::roma::kDefaultExecutionTimeoutMs;
using google::scp::roma::TypeConverter;
using google::scp::roma::sandbox::js_engine::JsEngineExecutionResponse;
using google::scp::roma::sandbox::js_engine::RomaJsEngineCompilationContext;
using google::scp::roma::sandbox::worker::WorkerUtils;
using google::scp::roma::worker::ExecutionUtils;
using google::scp::roma::worker::ExecutionWatchDog;
using std::make_shared;
using std::make_unique;
using std::shared_ptr;
using std::static_pointer_cast;
using std::string;
using std::to_string;
using std::unordered_map;
using std::vector;
using v8::ArrayBuffer;
using v8::Context;
using v8::Function;
using v8::HandleScope;
using v8::Int32;
using v8::Isolate;
using v8::JSON;
using v8::Local;
using v8::MemorySpan;
using v8::Script;
using v8::String;
using v8::TryCatch;
using v8::Undefined;
using v8::Value;
using v8::WasmModuleObject;

namespace {
constexpr char kTimeoutErrorMsg[] = "execution timeout";

shared_ptr<string> GetCodeFromContext(
    const RomaJsEngineCompilationContext& context) {
  shared_ptr<string> code;

  if (context.has_context) {
    code = static_pointer_cast<string>(context.context);
  }

  return code;
}

ExecutionResult GetError(Isolate*& isolate, TryCatch& try_catch,
                         uint64_t& error_code) {
  vector<string> errors;

  errors.push_back(GetErrorMessage(error_code));

  // Checks isolate is currently terminating because of a call to
  // TerminateExecution.
  if (isolate->IsExecutionTerminating()) {
    errors.push_back(kTimeoutErrorMsg);
  }

  if (try_catch.HasCaught()) {
    string error_msg;
    if (!try_catch.Message().IsEmpty() &&
        TypeConverter<string>::FromV8(isolate, try_catch.Message()->Get(),
                                      &error_msg)) {
      errors.push_back(error_msg);
    }
  }

  string error_string;
  for (auto& e : errors) {
    error_string += "\n" + e;
  }
  _ROMA_LOG_ERROR(error_string)

  return FailureExecutionResult(error_code);
}

ExecutionResultOr<Isolate*> CreateIsolate() {
  Isolate::CreateParams params;
  params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
  auto isolate = Isolate::New(params);

  if (!isolate) {
    return FailureExecutionResult(SC_ROMA_V8_ENGINE_COULD_NOT_CREATE_ISOLATE);
  }

  return isolate;
}

}  // namespace

namespace google::scp::roma::sandbox::js_engine::v8_js_engine {

ExecutionResult V8JsEngine::Init() noexcept {
  if (v8_isolate_) {
    return FailureExecutionResult(
        SC_ROMA_V8_ENGINE_ISOLATE_ALREADY_INITIALIZED);
  }
  auto isolate_or = CreateIsolate();
  RETURN_IF_FAILURE(isolate_or.result());
  v8_isolate_ = *isolate_or;

  // Start execution_watchdog_ thread to monitor the execution time for each
  // code object in the isolate.
  execution_watchdog_ = make_unique<ExecutionWatchDog>(v8_isolate_);
  execution_watchdog_->Run();
  return SuccessExecutionResult();
}

ExecutionResult V8JsEngine::Run() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult V8JsEngine::Stop() noexcept {
  if (execution_watchdog_) {
    execution_watchdog_->Stop();
  }

  if (v8_isolate_) {
    v8_isolate_->Dispose();
    v8_isolate_ = nullptr;
  }
  return SuccessExecutionResult();
}

ExecutionResult V8JsEngine::OneTimeSetup(
    const unordered_map<string, string>& config) noexcept {
  pid_t my_pid = getpid();
  string proc_exe_path = string("/proc/") + to_string(my_pid) + "/exe";
  auto my_path = make_unique<char[]>(PATH_MAX);
  readlink(proc_exe_path.c_str(), my_path.get(), PATH_MAX);
  v8::V8::InitializeICUDefaultLocation(my_path.get());
  v8::V8::InitializeExternalStartupData(my_path.get());

  if (v8_platform_ == nullptr) {
    v8_platform_ = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(v8_platform_.get());
    v8::V8::Initialize();
  }

  return SuccessExecutionResult();
}

void V8JsEngine::StartWatchdogTimer(
    const unordered_map<string, string>& metadata) noexcept {
  // Get the timeout value from metadata. If no timeout tag is set, the default
  // value kDefaultExecutionTimeoutMs will be used.
  int timeout_ms = kDefaultExecutionTimeoutMs;
  auto timeout_str_or =
      WorkerUtils::GetValueFromMetadata(metadata, kTimeoutMsTag);
  if (timeout_str_or.result().Successful()) {
    auto timeout_int_or = WorkerUtils::ConvertStrToInt(timeout_str_or.value());
    if (timeout_int_or.result().Successful()) {
      timeout_ms = timeout_int_or.value();
    } else {
      _ROMA_LOG_ERROR(string("Timeout tag parsing with error ") +
                      GetErrorMessage(timeout_int_or.result().status_code));
    }
  } else {
    _ROMA_LOG_ERROR(string("Timeout tag fetching with error ") +
                    GetErrorMessage(timeout_str_or.result().status_code));
  }
  execution_watchdog_->StartTimer(timeout_ms);
}

ExecutionResultOr<JsEngineExecutionResponse> V8JsEngine::CompileAndRunJs(
    const string& code, const string& function_name,
    const vector<string>& input, const unordered_map<string, string>& metadata,
    const RomaJsEngineCompilationContext& context) noexcept {
  if (!v8_isolate_) {
    return FailureExecutionResult(SC_ROMA_V8_ENGINE_ISOLATE_NOT_INITIALIZED);
  }
  // Start execution watchdog to timeout the execution if it runs too long.
  StartWatchdogTimer(metadata);

  string input_code;
  RomaJsEngineCompilationContext out_context;
  // For now we just store and reuse the actual code as context.
  auto context_code = GetCodeFromContext(context);
  if (context_code) {
    input_code = *context_code;
    out_context = context;
  } else {
    input_code = code;
    out_context.has_context = true;
    out_context.context = make_shared<string>(code);
  }

  auto isolate = v8_isolate_;

  string execution_response_string;
  vector<string> errors;

  Isolate::Scope isolate_scope(isolate);
  HandleScope handle_scope(isolate);
  Local<Context> v8_context = Context::New(isolate);
  Context::Scope context_scope(v8_context);

  {
    Local<Context> context(isolate->GetCurrentContext());
    TryCatch try_catch(isolate);

    for (auto& visitor : isolate_visitors_) {
      visitor->Visit(isolate);
    }

    auto js_source =
        TypeConverter<string>::ToV8(isolate, input_code).As<String>();
    Local<Script> script;
    if (!Script::Compile(v8_context, js_source).ToLocal(&script)) {
      return GetError(isolate, try_catch,
                      SC_ROMA_V8_ENGINE_ERROR_COMPILING_SCRIPT);
    }

    Local<Value> script_result;
    if (!script->Run(v8_context).ToLocal(&script_result)) {
      return GetError(isolate, try_catch,
                      SC_ROMA_V8_ENGINE_ERROR_RUNNING_SCRIPT);
    }

    // If the function name is empty then there's nothing to execute
    if (!function_name.empty()) {
      auto handler_name =
          TypeConverter<string>::ToV8(isolate, function_name).As<String>();

      Local<Value> handler;
      if (!v8_context->Global()
               ->Get(v8_context, handler_name)
               .ToLocal(&handler) ||
          !handler->IsFunction()) {
        return GetError(isolate, try_catch,
                        SC_ROMA_V8_ENGINE_COULD_NOT_FIND_HANDLER_BY_NAME);
      }

      Local<Function> handler_func = handler.As<Function>();

      auto argc = input.size();
      Local<Value> argv[argc];
      bool failed_parsing_input = false;
      for (size_t i = 0; i < input.size(); i++) {
        auto v8_string_arg =
            TypeConverter<string>::ToV8(isolate, input.at(i)).As<String>();
        Local<Value> json_parsed_input = Undefined(isolate);
        if (JSON::Parse(context, v8_string_arg).ToLocal(&json_parsed_input)) {
          argv[i] = json_parsed_input.As<String>();
        } else {
          failed_parsing_input = true;
          break;
        }
      }

      if (failed_parsing_input) {
        return GetError(isolate, try_catch,
                        SC_ROMA_V8_ENGINE_COULD_NOT_PARSE_SCRIPT_INPUT);
      }

      Local<Value> result;
      if (!handler_func->Call(v8_context, v8_context->Global(), argc, argv)
               .ToLocal(&result)) {
        return GetError(isolate, try_catch,
                        SC_ROMA_V8_ENGINE_ERROR_INVOKING_HANDLER);
      }

      auto result_json_maybe = JSON::Stringify(v8_context, result);
      Local<String> result_json;
      if (!result_json_maybe.ToLocal(&result_json)) {
        return GetError(isolate, try_catch,
                        SC_ROMA_V8_ENGINE_COULD_NOT_CONVERT_OUTPUT_TO_JSON);
      }

      auto conversion_worked = TypeConverter<string>::FromV8(
          isolate, result_json, &execution_response_string);
      if (!conversion_worked) {
        return GetError(isolate, try_catch,
                        SC_ROMA_V8_ENGINE_COULD_NOT_CONVERT_OUTPUT_TO_STRING);
      }
    }
  }

  JsEngineExecutionResponse execution_response;
  execution_response.response = execution_response_string;
  execution_response.compilation_context = out_context;

  // End execution_watchdog_ in case it terminate the standby isolate.
  execution_watchdog_->EndTimer();
  return execution_response;
}

ExecutionResultOr<JsEngineExecutionResponse> V8JsEngine::CompileAndRunWasm(
    const string& code, const string& function_name,
    const vector<string>& input,
    const std::unordered_map<std::string, std::string>& metadata,
    const RomaJsEngineCompilationContext& context) noexcept {
  if (!v8_isolate_) {
    return FailureExecutionResult(SC_ROMA_V8_ENGINE_ISOLATE_NOT_INITIALIZED);
  }

  // Start execution watchdog to timeout the execution if it runs too long.
  StartWatchdogTimer(metadata);

  string input_code;
  RomaJsEngineCompilationContext out_context;
  // For now we just store and reuse the actual code as context.
  auto context_code = GetCodeFromContext(context);
  if (context_code) {
    input_code = *context_code;
    out_context = context;
  } else {
    input_code = code;
    out_context.has_context = true;
    out_context.context = make_shared<string>(code);
  }

  auto isolate = v8_isolate_;
  string execution_response_string;
  vector<string> errors;

  Isolate::Scope isolate_scope(isolate);
  HandleScope handle_scope(isolate);
  Local<Context> v8_context = Context::New(isolate);
  Context::Scope context_scope(v8_context);

  {
    Local<Context> context(isolate->GetCurrentContext());
    TryCatch try_catch(isolate);

    for (auto& visitor : isolate_visitors_) {
      visitor->Visit(isolate);
    }

    std::string errors;
    auto result = ExecutionUtils::CompileRunWASM(input_code, errors);
    if (!result.Successful()) {
      _ROMA_LOG_ERROR(errors);
      return result;
    }

    if (!function_name.empty()) {
      Local<Value> wasm_handler;
      result =
          ExecutionUtils::GetWasmHandler(function_name, wasm_handler, errors);
      if (!result.Successful()) {
        _ROMA_LOG_ERROR(errors);
        return result;
      }

      auto wasm_input_array =
          ExecutionUtils::ParseAsWasmInput(v8_isolate_, context, input);

      if (wasm_input_array.IsEmpty() ||
          wasm_input_array->Length() != input.size()) {
        return GetError(isolate, try_catch,
                        SC_ROMA_V8_ENGINE_COULD_NOT_PARSE_SCRIPT_INPUT);
      }

      auto input_length = wasm_input_array->Length();
      Local<Value> wasm_input[input_length];
      for (size_t i = 0; i < input_length; ++i) {
        wasm_input[i] = wasm_input_array->Get(context, i).ToLocalChecked();
      }

      auto handler_function = wasm_handler.As<Function>();

      Local<Value> wasm_result;
      if (!handler_function
               ->Call(context, context->Global(), input_length, wasm_input)
               .ToLocal(&wasm_result)) {
        return GetError(v8_isolate_, try_catch,
                        SC_ROMA_V8_ENGINE_ERROR_INVOKING_HANDLER);
      }

      auto offset = wasm_result.As<Int32>()->Value();
      auto wasm_execution_output = ExecutionUtils::ReadFromWasmMemory(
          v8_isolate_, context, offset, WasmDataType::kString);
      auto result_json_maybe = JSON::Stringify(context, wasm_execution_output);
      Local<String> result_json;
      if (!result_json_maybe.ToLocal(&result_json)) {
        return GetError(v8_isolate_, try_catch,
                        SC_ROMA_V8_ENGINE_COULD_NOT_CONVERT_OUTPUT_TO_STRING);
      }

      auto conversion_worked = TypeConverter<string>::FromV8(
          isolate, result_json, &execution_response_string);
      if (!conversion_worked) {
        return GetError(isolate, try_catch,
                        SC_ROMA_V8_ENGINE_COULD_NOT_CONVERT_OUTPUT_TO_STRING);
      }
    }
  }

  JsEngineExecutionResponse execution_response;
  execution_response.response = execution_response_string;
  execution_response.compilation_context = out_context;

  // End execution_watchdog_ in case it terminate the standby isolate.
  execution_watchdog_->EndTimer();
  return execution_response;
}
}  // namespace google::scp::roma::sandbox::js_engine::v8_js_engine
