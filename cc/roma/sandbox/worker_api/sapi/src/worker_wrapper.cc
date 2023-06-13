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

#include "worker_wrapper.h"

#include <unistd.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/interface/errors.h"
#include "roma/sandbox/worker_api/sapi/src/worker_init_params.pb.h"
#include "roma/sandbox/worker_api/sapi/src/worker_params.pb.h"
#include "roma/sandbox/worker_factory/src/worker_factory.h"

#include "error_codes.h"

using google::scp::core::StatusCode;
using google::scp::core::errors::SC_ROMA_WORKER_API_UNINITIALIZED_WORKER;
using google::scp::roma::sandbox::worker::Worker;
using google::scp::roma::sandbox::worker::WorkerFactory;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;

shared_ptr<Worker> worker_;

StatusCode Init(worker_api::WorkerInitParamsProto* init_params) {
  if (worker_) {
    Stop();
  }

  auto worker_engine = static_cast<WorkerFactory::WorkerEngine>(
      init_params->worker_factory_js_engine());

  WorkerFactory::FactoryParams factory_params;
  factory_params.engine = worker_engine;
  factory_params.require_preload =
      init_params->require_code_preload_for_execution();

  if (worker_engine == WorkerFactory::WorkerEngine::v8) {
    vector<string> native_js_function_names(
        init_params->native_js_function_names().begin(),
        init_params->native_js_function_names().end());

    WorkerFactory::V8WorkerEngineParams v8_params{
        .native_js_function_comms_fd =
            init_params->native_js_function_comms_fd(),
        .native_js_function_names = native_js_function_names,
    };

    factory_params.v8_worker_engine_params = v8_params;
  }

  auto worker_or = WorkerFactory::Create(factory_params);
  if (!worker_or.result().Successful()) {
    return worker_or.result().status_code;
  }

  worker_ = *worker_or;

  return worker_->Init().status_code;
}

StatusCode Run() {
  if (!worker_) {
    return SC_ROMA_WORKER_API_UNINITIALIZED_WORKER;
  }

  return worker_->Run().status_code;
}

StatusCode Stop() {
  if (!worker_) {
    return SC_ROMA_WORKER_API_UNINITIALIZED_WORKER;
  }
  auto result = worker_->Stop();
  worker_.reset();
  return result.status_code;
}

StatusCode RunCode(worker_api::WorkerParamsProto* params) {
  if (!worker_) {
    return SC_ROMA_WORKER_API_UNINITIALIZED_WORKER;
  }

  auto code = params->code();
  vector<string> input;
  for (int i = 0; i < params->input_size(); i++) {
    input.push_back(params->input().at(i));
  }
  unordered_map<string, string> metadata;
  for (auto&& element : params->metadata()) {
    metadata[element.first] = element.second;
  }

  auto response_or = worker_->RunCode(code, input, metadata);

  if (!response_or.result().Successful()) {
    return response_or.result().status_code;
  }

  params->set_response(*response_or);
  return SC_OK;
}
