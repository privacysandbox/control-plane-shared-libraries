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

#include "roma/sandbox/dispatcher/src/dispatcher.h"

#include <gtest/gtest.h>

#include <atomic>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include <gmock/gmock.h>

#include "absl/status/statusor.h"
#include "core/async_executor/src/async_executor.h"
#include "core/test/utils/auto_init_run_stop.h"
#include "core/test/utils/conditional_wait.h"
#include "public/core/test/interface/execution_result_matchers.h"
#include "roma/interface/roma.h"
#include "roma/sandbox/worker_api/src/worker_api.h"
#include "roma/sandbox/worker_api/src/worker_api_sapi.h"
#include "roma/sandbox/worker_pool/src/worker_pool.h"
#include "roma/sandbox/worker_pool/src/worker_pool_api_sapi.h"

using absl::StatusOr;
using google::scp::core::AsyncExecutor;
using google::scp::core::ExecutionResultOr;
using google::scp::core::FailureExecutionResult;
using google::scp::core::test::AutoInitRunStop;
using google::scp::core::test::WaitUntil;
using google::scp::roma::sandbox::worker_api::WorkerApi;
using google::scp::roma::sandbox::worker_api::WorkerApiSapi;
using google::scp::roma::sandbox::worker_api::WorkerApiSapiConfig;
using google::scp::roma::sandbox::worker_pool::WorkerPool;
using google::scp::roma::sandbox::worker_pool::WorkerPoolApiSapi;
using std::atomic;
using std::make_shared;
using std::make_unique;
using std::shared_ptr;
using std::string;
using std::to_string;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;

