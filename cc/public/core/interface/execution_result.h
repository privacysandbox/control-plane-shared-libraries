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

#ifndef SCP_CORE_INTERFACE_EXECUTION_RESULT_H_
#define SCP_CORE_INTERFACE_EXECUTION_RESULT_H_

#include "core/common/proto/common.pb.h"

namespace google::scp::core {

#define RETURN_IF_FAILURE(__execution_result)                            \
  if (ExecutionResult __res = __execution_result; !__res.Successful()) { \
    return __res;                                                        \
  }

/// Operation's execution status.
enum class ExecutionStatus {
  /// Executed successfully.
  Success = 0,
  /// Execution failed.
  Failure = 1,
  /// did not execution and requires retry.
  Retry = 2,
};

/// Convert ExecutionStatus to Proto.
core::common::proto::ExecutionStatus ToStatusProto(ExecutionStatus& status);

/// Status code returned from operation execution.
typedef uint64_t StatusCode;
#define SC_OK 0UL
#define SC_UNKNOWN 1UL

struct ExecutionResult;
constexpr ExecutionResult SuccessExecutionResult();

/// Operation's execution result including status and status code.
struct ExecutionResult {
  /**
   * @brief Construct a new Execution Result object
   *
   * @param status status of the execution.
   * @param status_code code of the execution status.
   */
  constexpr ExecutionResult(ExecutionStatus status, StatusCode status_code)
      : status(status), status_code(status_code) {}

  constexpr ExecutionResult()
      : ExecutionResult(ExecutionStatus::Failure, SC_UNKNOWN) {}

  explicit ExecutionResult(
      const core::common::proto::ExecutionResult result_proto);

  bool operator==(const ExecutionResult& other) const {
    return status == other.status && status_code == other.status_code;
  }

  bool operator!=(const ExecutionResult& other) const {
    return !operator==(other);
  }

  core::common::proto::ExecutionResult ToProto();

  bool Successful() const { return *this == SuccessExecutionResult(); }

  explicit operator bool() const { return Successful(); }

  /// Status of the executed operation.
  ExecutionStatus status = ExecutionStatus::Failure;
  /**
   * @brief if the operation was not successful, status_code will indicate the
   * error code.
   */
  StatusCode status_code = SC_UNKNOWN;
};

/// ExecutionResult with success status
constexpr ExecutionResult SuccessExecutionResult() {
  return ExecutionResult(ExecutionStatus::Success, SC_OK);
}

/// ExecutionResult with failure status
class FailureExecutionResult : public ExecutionResult {
 public:
  /// Construct a new Failure Execution Result object
  explicit constexpr FailureExecutionResult(StatusCode status_code)
      : ExecutionResult(ExecutionStatus::Failure, status_code) {}
};

/// ExecutionResult with retry status
class RetryExecutionResult : public ExecutionResult {
 public:
  /// Construct a new Retry Execution Result object
  explicit RetryExecutionResult(StatusCode status_code)
      : ExecutionResult(ExecutionStatus::Retry, status_code) {}
};
}  // namespace google::scp::core

#endif  // SCP_CORE_INTERFACE_EXECUTION_RESULT_H_
