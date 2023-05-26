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

#include "worker_factory.h"

#include <memory>

#include "roma/sandbox/js_engine/src/v8_engine/v8_js_engine.h"

using google::scp::core::ExecutionResultOr;
using google::scp::core::FailureExecutionResult;
using google::scp::roma::sandbox::js_engine::v8_js_engine::V8JsEngine;
using std::make_shared;
using std::shared_ptr;

namespace google::scp::roma::sandbox::worker {

ExecutionResultOr<shared_ptr<Worker>> WorkerFactory::Create(
    const WorkerFactory::FactoryParams& params) {
  if (params.engine == WorkerFactory::WorkerEngine::v8) {
    auto v8_engine = make_shared<V8JsEngine>();
    v8_engine->OneTimeSetup();
    auto worker = make_shared<Worker>(v8_engine, params.require_preload);
    return worker;
  } else {
    // TODO: Add named error
    throw FailureExecutionResult(SC_UNKNOWN);
  }
}
}  // namespace google::scp::roma::sandbox::worker
