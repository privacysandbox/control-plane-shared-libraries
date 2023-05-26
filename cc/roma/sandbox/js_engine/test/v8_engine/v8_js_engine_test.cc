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

#include "roma/sandbox/js_engine/src/v8_engine/v8_js_engine.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "core/test/utils/auto_init_run_stop.h"
#include "public/core/test/interface/execution_result_matchers.h"

using google::scp::core::test::AutoInitRunStop;
using std::string;
using std::vector;

using google::scp::roma::sandbox::js_engine::v8_js_engine::V8JsEngine;

namespace google::scp::roma::sandbox::js_engine::test {
class V8JsEngineTest : public ::testing::Test {
 public:
  static void SetUpTestSuite() {
    V8JsEngine engine;
    engine.OneTimeSetup();
  }
};

TEST_F(V8JsEngineTest, CanRunJsCode) {
  V8JsEngine engine;
  AutoInitRunStop to_handle_engine(engine);

  auto js_code =
      "function hello_js(input1, input2) { return \"Hello World!\" + \" \" + "
      "input1 + \" \" + input2 }";
  vector<string> input = {"\"vec input 1\"", "\"vec input 2\""};

  auto response_or = engine.CompileAndRunJs(js_code, "hello_js", input);

  EXPECT_SUCCESS(response_or.result());
  auto response_string = response_or->response;
  EXPECT_EQ(response_string, "\"Hello World! vec input 1 vec input 2\"");
}

TEST_F(V8JsEngineTest, CanHandleCompilationFailures) {
  V8JsEngine engine;
  AutoInitRunStop to_handle_engine(engine);

  auto js_code = "function hello_js(input1, input2) {";
  vector<string> input = {"\"vec input 1\"", "\"vec input 2\""};

  auto response_or = engine.CompileAndRunJs(js_code, "hello_js", input);

  EXPECT_FALSE(response_or.result().Successful());
}

TEST_F(V8JsEngineTest, ShouldSucceedWithEmptyResponseIfHandlerNameIsEmpty) {
  V8JsEngine engine;
  AutoInitRunStop to_handle_engine(engine);

  auto js_code =
      "function hello_js(input1, input2) { return \"Hello World!\" + \" \" + "
      "input1 + \" \" + input2 }";
  vector<string> input = {"\"vec input 1\"", "\"vec input 2\""};

  // Empty handler
  auto response_or = engine.CompileAndRunJs(js_code, "", input);

  EXPECT_SUCCESS(response_or.result());
  auto response_string = response_or->response;
  EXPECT_EQ(response_string, "");
}

TEST_F(V8JsEngineTest, ShouldFailIfInputCannotBeParsed) {
  V8JsEngine engine;
  AutoInitRunStop to_handle_engine(engine);

  auto js_code =
      "function hello_js(input1, input2) { return \"Hello World!\" + \" \" + "
      "input1 + \" \" + input2 }";
  // Bad input
  vector<string> input = {"vec input 1\"", "\"vec input 2\""};

  auto response_or = engine.CompileAndRunJs(js_code, "hello_js", input);

  EXPECT_FALSE(response_or.result());
}
}  // namespace google::scp::roma::sandbox::js_engine::test
