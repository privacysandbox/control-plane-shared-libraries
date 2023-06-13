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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "include/libplatform/libplatform.h"
#include "include/v8.h"
#include "public/core/interface/execution_result.h"
#include "roma/sandbox/js_engine/src/js_engine.h"

#include "v8_isolate_visitor.h"

namespace google::scp::roma::sandbox::js_engine::v8_js_engine {
/**
 * @brief Implementation of a JS engine using v8
 *
 */
class V8JsEngine : public JsEngine {
 public:
  V8JsEngine(
      const std::vector<std::shared_ptr<V8IsolateVisitor>>& isolate_visitors =
          std::vector<std::shared_ptr<V8IsolateVisitor>>())
      : isolate_visitors_(isolate_visitors) {}

  core::ExecutionResult Init() noexcept override;

  core::ExecutionResult Run() noexcept override;

  core::ExecutionResult Stop() noexcept override;

  core::ExecutionResult OneTimeSetup(
      const std::unordered_map<std::string, std::string>& config =
          std::unordered_map<std::string, std::string>()) noexcept override;

  core::ExecutionResultOr<js_engine::JsExecutionResponse> CompileAndRunJs(
      const std::string& code, const std::string& function_name,
      const std::vector<std::string>& input,
      const js_engine::RomaJsEngineCompilationContext& context =
          RomaJsEngineCompilationContext()) noexcept override;

 protected:
  inline static std::unique_ptr<v8::Platform> v8_platform_;

  v8::Isolate* v8_isolate_ = nullptr;
  const std::vector<std::shared_ptr<V8IsolateVisitor>> isolate_visitors_;
};
}  // namespace google::scp::roma::sandbox::js_engine::v8_js_engine
