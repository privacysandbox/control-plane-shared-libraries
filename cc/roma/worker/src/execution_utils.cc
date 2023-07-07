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

#include "execution_utils.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "absl/strings/str_format.h"
#include "include/v8.h"
#include "public/core/interface/execution_result.h"

#include "error_codes.h"

using google::scp::core::ExecutionResult;
using google::scp::core::FailureExecutionResult;
using google::scp::core::StatusCode;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::errors::SC_ROMA_V8_WORKER_BAD_HANDLER_NAME;
using google::scp::core::errors::SC_ROMA_V8_WORKER_CODE_COMPILE_FAILURE;
using google::scp::core::errors::SC_ROMA_V8_WORKER_SCRIPT_RUN_FAILURE;
using google::scp::roma::common::RomaString;
using google::scp::roma::common::RomaVector;
using google::scp::roma::wasm::RomaWasmListOfStringRepresentation;
using google::scp::roma::wasm::RomaWasmStringRepresentation;
using google::scp::roma::wasm::WasmDeserializer;
using google::scp::roma::wasm::WasmSerializer;
using std::shared_ptr;
using std::string;
using std::vector;

using v8::Array;
using v8::Context;
using v8::External;
using v8::Function;
using v8::FunctionCallback;
using v8::FunctionCallbackInfo;
using v8::Global;
using v8::HandleScope;
using v8::Int32;
using v8::Isolate;
using v8::JSON;
using v8::Local;
using v8::MemorySpan;
using v8::Message;
using v8::Name;
using v8::NewStringType;
using v8::Object;
using v8::ObjectTemplate;
using v8::Script;
using v8::ScriptCompiler;
using v8::String;
using v8::TryCatch;
using v8::UnboundScript;
using v8::Undefined;
using v8::Value;
using v8::WasmMemoryObject;
using v8::WasmModuleObject;

namespace google::scp::roma::worker {
static constexpr char kWasmMemory[] = "memory";
static constexpr char kWasiSnapshotPreview[] = "wasi_snapshot_preview1";
static constexpr char kWasiProcExitFunctionName[] = "proc_exit";

void ExecutionUtils::GlobalV8FunctionCallback(
    const FunctionCallbackInfo<Value>& info) {
  auto isolate = info.GetIsolate();
  Isolate::Scope isolate_scope(isolate);
  HandleScope handle_scope(isolate);

  if (info.Data().IsEmpty() || !info.Data()->IsExternal()) {
    isolate->ThrowError("Unexpected data in global callback");
    return;
  }

  // Get the user-provided function
  Local<External> data_object = Local<External>::Cast(info.Data());
  auto user_function =
      reinterpret_cast<FunctionBindingObjectBase*>(data_object->Value());

  if (user_function->Signature != FunctionBindingObjectBase::KnownSignature) {
    // This signals a bad cast. The pointer we got is not really a
    // FunctionBindingObjectBase
    isolate->ThrowError("Unexpected function in global callback");
    return;
  }

  user_function->InvokeInternalHandler(info);
}

void ExecutionUtils::GetV8Context(
    Isolate* isolate,
    const vector<shared_ptr<FunctionBindingObjectBase>>& function_bindings,
    Local<Context>& context) noexcept {
  // Create a global object template
  Local<ObjectTemplate> global_object_template = ObjectTemplate::New(isolate);
  // Add the global function bindings
  for (auto& func : function_bindings) {
    auto function_name =
        TypeConverter<string>::ToV8(isolate, func->GetFunctionName())
            .As<String>();

    // Allow retrieving the user-provided function from the FunctionCallbackInfo
    // when the C++ callback is invoked so that it can be called.
    Local<External> user_provided_function =
        External::New(isolate, reinterpret_cast<void*>(&*func));
    auto function_template = v8::FunctionTemplate::New(
        isolate, &ExecutionUtils::GlobalV8FunctionCallback,
        user_provided_function);

    // set the global function
    global_object_template->Set(function_name, function_template);
  }

  // Create a new context.
  context = Context::New(isolate, NULL, global_object_template);
}

Local<Value> ExecutionUtils::GetWasmMemoryObject(Isolate* isolate,
                                                 Local<Context>& context) {
  auto wasm_exports = context->Global()
                          ->Get(context, TypeConverter<std::string>::ToV8(
                                             isolate, kRegisteredWasmExports))
                          .ToLocalChecked()
                          .As<Object>();

  auto wasm_memory_maybe = wasm_exports->Get(
      context, TypeConverter<std::string>::ToV8(isolate, kWasmMemory));

  if (wasm_memory_maybe.IsEmpty()) {
    return Undefined(isolate);
  }

  return wasm_memory_maybe.ToLocalChecked();
}

Local<Array> ExecutionUtils::InputToLocalArgv(
    const RomaVector<RomaString>& input, bool is_wasm) noexcept {
  auto isolate = Isolate::GetCurrent();
  auto context = isolate->GetCurrentContext();

  if (is_wasm) {
    return ParseAsWasmInput(isolate, context, input);
  }

  return ExecutionUtils::ParseAsJsInput(input);
}

string ExecutionUtils::ExtractMessage(Isolate* isolate,
                                      v8::Local<v8::Message> message) noexcept {
  string exception_msg;
  TypeConverter<string>::FromV8(isolate, message->Get(), &exception_msg);
  // We want to return a message of the form:
  //
  //     line 7: Uncaught ReferenceError: blah is not defined.
  //
  int line;
  // Sometimes for multi-line errors there is no line number.
  if (!message->GetLineNumber(isolate->GetCurrentContext()).To(&line)) {
    return absl::StrFormat("%s", exception_msg);
  }

  return absl::StrFormat("line %i: %s", line, exception_msg);
}

string ExecutionUtils::DescribeError(Isolate* isolate,
                                     TryCatch* try_catch) noexcept {
  const Local<Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    return std::string();
  }

