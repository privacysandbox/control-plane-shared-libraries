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

#pragma once

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "include/v8.h"
#include "public/core/interface/execution_result.h"
#include "roma/config/src/type_converter.h"
#include "roma/ipc/src/ipc_message.h"
#include "roma/wasm/src/deserializer.h"
#include "roma/wasm/src/serializer.h"
#include "roma/wasm/src/wasm_types.h"

#include "error_codes.h"

namespace google::scp::roma::worker {

static constexpr char kWebAssemblyTag[] = "WebAssembly";
static constexpr char kInstanceTag[] = "Instance";
static constexpr char kExportsTag[] = "exports";
static constexpr char kRegisteredWasmExports[] = "RomaRegisteredWasmExports";
static constexpr char kTimeoutErrorMsg[] = "execution timeout";

class ExecutionUtils {
 public:
  /**
   * @brief Compiles and runs JavaScript code object.
   *
   * @param js the string object of JavaScript code.
   * @param err_msg
   * @param[out] unbound_script this is optional output. If unbound_script is
   * provided, a local UnboundScript will be assigned to unbound_script.
   * @return core::ExecutionResult
   */
  static core::ExecutionResult CompileRunJS(
      const common::RomaString& js, common::RomaString& err_msg,
      v8::Local<v8::UnboundScript>* unbound_script = nullptr) noexcept;

  /**
   * @brief Get JS handler from context.
   *
   * @param handler_name the name of the handler.
   * @param handler the handler of the code object.
   * @param err_msg the error message to output.
   * @return core::ExecutionResult
   */
  static core::ExecutionResult GetJsHandler(
      const common::RomaString& handler_name, v8::Local<v8::Value>& handler,
      common::RomaString& err_msg) noexcept;

  /**
   * @brief Compiles and runs WASM code object.
   *
   * @param wasm the byte object of WASM code.
   * @param err_msg the error message to output.
   * @return core::ExecutionResult the execution result of JavaScript code
   * object compile and run.
   */
  template <typename StringT = common::RomaString>
  static core::ExecutionResult CompileRunWASM(const StringT& wasm,
                                              StringT& err_msg) noexcept {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(isolate);
    v8::TryCatch try_catch(isolate);
    v8::Local<v8::Context> context(isolate->GetCurrentContext());

    auto module_maybe = v8::WasmModuleObject::Compile(
        isolate, v8::MemorySpan<const uint8_t>(
                     reinterpret_cast<const unsigned char*>(wasm.c_str()),
                     wasm.length()));
    v8::Local<v8::WasmModuleObject> wasm_module;
    if (!module_maybe.ToLocal(&wasm_module)) {
      ExecutionUtils::ReportException(&try_catch, err_msg);
      return core::FailureExecutionResult(
          core::errors::SC_ROMA_V8_WORKER_WASM_COMPILE_FAILURE);
    }

    v8::Local<v8::Value> web_assembly;
    if (!context->Global()
             ->Get(context, v8::String::NewFromUtf8(isolate, kWebAssemblyTag)
                                .ToLocalChecked())
             .ToLocal(&web_assembly)) {
      ExecutionUtils::ReportException(&try_catch, err_msg);
      return core::FailureExecutionResult(
          core::errors::SC_ROMA_V8_WORKER_WASM_OBJECT_CREATION_FAILURE);
    }

    v8::Local<v8::Value> wasm_instance;
    if (!web_assembly.As<v8::Object>()
             ->Get(context, v8::String::NewFromUtf8(isolate, kInstanceTag)
                                .ToLocalChecked())
             .ToLocal(&wasm_instance)) {
      ExecutionUtils::ReportException(&try_catch, err_msg);
      return core::FailureExecutionResult(
          core::errors::SC_ROMA_V8_WORKER_WASM_OBJECT_CREATION_FAILURE);
    }

    auto wasm_imports = GenerateWasmImports(isolate);

    v8::Local<v8::Value> instance_args[] = {wasm_module, wasm_imports};
    v8::Local<v8::Value> wasm_construct;
    if (!wasm_instance.As<v8::Object>()
             ->CallAsConstructor(context, 2, instance_args)
             .ToLocal(&wasm_construct)) {
      ExecutionUtils::ReportException(&try_catch, err_msg);
      return core::FailureExecutionResult(
          core::errors::SC_ROMA_V8_WORKER_WASM_OBJECT_CREATION_FAILURE);
    }

    v8::Local<v8::Value> wasm_exports;
    if (!wasm_construct.As<v8::Object>()
             ->Get(
                 context,
                 v8::String::NewFromUtf8(isolate, kExportsTag).ToLocalChecked())
             .ToLocal(&wasm_exports)) {
      ExecutionUtils::ReportException(&try_catch, err_msg);
      return core::FailureExecutionResult(
          core::errors::SC_ROMA_V8_WORKER_WASM_OBJECT_CREATION_FAILURE);
    }

    // Register wasm_exports object in context.
    if (!context->Global()
             ->Set(context,
                   v8::String::NewFromUtf8(isolate, kRegisteredWasmExports)
                       .ToLocalChecked(),
                   wasm_exports)
             .ToChecked()) {
      ExecutionUtils::ReportException(&try_catch, err_msg);
      return core::FailureExecutionResult(
          core::errors::SC_ROMA_V8_WORKER_WASM_OBJECT_CREATION_FAILURE);
    }

    return core::SuccessExecutionResult();
  }

