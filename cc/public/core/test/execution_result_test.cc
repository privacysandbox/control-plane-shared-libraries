// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "public/core/interface/execution_result.h"

#include <gtest/gtest.h>

#include "core/common/proto/common.pb.h"

using std::string;

namespace google::scp::core::test {
TEST(ExecutionResultTest, ToProto) {
  auto success = SuccessExecutionResult();
  auto actual_result = success.ToProto();
  EXPECT_EQ(actual_result.status(),
            core::common::proto::ExecutionStatus::EXECUTION_STATUS_SUCCESS);
  EXPECT_EQ(actual_result.status_code(), 0);

  FailureExecutionResult failure(2);
  actual_result = failure.ToProto();
  EXPECT_EQ(actual_result.status(),
            core::common::proto::ExecutionStatus::EXECUTION_STATUS_FAILURE);
  EXPECT_EQ(actual_result.status_code(), 2);

  RetryExecutionResult retry(2);
  actual_result = retry.ToProto();
  EXPECT_EQ(actual_result.status(),
            core::common::proto::ExecutionStatus::EXECUTION_STATUS_RETRY);
  EXPECT_EQ(actual_result.status_code(), 2);
}

TEST(ExecutionResultTest, FromProto) {
  core::common::proto::ExecutionResult success_proto;
  success_proto.set_status(
      core::common::proto::ExecutionStatus::EXECUTION_STATUS_SUCCESS);
  auto actual_result = ExecutionResult(success_proto);
  EXPECT_EQ(actual_result.status, ExecutionStatus::Success);
  EXPECT_EQ(actual_result.status_code, 0);

  core::common::proto::ExecutionResult failure_proto;
  failure_proto.set_status(
      core::common::proto::ExecutionStatus::EXECUTION_STATUS_FAILURE);
  failure_proto.set_status_code(2);
  actual_result = ExecutionResult(failure_proto);
  EXPECT_EQ(actual_result.status, ExecutionStatus::Failure);
  EXPECT_EQ(actual_result.status_code, 2);

  core::common::proto::ExecutionResult retry_proto;
  retry_proto.set_status(
      core::common::proto::ExecutionStatus::EXECUTION_STATUS_RETRY);
  retry_proto.set_status_code(2);
  actual_result = ExecutionResult(retry_proto);
  EXPECT_EQ(actual_result.status, ExecutionStatus::Retry);
  EXPECT_EQ(actual_result.status_code, 2);
}

TEST(ExecutionResultTest, FromUnknownProto) {
  core::common::proto::ExecutionResult unknown_proto;
  unknown_proto.set_status(
      core::common::proto::ExecutionStatus::EXECUTION_STATUS_UNKNOWN);
  auto actual_result = ExecutionResult(unknown_proto);
  EXPECT_EQ(actual_result.status, ExecutionStatus::Failure);
  EXPECT_EQ(actual_result.status_code, 0);
}
}  // namespace google::scp::core::test