  return ExecutionUtils::ExtractMessage(isolate, message);
}

/**
 * @brief Handler for the WASI proc_exit function
 *
 * @param info
 */
static void WasiProcExit(const FunctionCallbackInfo<Value>& info) {
  (void)info;
  auto isolate = info.GetIsolate();
  isolate->TerminateExecution();
}

/**
 * @brief Register a function in the object that represents the
 * wasi_snapshot_preview1 module
 *
 * @param isolate
 * @param wasi_snapshot_preview_object
 * @param name
 * @param wasi_function
 */
static void RegisterWasiFunction(Isolate* isolate,
                                 Local<Object>& wasi_snapshot_preview_object,
                                 const std::string& name,
                                 FunctionCallback wasi_function) {
  auto context = isolate->GetCurrentContext();

  auto func_name = TypeConverter<string>::ToV8(isolate, name);
  wasi_snapshot_preview_object
      ->Set(context, func_name,
            v8::FunctionTemplate::New(isolate, wasi_function)
                ->GetFunction(context)
                .ToLocalChecked()
                .As<Object>())
      .Check();
}

/**
 * @brief Generate an object which represents the wasi_snapshot_preview1 module
 *
 * @param isolate
 * @return Local<Object>
 */
static Local<Object> GenerateWasiObject(Isolate* isolate) {
  // Register WASI runtime allowed functions
  auto wasi_snapshot_preview_object = v8::Object::New(isolate);

  RegisterWasiFunction(isolate, wasi_snapshot_preview_object,
                       kWasiProcExitFunctionName, &WasiProcExit);

  return wasi_snapshot_preview_object;
}

/**
 * @brief Register an object in the WASM imports module
 *
 * @param isolate
 * @param imports_object
 * @param name
 * @param new_object
 */
static void RegisterObjectInWasmImports(Isolate* isolate,
                                        Local<Object>& imports_object,
                                        const std::string& name,
                                        Local<Object>& new_object) {
  auto context = isolate->GetCurrentContext();

  auto obj_name = TypeConverter<string>::ToV8(isolate, name);
  imports_object->Set(context, obj_name, new_object).Check();
}

/**
 * @brief Generate an object that represents the WASM imports modules
 *
 * @param isolate
 * @return Local<Object>
 */
Local<Object> ExecutionUtils::GenerateWasmImports(Isolate* isolate) {
  auto imports_object = v8::Object::New(isolate);

  auto wasi_object = GenerateWasiObject(isolate);

  RegisterObjectInWasmImports(isolate, imports_object, kWasiSnapshotPreview,
                              wasi_object);

  return imports_object;
}

ExecutionResult ExecutionUtils::GetExecutionResult(
    const ExecutionResult& exception_result, StatusCode defined_code) noexcept {
  if (exception_result != FailureExecutionResult(SC_UNKNOWN)) {
    return exception_result;
  }

  return FailureExecutionResult(defined_code);
}

Local<Value> ExecutionUtils::ReadFromWasmMemory(Isolate* isolate,
                                                Local<Context>& context,
                                                int32_t offset,
                                                WasmDataType read_value_type) {
  if (offset < 0 || (read_value_type != WasmDataType::kUint32 &&
                     read_value_type != WasmDataType::kString &&
                     read_value_type != WasmDataType::kListOfString)) {
    return Undefined(isolate);
  }

  if (read_value_type == WasmDataType::kUint32) {
    // In this case, the offset is the value so no deserialization is needed.
    return TypeConverter<uint32_t>::ToV8(isolate, offset);
  }

  auto wasm_memory_maybe = GetWasmMemoryObject(isolate, context);
  if (wasm_memory_maybe->IsUndefined()) {
    return Undefined(isolate);
  }

  auto wasm_memory = wasm_memory_maybe.As<WasmMemoryObject>()
                         ->Buffer()
                         ->GetBackingStore()
                         ->Data();
  auto wasm_memory_size = wasm_memory_maybe.As<WasmMemoryObject>()
                              ->Buffer()
                              ->GetBackingStore()
                              ->ByteLength();
  auto wasm_memory_blob = static_cast<uint8_t*>(wasm_memory);

  Local<Value> ret_val = Undefined(isolate);

  if (read_value_type == WasmDataType::kString) {
    string read_str;
    WasmDeserializer::ReadCustomString(wasm_memory_blob, wasm_memory_size,
                                       offset, read_str);

    ret_val = TypeConverter<string>::ToV8(isolate, read_str);
  }
  if (read_value_type == WasmDataType::kListOfString) {
    vector<string> read_vec;
    WasmDeserializer::ReadCustomListOfString(wasm_memory_blob, wasm_memory_size,
                                             offset, read_vec);

    ret_val = TypeConverter<vector<string>>::ToV8(isolate, read_vec);
  }

  return ret_val;
}
}  // namespace google::scp::roma::worker