  /**
   * @brief Get handler from WASM export object.
   *
   * @param handler_name the name of the handler.
   * @param handler the handler of the code object.
   * @param err_msg the error message to output.
   * @return core::ExecutionResult
   */
  template <typename StringT = common::RomaString>
  static core::ExecutionResult GetWasmHandler(const StringT& handler_name,
                                              v8::Local<v8::Value>& handler,
                                              StringT& err_msg) noexcept {
    auto isolate = v8::Isolate::GetCurrent();
    v8::TryCatch try_catch(isolate);
    v8::Local<v8::Context> context(isolate->GetCurrentContext());

    // Get wasm export object.
    v8::Local<v8::Value> wasm_exports;
    if (!context->Global()
             ->Get(context,
                   v8::String::NewFromUtf8(isolate, kRegisteredWasmExports)
                       .ToLocalChecked())
             .ToLocal(&wasm_exports)) {
      ExecutionUtils::ReportException(&try_catch, err_msg);
      return core::FailureExecutionResult(
          core::errors::SC_ROMA_V8_WORKER_WASM_OBJECT_RETRIEVAL_FAILURE);
    }

    // Fetch out the handler name from code object.
    auto str = std::string(handler_name.c_str(), handler_name.size());
    v8::Local<v8::String> local_name =
        TypeConverter<std::string>::ToV8(isolate, str).As<v8::String>();

    // If there is no handler function, or if it is not a function,
    // bail out
    if (!wasm_exports.As<v8::Object>()
             ->Get(context, local_name)
             .ToLocal(&handler) ||
        !handler->IsFunction()) {
      auto exception_result =
          ExecutionUtils::ReportException(&try_catch, err_msg);
      return GetExecutionResult(
          exception_result,
          core::errors::SC_ROMA_V8_WORKER_HANDLER_INVALID_FUNCTION);
    }

    return core::SuccessExecutionResult();
  }

