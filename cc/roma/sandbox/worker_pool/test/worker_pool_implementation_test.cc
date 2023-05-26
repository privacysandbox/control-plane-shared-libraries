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

#include "roma/sandbox/worker_pool/src/worker_pool_implementation.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "public/core/test/interface/execution_result_matchers.h"
#include "roma/sandbox/worker_api/src/worker_api_sapi.h"

using google::scp::roma::sandbox::worker_api::WorkerApiSapi;

namespace google::scp::roma::sandbox::worker_pool::test {
TEST(WorkerPoolTest, CanInitRunAndStop) {
  auto pool = WorkerPoolImplementation<WorkerApiSapi>();

  auto result = pool.Init();
  EXPECT_SUCCESS(result);

  result = pool.Run();
  EXPECT_SUCCESS(result);

  result = pool.Stop();
  EXPECT_SUCCESS(result);
}

TEST(WorkerPoolTest, CanGetPoolCount) {
  auto pool = WorkerPoolImplementation<WorkerApiSapi>(2);

  auto result = pool.Init();
  EXPECT_SUCCESS(result);

  result = pool.Run();
  EXPECT_SUCCESS(result);

  EXPECT_EQ(pool.GetPoolSize(), 2);

  result = pool.Stop();
  EXPECT_SUCCESS(result);
}

TEST(WorkerPoolTest, CanGetWorker) {
  auto pool = WorkerPoolImplementation<WorkerApiSapi>(2);

  auto result = pool.Init();
  EXPECT_SUCCESS(result);

  result = pool.Run();
  EXPECT_SUCCESS(result);

  auto worker1 = pool.GetWoker(0);
  EXPECT_SUCCESS(worker1.result());
  auto worker2 = pool.GetWoker(1);
  EXPECT_SUCCESS(worker2.result());

  EXPECT_NE(worker1->get(), worker2->get());

  result = pool.Stop();
  EXPECT_SUCCESS(result);
}
}  // namespace google::scp::roma::sandbox::worker_pool::test
