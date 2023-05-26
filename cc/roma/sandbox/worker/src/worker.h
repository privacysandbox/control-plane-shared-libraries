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

#include "core/common/lru_cache/src/lru_cache.h"
#include "core/interface/service_interface.h"
#include "public/core/interface/execution_result.h"
#include "roma/sandbox/js_engine/src/js_engine.h"

namespace google::scp::roma::sandbox::worker {
/// @brief This class acts a single-threaded worker which receives work items
/// and executes them inside of a JS/WASM engine.
class Worker : public core::ServiceInterface {
 public:
  explicit Worker(std::shared_ptr<js_engine::JsEngine> js_engine,
                  bool require_preload = true)
      : js_engine_(js_engine),
        require_preload_(require_preload),
        compilation_contexts_(5) {}

  core::ExecutionResult Init() noexcept override;

  core::ExecutionResult Run() noexcept override;

  core::ExecutionResult Stop() noexcept override;

  virtual core::ExecutionResultOr<std::string> RunCode(
      const std::string& code, const std::vector<std::string>& input,
      const std::unordered_map<std::string, std::string>& metadata);

 private:
  std::shared_ptr<js_engine::JsEngine> js_engine_;
  /**
   * @brief Used to keep track of compilation cotexts
   *
   */
  core::common::LruCache<std::string, js_engine::RomaJsEngineCompilationContext>
      compilation_contexts_;
  bool require_preload_;
};
}  // namespace google::scp::roma::sandbox::worker
