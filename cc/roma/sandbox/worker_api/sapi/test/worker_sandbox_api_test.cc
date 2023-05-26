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

#include "roma/sandbox/worker_api/sapi/src/worker_sandbox_api.h"

#include <gtest/gtest.h>

#include "public/core/test/interface/execution_result_matchers.h"
#include "roma/sandbox/constants/constants.h"
#include "roma/sandbox/worker_factory/src/worker_factory.h"

using google::scp::roma::sandbox::constants::kCodeVersion;
using google::scp::roma::sandbox::constants::kHandlerName;
using google::scp::roma::sandbox::constants::kRequestAction;
using google::scp::roma::sandbox::constants::kRequestActionExecute;
using google::scp::roma::sandbox::constants::kRequestType;
using google::scp::roma::sandbox::constants::kRequestTypeJavascript;
using google::scp::roma::sandbox::worker::WorkerFactory;

namespace google::scp::roma::sandbox::worker_api::test {
TEST(WorkerSandboxApiTest, WorkerWorksThroughSandbox) {
  WorkerSandboxApi sandbox_api(WorkerFactory::WorkerEngine::v8,
                               false /*require_preaload*/);

  auto result = sandbox_api.Init();
  EXPECT_SUCCESS(result);

  result = sandbox_api.Run();
  EXPECT_SUCCESS(result);

  ::worker_api::WorkerParamsProto params_proto;
  params_proto.set_code(
      "function cool_func() { return \"Hi there from sandboxed JS :)\" }");
  (*params_proto.mutable_metadata())[kRequestType] = kRequestTypeJavascript;
  (*params_proto.mutable_metadata())[kHandlerName] = "cool_func";
  (*params_proto.mutable_metadata())[kCodeVersion] = "1";
  (*params_proto.mutable_metadata())[kRequestAction] = kRequestActionExecute;

  result = sandbox_api.RunCode(params_proto);
  EXPECT_SUCCESS(result);
  EXPECT_EQ(params_proto.response(), "\"Hi there from sandboxed JS :)\"");

  result = sandbox_api.Stop();
  EXPECT_SUCCESS(result);
}
}  // namespace google::scp::roma::sandbox::worker_api::test