  /**
   * @brief Reports the caught exception from v8 isolate to error
   * message, and returns associated execution result.
   *
   * @param try_catch the pointer to v8 try catch object.
   * @param err_msg the reference to error message.
   * @return core::ExecutionResult
   */
  template <typename StringT = common::RomaString>
  static core::ExecutionResult ReportException(v8::TryCatch* try_catch,
                                               StringT& err_msg) noexcept {
    auto isolate = v8::Isolate::GetCurrent();
    v8::HandleScope handle_scope(isolate);
    v8::String::Utf8Value exception(isolate, try_catch->Exception());

    // Checks isolate is currently terminating because of a call to
    // TerminateExecution.
    if (isolate->IsExecutionTerminating()) {
      err_msg = StringT(kTimeoutErrorMsg);
      return core::FailureExecutionResult(
          core::errors::SC_ROMA_V8_WORKER_SCRIPT_EXECUTION_TIMEOUT);
    }

    err_msg = DescribeError(isolate, try_catch);

    return core::FailureExecutionResult(SC_UNKNOWN);
  }

  /**
   * @brief Converts RomaVector to v8 Array.
   *
   * @param input The object of RomaVector.
   * @param is_wasm Whether this is targeted towards a wasm handler.
   * @return v8::Local<v8::String> The output of v8 Value.
   */
  static v8::Local<v8::Array> InputToLocalArgv(
      const common::RomaVector<common::RomaString>& input,
      bool is_wasm = false) noexcept;

  /**
   * @brief Gets the execution result based on the exception_result and
   * predefined_result. Returns the exception_result if it is defined.
   * Otherwise, return the predefined_result.
   *
   * @param exception_result The exception_result from ReportException().
   * @param defined_code The pre-defined status code.
   * @return core::ExecutionResult
   */
  static core::ExecutionResult GetExecutionResult(
      const core::ExecutionResult& exception_result,
      core::StatusCode defined_code) noexcept;

  /**
   * @brief Read a value from WASM memory
   *
   * @param isolate
   * @param context
   * @param offset
   * @param read_value_type The type of the WASM value being read
   * @return v8::Local<v8::Value>
   */
  static v8::Local<v8::Value> ReadFromWasmMemory(
      v8::Isolate* isolate, v8::Local<v8::Context>& context, int32_t offset,
      WasmDataType read_value_type);

  /**
   * @brief Extract the error message from v8::Message object.
   *
   * @param isolate
   * @param message
   * @return std::string
   */
  static std::string ExtractMessage(v8::Isolate* isolate,
                                    v8::Local<v8::Message> message) noexcept;

  /**
   * @brief Parse the input using JSON::Parse to turn it into the right JS types
   *
   * @param input
   * @return Local<Array> The array of parsed values
   */
  template <typename InputT = common::RomaVector<common::RomaString>>
  static v8::Local<v8::Array> ParseAsJsInput(const InputT& input) {
    auto isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetCurrentContext();

    const int argc = input.size();

    v8::Local<v8::Array> argv = v8::Array::New(isolate, argc);
    for (auto i = 0; i < argc; ++i) {
      auto str = std::string(input[i].c_str(), input[i].size());

      v8::Local<v8::String> arg_str =
          TypeConverter<std::string>::ToV8(isolate, str).As<v8::String>();

      v8::Local<v8::Value> arg = v8::Undefined(isolate);
      if (arg_str->Length() > 0 &&
          !v8::JSON::Parse(context, arg_str).ToLocal(&arg)) {
        return v8::Local<v8::Array>();
      }
      if (!argv->Set(context, i, arg).ToChecked()) {
        return v8::Local<v8::Array>();
      }
    }

    return argv;
  }

