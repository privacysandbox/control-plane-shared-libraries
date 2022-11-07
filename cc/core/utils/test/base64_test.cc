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

#include "core/utils/src/base64.h"

#include <gtest/gtest.h>

#include <string>

#include "core/utils/src/error_codes.h"

using std::string;

namespace google::scp::core::utils::test {
TEST(Base64Test, Base64EncodeInvalidValue) {
  string empty;
  string encoded;
  EXPECT_EQ(Base64Encode(empty, encoded),
            FailureExecutionResult(errors::SC_CORE_UTILS_INVALID_INPUT));
}

TEST(Base64Test, Base64EncodeValidValue) {
  string decoded("test_test_test");
  string encoded;
  EXPECT_EQ(Base64Encode(decoded, encoded), SuccessExecutionResult());
  EXPECT_EQ(encoded, "dGVzdF90ZXN0X3Rlc3Q=");
}

TEST(Base64Test, Base64DecodeInvalidValue) {
  string encoded("sdasdasdas");
  string decoded;
  EXPECT_EQ(Base64Decode(encoded, decoded),
            FailureExecutionResult(errors::SC_CORE_UTILS_INVALID_INPUT));
}

TEST(Base64Test, Base64DecodeValidValues) {
  string empty;
  string decoded;
  EXPECT_EQ(Base64Decode(empty, decoded), SuccessExecutionResult());

  string encoded("dGVzdF90ZXN0X3Rlc3Q=");
  EXPECT_EQ(Base64Decode(encoded, decoded), SuccessExecutionResult());
  EXPECT_EQ(decoded, "test_test_test");
}

}  // namespace google::scp::core::utils::test
