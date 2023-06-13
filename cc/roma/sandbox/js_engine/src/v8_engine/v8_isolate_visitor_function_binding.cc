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

#include "v8_isolate_visitor_function_binding.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "cc/roma/interface/function_binding_io.pb.h"
#include "roma/common/src/containers.h"
#include "roma/config/src/type_converter.h"

#include "error_codes.h"

using google::scp::core::ExecutionResult;
using google::scp::core::FailureExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::errors::
    SC_ROMA_V8_ENGINE_COULD_NOT_REGISTER_FUNCTION_BINDING;
using google::scp::core::errors::
    SC_ROMA_V8_ISOLATE_VISITOR_FUNCTION_BINDING_EMPTY_CONTEXT;
using google::scp::core::errors::
    SC_ROMA_V8_ISOLATE_VISITOR_FUNCTION_BINDING_INVALID_ISOLATE;
using google::scp::roma::proto::FunctionBindingIoProto;
using std::string;
using std::to_string;
using std::unordered_map;
using std::vector;
using v8::Array;
using v8::Context;
using v8::External;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Map;
using v8::Object;
using v8::String;
using v8::Undefined;
using v8::Value;

static constexpr char kCouldNotRunFunctionBinding[] =
    "ROMA: Could not run C++ function binding.";
static constexpr char kUnexpectedDataInBindingCallback[] =
    "ROMA: Unexpected data in global callback.";
static constexpr char kThisInstanceLookupKey[] =
    "V8IsolateVisitorFunctionBinding";
static constexpr char kFunctionLookupKey[] = "RomaBindingName";
static constexpr char kCouldNotConvertJsFunctionInputToNative[] =
    "ROMA: Could not convert JS function input to native C++ type.";
static constexpr char KCouldNotConvertNativeFunctionReturnToV8Type[] =
    "ROMA: Could not convert native function return to JS type.";
static constexpr char kErrorInFunctionBindingInvocation[] =
    "ROMA: Error while executing native function binding.";