namespace google::scp::roma::sandbox::dispatcher::test {
TEST(DispatcherTest, CanRunCode) {
  auto async_executor = make_shared<AsyncExecutor>(1, 10);

  vector<WorkerApiSapiConfig> configs;
  WorkerApiSapiConfig config;
  config.worker_js_engine = worker::WorkerFactory::WorkerEngine::v8;
  config.js_engine_require_code_preload = true;
  config.native_js_function_comms_fd = -1;
  config.native_js_function_names = vector<string>();
  configs.push_back(config);

  shared_ptr<WorkerPool> worker_pool =
      make_shared<WorkerPoolApiSapi>(configs, 1);
  AutoInitRunStop for_async_executor(*async_executor);
  AutoInitRunStop for_worker_pool(*worker_pool);

  Dispatcher dispatcher(async_executor, worker_pool, 10);
  AutoInitRunStop for_dispatcher(dispatcher);

  auto load_request = make_unique<CodeObject>();
  load_request->id = "some_id";
  load_request->version_num = 1;
  load_request->js =
      "function test(input) { return input + \" Some string\"; }";

  atomic<bool> done_loading(false);

  auto result = dispatcher.Dispatch(
      move(load_request),
      [&done_loading](unique_ptr<StatusOr<ResponseObject>> resp) {
        EXPECT_TRUE(resp->ok());
        done_loading.store(true);
      });
  EXPECT_SUCCESS(result);

  WaitUntil([&done_loading]() { return done_loading.load(); });

  auto execute_request = make_unique<InvocationRequestStrInput>();
  execute_request->id = "some_id";
  execute_request->version_num = 1;
  execute_request->handler_name = "test";
  execute_request->input.push_back("\"Hello\"");

  atomic<bool> done_executing(false);

  result = dispatcher.Dispatch(
      move(execute_request),
      [&done_executing](unique_ptr<StatusOr<ResponseObject>> resp) {
        EXPECT_TRUE(resp->ok());
        EXPECT_EQ("\"Hello Some string\"", (*resp)->resp);
        done_executing.store(true);
      });

  EXPECT_SUCCESS(result);

  WaitUntil([&done_executing]() { return done_executing.load(); });
}

TEST(DispatcherTest, CanHandleCodeFailures) {
  auto async_executor = make_shared<AsyncExecutor>(1, 10);

  vector<WorkerApiSapiConfig> configs;
  WorkerApiSapiConfig config;
  config.worker_js_engine = worker::WorkerFactory::WorkerEngine::v8;
  config.js_engine_require_code_preload = true;
  config.native_js_function_comms_fd = -1;
  config.native_js_function_names = vector<string>();
  configs.push_back(config);

  shared_ptr<WorkerPool> worker_pool =
      make_shared<WorkerPoolApiSapi>(configs, 1);
  AutoInitRunStop for_async_executor(*async_executor);
  AutoInitRunStop for_worker_pool(*worker_pool);

  Dispatcher dispatcher(async_executor, worker_pool, 10);
  AutoInitRunStop for_dispatcher(dispatcher);

  auto load_request = make_unique<CodeObject>();
  load_request->id = "some_id";
  load_request->version_num = 1;
  // Bad JS
  load_request->js = "function test(input) { ";

  atomic<bool> done_loading(false);

  auto result = dispatcher.Dispatch(
      move(load_request),
      [&done_loading](unique_ptr<StatusOr<ResponseObject>> resp) {
        // That didn't work
        EXPECT_FALSE(resp->ok());
        done_loading.store(true);
      });
  EXPECT_SUCCESS(result);

  WaitUntil([&done_loading]() { return done_loading.load(); });
}

TEST(DispatcherTest, CanHandleExecuteWithoutLoadFailure) {
  auto async_executor = make_shared<AsyncExecutor>(1, 10);

  vector<WorkerApiSapiConfig> configs;
  WorkerApiSapiConfig config;
  config.worker_js_engine = worker::WorkerFactory::WorkerEngine::v8;
  config.js_engine_require_code_preload = true;
  config.native_js_function_comms_fd = -1;
  config.native_js_function_names = vector<string>();
  configs.push_back(config);

  shared_ptr<WorkerPool> worker_pool =
      make_shared<WorkerPoolApiSapi>(configs, 1);
  AutoInitRunStop for_async_executor(*async_executor);
  AutoInitRunStop for_worker_pool(*worker_pool);

  Dispatcher dispatcher(async_executor, worker_pool, 10);
  AutoInitRunStop for_dispatcher(dispatcher);

  auto execute_request = make_unique<InvocationRequestStrInput>();
  execute_request->id = "some_id";
  execute_request->version_num = 1;
  execute_request->handler_name = "test";
  execute_request->input.push_back("\"Hello\"");

  atomic<bool> done_executing(false);

  auto result = dispatcher.Dispatch(
      move(execute_request),
      [&done_executing](unique_ptr<StatusOr<ResponseObject>> resp) {
        EXPECT_FALSE(resp->ok());
        done_executing.store(true);
      });

  EXPECT_SUCCESS(result);

  WaitUntil([&done_executing]() { return done_executing.load(); });
}

TEST(DispatcherTest, BroadcastShouldUpdateAllWorkers) {
  const size_t number_of_workers = 5;
  auto async_executor = make_shared<AsyncExecutor>(number_of_workers, 100);

  vector<WorkerApiSapiConfig> configs;
  for (int i = 0; i < number_of_workers; i++) {
    WorkerApiSapiConfig config;
    config.worker_js_engine = worker::WorkerFactory::WorkerEngine::v8;
    config.js_engine_require_code_preload = true;
    config.native_js_function_comms_fd = -1;
    config.native_js_function_names = vector<string>();
    configs.push_back(config);
  }

  shared_ptr<WorkerPool> worker_pool =
      make_shared<WorkerPoolApiSapi>(configs, number_of_workers);
  AutoInitRunStop for_async_executor(*async_executor);
  AutoInitRunStop for_worker_pool(*worker_pool);

  Dispatcher dispatcher(async_executor, worker_pool, 100);
  AutoInitRunStop for_dispatcher(dispatcher);

  auto load_request = make_unique<CodeObject>();
  load_request->id = "some_id";
  load_request->version_num = 1;
  load_request->js =
      "function test(input) { return input + \" Some string\"; }";

  atomic<bool> done_loading(false);

  auto result = dispatcher.Broadcast(
      move(load_request),
      [&done_loading](unique_ptr<StatusOr<ResponseObject>> resp) {
        EXPECT_TRUE(resp->ok());
        done_loading.store(true);
      });
  EXPECT_SUCCESS(result);

  WaitUntil([&done_loading]() { return done_loading.load(); });

  atomic<int> execution_count(0);
  // More than the number of workers to make sure the requests can indeed run in
  // all workers.
  int requests_sent = number_of_workers * 3;

  for (int i = 0; i < requests_sent; i++) {
    auto execute_request = make_unique<InvocationRequestStrInput>();
    execute_request->id = "some_id" + to_string(i);
    execute_request->version_num = 1;
    execute_request->handler_name = "test";
    execute_request->input.push_back("\"Hello" + to_string(i) + "\"");

    result = dispatcher.Dispatch(
        move(execute_request),
        [&execution_count, i](unique_ptr<StatusOr<ResponseObject>> resp) {
          EXPECT_TRUE(resp->ok());
          EXPECT_EQ("\"Hello" + to_string(i) + " Some string\"", (*resp)->resp);
          execution_count++;
        });

    EXPECT_SUCCESS(result);
  }

  WaitUntil([&execution_count, requests_sent]() {
    return execution_count.load() >= requests_sent;
  });
}

TEST(DispatcherTest, BroadcastShouldExitGracefullyIfThereAreErrorsWithTheCode) {
  const size_t number_of_workers = 5;
  auto async_executor = make_shared<AsyncExecutor>(number_of_workers, 100);

  vector<WorkerApiSapiConfig> configs;
  for (int i = 0; i < number_of_workers; i++) {
    WorkerApiSapiConfig config;
    config.worker_js_engine = worker::WorkerFactory::WorkerEngine::v8;
    config.js_engine_require_code_preload = true;
    config.native_js_function_comms_fd = -1;
    config.native_js_function_names = vector<string>();
    configs.push_back(config);
  }

  shared_ptr<WorkerPool> worker_pool =
      make_shared<WorkerPoolApiSapi>(configs, number_of_workers);
  AutoInitRunStop for_async_executor(*async_executor);
  AutoInitRunStop for_worker_pool(*worker_pool);

  Dispatcher dispatcher(async_executor, worker_pool, 100);
  AutoInitRunStop for_dispatcher(dispatcher);

  auto load_request = make_unique<CodeObject>();
  load_request->id = "some_id";
  load_request->version_num = 1;
  // Bad syntax
  load_request->js = "function test(input) { return";

  atomic<bool> done_loading(false);

  auto result = dispatcher.Broadcast(
      move(load_request),
      [&done_loading](unique_ptr<StatusOr<ResponseObject>> resp) {
        // That failed
        EXPECT_FALSE(resp->ok());
        done_loading.store(true);
      });
  EXPECT_SUCCESS(result);

  WaitUntil([&done_loading]() { return done_loading.load(); });
}

TEST(DispatcherTest, DispatchBatchShouldExecuteAllRequests) {
  const size_t number_of_workers = 5;
  auto async_executor = make_shared<AsyncExecutor>(number_of_workers, 100);

  vector<WorkerApiSapiConfig> configs;
  for (int i = 0; i < number_of_workers; i++) {
    WorkerApiSapiConfig config;
    config.worker_js_engine = worker::WorkerFactory::WorkerEngine::v8;
    config.js_engine_require_code_preload = true;
    config.native_js_function_comms_fd = -1;
    config.native_js_function_names = vector<string>();
    configs.push_back(config);
  }

  shared_ptr<WorkerPool> worker_pool =
      make_shared<WorkerPoolApiSapi>(configs, number_of_workers);
  AutoInitRunStop for_async_executor(*async_executor);
  AutoInitRunStop for_worker_pool(*worker_pool);

  Dispatcher dispatcher(async_executor, worker_pool, 100);
  AutoInitRunStop for_dispatcher(dispatcher);

  auto load_request = make_unique<CodeObject>();
  load_request->id = "some_id";
  load_request->version_num = 1;
  load_request->js =
      "function test(input) { return input + \" Some string\"; }";

  atomic<bool> done_loading(false);

  auto result = dispatcher.Broadcast(
      move(load_request),
      [&done_loading](unique_ptr<StatusOr<ResponseObject>> resp) {
        EXPECT_TRUE(resp->ok());
        done_loading.store(true);
      });
  EXPECT_SUCCESS(result);

  WaitUntil([&done_loading]() { return done_loading.load(); });

  // More than the number of workers to make sure the requests can indeed run in
  // all workers.
  int requests_sent = number_of_workers * 3;

  vector<InvocationRequestStrInput> batch;
  unordered_set<string> request_ids;

  for (int i = 0; i < requests_sent; i++) {
    auto execute_request = InvocationRequestStrInput();
    execute_request.id = "some_id" + to_string(i);
    execute_request.version_num = 1;
    execute_request.handler_name = "test";
    execute_request.input.push_back("\"Hello" + to_string(i) + "\"");

    // Keep track of the request ids
    request_ids.insert(execute_request.id);
    batch.push_back(execute_request);
  }

  atomic<bool> finished_batch(false);
  vector<StatusOr<ResponseObject>> test_batch_response;

  dispatcher.DispatchBatch(
      batch, [&finished_batch, &test_batch_response](
                 const vector<StatusOr<ResponseObject>>& batch_response) {
        for (auto& r : batch_response) {
          test_batch_response.push_back(r);
        }
        finished_batch.store(true);
      });

  WaitUntil([&finished_batch]() { return finished_batch.load(); });

  for (auto& r : test_batch_response) {
    EXPECT_TRUE(r.ok());
    // Remove the ids we see form the set
    request_ids.erase(r->id);
  }

  // Since we should have a gotten a response for all request ID, we expect all
  // the ids to have been removed from this set.
  EXPECT_TRUE(request_ids.empty());
}
}  // namespace google::scp::roma::sandbox::dispatcher::test