  /**
   * @brief Parse the handler input to be provided to a WASM handler.
   * This function handles writing to the WASM memory if necessary.
   *
   * @param isolate
   * @param context
   * @param input
   * @return Local<Array> The input arguments to be provided to the WASM
   * handler.
   */
  template <typename InputT = common::RomaVector<common::RomaString>>
  static v8::Local<v8::Array> ParseAsWasmInput(v8::Isolate* isolate,
                                               v8::Local<v8::Context>& context,
                                               const InputT input) {
    // Parse it into JS types so we can distinguish types
    auto parsed_args = ExecutionUtils::ParseAsJsInput(input);
    const int argc = parsed_args.IsEmpty() ? 0 : parsed_args->Length();

    // Parsing the input failed
    if (argc != input.size()) {
      return v8::Local<v8::Array>();
    }

    v8::Local<v8::Array> argv = v8::Array::New(isolate, argc);

    auto wasm_memory = GetWasmMemoryObject(isolate, context);

    if (wasm_memory->IsUndefined()) {
      // The module has no memory object. This is either a very basic WASM, or
      // invalid, we'll just exit early, and pass the input as it was parsed.
      return parsed_args;
    }

    auto wasm_memory_blob = wasm_memory.As<v8::WasmMemoryObject>()
                                ->Buffer()
                                ->GetBackingStore()
                                ->Data();
    auto wasm_memory_size = wasm_memory.As<v8::WasmMemoryObject>()
                                ->Buffer()
                                ->GetBackingStore()
                                ->ByteLength();

    size_t wasm_memory_offset = 0;

    for (auto i = 0; i < argc; ++i) {
      auto arg = parsed_args->Get(context, i).ToLocalChecked();

      // We only support uint/int, string and array of string args
      if (!arg->IsUint32() && !arg->IsInt32() && !arg->IsString() &&
          !arg->IsArray()) {
        argv.Clear();
        return v8::Local<v8::Array>();
      }

      v8::Local<v8::Value> new_arg;

      if (arg->IsUint32() || arg->IsInt32()) {
        // No serialization needed
        new_arg = arg;
      }
      if (arg->IsString()) {
        std::string str_value;
        TypeConverter<std::string>::FromV8(isolate, arg, &str_value);
        auto string_ptr_in_wasm_memory =
            wasm::WasmSerializer::WriteCustomString(
                wasm_memory_blob, wasm_memory_size, wasm_memory_offset,
                str_value);

        // The serialization failed
        if (string_ptr_in_wasm_memory == UINT32_MAX) {
          return v8::Local<v8::Array>();
        }

        new_arg =
            TypeConverter<uint32_t>::ToV8(isolate, string_ptr_in_wasm_memory);
        wasm_memory_offset +=
            wasm::RomaWasmStringRepresentation::ComputeMemorySizeFor(str_value);
      }
      if (arg->IsArray()) {
        std::vector<std::string> vec_value;
        bool worked = TypeConverter<std::vector<std::string>>::FromV8(
            isolate, arg, &vec_value);

        if (!worked) {
          // This means the array is not an array of string
          return v8::Local<v8::Array>();
        }

        auto list_ptr_in_wasm_memory =
            wasm::WasmSerializer::WriteCustomListOfString(
                wasm_memory_blob, wasm_memory_size, wasm_memory_offset,
                vec_value);

        // The serialization failed
        if (list_ptr_in_wasm_memory == UINT32_MAX) {
          return v8::Local<v8::Array>();
        }

        new_arg =
            TypeConverter<uint32_t>::ToV8(isolate, list_ptr_in_wasm_memory);
        wasm_memory_offset +=
            wasm::RomaWasmListOfStringRepresentation::ComputeMemorySizeFor(
                vec_value);
      }

      if (!argv->Set(context, i, new_arg).ToChecked()) {
        return v8::Local<v8::Array>();
      }
    }

    return argv;
  }

  /**
   * @brief Generate an object that represents the WASM imports modules
   *
   * @param isolate
   * @return Local<Object>
   */
  static v8::Local<v8::Object> GenerateWasmImports(v8::Isolate* isolate);

  static std::string DescribeError(v8::Isolate* isolate,
                                   v8::TryCatch* try_catch) noexcept;

  /**
   * @brief Get the WASM memory object that was registered in the global context
   *
   * @param isolate
   * @param context
   * @return Local<Value> The WASM memory object
   */
  static v8::Local<v8::Value> GetWasmMemoryObject(
      v8::Isolate* isolate, v8::Local<v8::Context>& context);
};
}  // namespace google::scp::roma::worker
