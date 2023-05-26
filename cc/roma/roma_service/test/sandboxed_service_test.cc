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

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "core/test/utils/conditional_wait.h"
#include "roma/interface/roma.h"

using absl::StatusOr;
using google::scp::core::test::WaitUntil;
using std::atomic;
using std::bind;
using std::get;
using std::make_shared;
using std::make_unique;
using std::move;
using std::string;
using std::to_string;
using std::tuple;
using std::unique_ptr;
using std::vector;

using namespace std::placeholders;     // NOLINT
using namespace std::chrono_literals;  // NOLINT

namespace google::scp::roma::test {
TEST(SandboxedServiceTest, InitStop) {
  auto status = RomaInit();
  EXPECT_TRUE(status.ok());
  status = RomaStop();
  EXPECT_TRUE(status.ok());
}

TEST(SandboxedServiceTest, ExecuteCode) {
  Config config;
  config.NumberOfWorkers = 2;
  auto status = RomaInit(config);
  EXPECT_TRUE(status.ok());

  string result;
  atomic<bool> load_finished = false;
  atomic<bool> execute_finished = false;

  {
    auto code_obj = make_unique<CodeObject>();
    code_obj->id = "foo";
    code_obj->version_num = 1;
    code_obj->js = R"JS_CODE(
    function Handler(input) { return "Hello world! " + JSON.stringify(input);
    }
  )JS_CODE";

    status = LoadCodeObj(move(code_obj),
                         [&](unique_ptr<absl::StatusOr<ResponseObject>> resp) {
                           EXPECT_TRUE(resp->ok());
                           load_finished.store(true);
                         });
    EXPECT_TRUE(status.ok());
  }

  {
    auto execution_obj = make_unique<InvocationRequestStrInput>();
    execution_obj->id = "foo";
    execution_obj->version_num = 1;
    execution_obj->handler_name = "Handler";
    execution_obj->input.push_back("\"Foobar\"");

    status = Execute(move(execution_obj),
                     [&](unique_ptr<absl::StatusOr<ResponseObject>> resp) {
                       EXPECT_TRUE(resp->ok());
                       if (resp->ok()) {
                         auto& code_resp = **resp;
                         result = code_resp.resp;
                       }
                       execute_finished.store(true);
                     });
    EXPECT_TRUE(status.ok());
  }
  WaitUntil([&]() { return load_finished.load(); }, 10s);
  WaitUntil([&]() { return execute_finished.load(); }, 10s);
  EXPECT_EQ(result, R"("Hello world! \"Foobar\"")");

  status = RomaStop();
  EXPECT_TRUE(status.ok());
}

TEST(SandboxedServiceTest, BatchExecute) {
  Config config;
  config.NumberOfWorkers = 2;
  auto status = RomaInit(config);
  EXPECT_TRUE(status.ok());

  atomic<int> res_count(0);
  size_t batch_size(5);
  atomic<bool> load_finished = false;
  atomic<bool> execute_finished = false;
  {
    auto code_obj = make_unique<CodeObject>();
    code_obj->id = "foo";
    code_obj->version_num = 1;
    code_obj->js = R"JS_CODE(
    function Handler(input) { return "Hello world! " + JSON.stringify(input);
    }
  )JS_CODE";

    status = LoadCodeObj(move(code_obj),
                         [&](unique_ptr<absl::StatusOr<ResponseObject>> resp) {
                           EXPECT_TRUE(resp->ok());
                           load_finished.store(true);
                         });
    EXPECT_TRUE(status.ok());
  }

  {
    auto execution_obj = InvocationRequestStrInput();
    execution_obj.id = "foo";
    execution_obj.version_num = 1;
    execution_obj.handler_name = "Handler";
    execution_obj.input.push_back("\"Foobar\"");

    vector<InvocationRequestStrInput> batch(batch_size, execution_obj);
    status = BatchExecute(
        batch,
        [&](const std::vector<absl::StatusOr<ResponseObject>>& batch_resp) {
          for (auto resp : batch_resp) {
            EXPECT_TRUE(resp.ok());
            EXPECT_EQ(resp->resp, R"("Hello world! \"Foobar\"")");
          }
          res_count.store(batch_resp.size());
          execute_finished.store(true);
        });
    EXPECT_TRUE(status.ok());
  }

  WaitUntil([&]() { return load_finished.load(); });
  WaitUntil([&]() { return execute_finished.load(); });
  EXPECT_EQ(res_count.load(), batch_size);

  status = RomaStop();
  EXPECT_TRUE(status.ok());
}

TEST(SandboxedServiceTest, ExecuteCodeConcurrently) {
  Config config;
  config.NumberOfWorkers = 2;
  auto status = RomaInit(config);
  EXPECT_TRUE(status.ok());

  atomic<bool> load_finished = false;
  size_t total_runs = 10;
  vector<string> results(total_runs);
  vector<atomic<bool>> finished(total_runs);
  {
    auto code_obj = make_unique<CodeObject>();
    code_obj->id = "foo";
    code_obj->version_num = 1;
    code_obj->js = R"JS_CODE(
    function Handler(input) { return "Hello world! " + JSON.stringify(input);
    }
  )JS_CODE";

    status = LoadCodeObj(move(code_obj),
                         [&](unique_ptr<absl::StatusOr<ResponseObject>> resp) {
                           EXPECT_TRUE(resp->ok());
                           load_finished.store(true);
                         });
    EXPECT_TRUE(status.ok());
  }

  {
    for (auto i = 0u; i < total_runs; ++i) {
      auto code_obj = make_unique<InvocationRequestSharedInput>();
      code_obj->id = "foo";
      code_obj->version_num = 1;
      code_obj->handler_name = "Handler";
      code_obj->input.push_back(
          make_shared<string>("\"Foobar" + to_string(i) + "\""));

      status = Execute(move(code_obj),
                       [&, i](unique_ptr<absl::StatusOr<ResponseObject>> resp) {
                         EXPECT_TRUE(resp->ok());
                         if (resp->ok()) {
                           auto& code_resp = **resp;
                           results[i] = code_resp.resp;
                         }
                         finished[i].store(true);
                       });
      EXPECT_TRUE(status.ok());
    }
  }

  WaitUntil([&]() { return load_finished.load(); });

  for (auto i = 0u; i < total_runs; ++i) {
    WaitUntil([&, i]() { return finished[i].load(); }, 30s);
    string expected_result = string("\"Hello world! ") + string("\\\"Foobar") +
                             to_string(i) + string("\\\"\"");
    EXPECT_EQ(results[i], expected_result);
  }

  status = RomaStop();
  EXPECT_TRUE(status.ok());
}
}  // namespace google::scp::roma::test