namespace google::scp::roma::sandbox::js_engine::v8_js_engine {
static bool V8TypesToProto(const FunctionCallbackInfo<Value>& info,
                           FunctionBindingIoProto& proto) {
  if (info.Length() == 0) {
    // No arguments were passed to function
    return true;
  }
  if (info.Length() > 1) {
    return false;
  }

  auto isolate = info.GetIsolate();
  auto function_parameter = info[0];

  // Try to convert to one of the supported types
  string string_native;
  vector<string> vector_of_string_native;
  unordered_map<string, string> map_of_string_native;

  if (TypeConverter<string>::FromV8(isolate, function_parameter,
                                    &string_native)) {
    proto.set_input_string(string_native);
  } else if (TypeConverter<vector<string>>::FromV8(isolate, function_parameter,
                                                   &vector_of_string_native)) {
    proto.mutable_input_list_of_string()->mutable_data()->Add(
        vector_of_string_native.begin(), vector_of_string_native.end());
  } else if (TypeConverter<unordered_map<string, string>>::FromV8(
                 isolate, function_parameter, &map_of_string_native)) {
    for (auto&& kvp : map_of_string_native) {
      (*proto.mutable_input_map_of_string()->mutable_data())[kvp.first] =
          kvp.second;
    }
  } else {
    // Unknown type
    return false;
  }

  return true;
}

static Local<Value> ProtoToV8Type(Isolate* isolate,
                                  const FunctionBindingIoProto& proto) {
  if (proto.has_output_string()) {
    return TypeConverter<string>::ToV8(isolate, proto.output_string());
  } else if (proto.has_output_list_of_string()) {
    auto len = proto.output_list_of_string().data().size();
    vector<string> output_list;
    for (int i = 0; i < len; i++) {
      output_list.push_back(proto.output_list_of_string().data().at(i));
    }
    return TypeConverter<vector<string>>::ToV8(isolate, output_list);
  } else if (proto.has_output_map_of_string()) {
    unordered_map<string, string> output_map;
    for (auto& [key, value] : proto.output_map_of_string().data()) {
      output_map[key] = value;
    }
    return TypeConverter<unordered_map<string, string>>::ToV8(isolate,
                                                              output_map);
  }

  return Undefined(isolate);
}

void V8IsolateVisitorFunctionBinding::GlobalV8FunctionCallback(
    const FunctionCallbackInfo<Value>& info) {
  auto isolate = info.GetIsolate();
  auto context = isolate->GetCurrentContext();
  Isolate::Scope isolate_scope(isolate);
  HandleScope handle_scope(isolate);

  auto data = info.Data();

  if (data.IsEmpty()) {
    isolate->ThrowError(kUnexpectedDataInBindingCallback);
    return;
  }
  // The data we get here is an object containing the name of the function that
  // was called and a pointer to this class instance.
  auto data_object = Local<Object>::Cast(data);

  // Convert the data into a usable V8IsolateVisitorFunctionBinding
  auto this_object_maybe = data_object->Get(
      context, TypeConverter<string>::ToV8(isolate, kThisInstanceLookupKey));
  Local<Value> this_object_local;
  if (!this_object_maybe.ToLocal(&this_object_local)) {
    isolate->ThrowError(kUnexpectedDataInBindingCallback);
    return;
  }
  if (!this_object_local->IsExternal()) {
    isolate->ThrowError(kUnexpectedDataInBindingCallback);
    return;
  }
  Local<External> this_object_external =
      Local<External>::Cast(this_object_local);
  auto this_object = reinterpret_cast<V8IsolateVisitorFunctionBinding*>(
      this_object_external->Value());

  // Get the function name
  auto function_name_maybe = data_object->Get(
      context, TypeConverter<string>::ToV8(isolate, kFunctionLookupKey));
  Local<Value> function_name_local;
  if (!function_name_maybe.ToLocal(&function_name_local)) {
    isolate->ThrowError(kUnexpectedDataInBindingCallback);
    return;
  }

  string native_function_name;
  if (!TypeConverter<string>::FromV8(isolate, function_name_local.As<String>(),
                                     &native_function_name)) {
    isolate->ThrowError(kUnexpectedDataInBindingCallback);
    return;
  }

  FunctionBindingIoProto function_invocation_proto;
  if (!V8TypesToProto(info, function_invocation_proto)) {
    isolate->ThrowError(kCouldNotConvertJsFunctionInputToNative);
    return;
  }

  auto result = this_object->function_invoker_->Invoke(
      native_function_name, function_invocation_proto);
  if (!result.Successful()) {
    isolate->ThrowError(kCouldNotRunFunctionBinding);
    return;
  }
  if (function_invocation_proto.errors().size() > 0) {
    isolate->ThrowError(kErrorInFunctionBindingInvocation);
    return;
  }

  auto returned_value = ProtoToV8Type(isolate, function_invocation_proto);
  if (returned_value->IsUndefined()) {
    isolate->ThrowError(KCouldNotConvertNativeFunctionReturnToV8Type);
    return;
  }

  info.GetReturnValue().Set(returned_value);
}

ExecutionResult V8IsolateVisitorFunctionBinding::Visit(
    Isolate* isolate) noexcept {
  if (!isolate) {
    return FailureExecutionResult(
        SC_ROMA_V8_ISOLATE_VISITOR_FUNCTION_BINDING_INVALID_ISOLATE);
  }

  auto context = isolate->GetCurrentContext();

  if (context.IsEmpty()) {
    return FailureExecutionResult(
        SC_ROMA_V8_ISOLATE_VISITOR_FUNCTION_BINDING_EMPTY_CONTEXT);
  }

  auto global_object = context->Global();

  for (auto& function_name : function_names_) {
    // This is used to be able to retrieve data from the callback
    auto function_template_data_object = Object::New(isolate);
    // Store a pointer to this V8IsolateVisitorFunctionBinding instance
    Local<External> this_class_instance =
        External::New(isolate, reinterpret_cast<void*>(this));
    auto set_this_instance = function_template_data_object->Set(
        context, TypeConverter<string>::ToV8(isolate, kThisInstanceLookupKey),
        this_class_instance);
    if (!set_this_instance.FromMaybe(false /*default_if_empty*/)) {
      return FailureExecutionResult(
          SC_ROMA_V8_ENGINE_COULD_NOT_REGISTER_FUNCTION_BINDING);
    }

    // Store the name of the function as it is to be called from
    // javascript
    auto set_function_name = function_template_data_object->Set(
        context, TypeConverter<string>::ToV8(isolate, kFunctionLookupKey),
        TypeConverter<string>::ToV8(isolate, function_name));
    if (!set_function_name.FromMaybe(false /*default_if_empty*/)) {
      return FailureExecutionResult(
          SC_ROMA_V8_ENGINE_COULD_NOT_REGISTER_FUNCTION_BINDING);
    }
    // Create the function template to register in the global object
    auto function_template = FunctionTemplate::New(
        isolate, &GlobalV8FunctionCallback, function_template_data_object);
    Local<Value> function_instance;
    if (!function_template->GetFunction(context).ToLocal(&function_instance)) {
      return FailureExecutionResult(
          SC_ROMA_V8_ENGINE_COULD_NOT_REGISTER_FUNCTION_BINDING);
    }
    // Convert the function binding name to a v8 type
    auto binding_name =
        TypeConverter<string>::ToV8(isolate, function_name).As<String>();
    // Register the function in the global object
    auto set_function_template =
        global_object->Set(context, binding_name, function_instance);
    if (!set_function_template.FromMaybe(false /*default_if_empty*/)) {
      return FailureExecutionResult(
          SC_ROMA_V8_ENGINE_COULD_NOT_REGISTER_FUNCTION_BINDING);
    }
  }

  return SuccessExecutionResult();
}
}  // namespace google::scp::roma::sandbox::js_engine::v8_js_engine
