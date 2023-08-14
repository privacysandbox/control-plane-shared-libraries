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

#include "cc/cpio/client_providers/job_client_provider/src/job_client_utils.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "cc/cpio/client_providers/job_client_provider/src/error_codes.h"
#include "core/common/serialization/src/serialization.h"
#include "core/interface/async_context.h"
#include "core/utils/src/base64.h"
#include "cpio/client_providers/job_client_provider/test/hello_world.pb.h"
#include "google/protobuf/util/time_util.h"
#include "public/core/interface/execution_result.h"
#include "public/core/test/interface/execution_result_matchers.h"
#include "public/cpio/proto/job_service/v1/job_service.pb.h"

using google::cmrt::sdk::job_service::v1::Job;
using google::cmrt::sdk::job_service::v1::JobStatus;
using google::cmrt::sdk::nosql_database_service::v1::Item;
using google::cmrt::sdk::nosql_database_service::v1::ItemAttribute;
using google::cmrt::sdk::nosql_database_service::v1::UpsertDatabaseItemRequest;
using google::protobuf::Any;
using google::protobuf::Duration;
using google::protobuf::util::TimeUtil;
using google::scp::core::ExecutionResult;
using google::scp::core::FailureExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::errors::SC_JOB_CLIENT_PROVIDER_INVALID_JOB_ITEM;
using google::scp::core::errors::SC_JOB_CLIENT_PROVIDER_INVALID_JOB_STATUS;
using google::scp::core::errors::SC_JOB_CLIENT_PROVIDER_SERIALIZATION_FAILED;
using google::scp::core::test::IsSuccessful;
using google::scp::core::test::ResultIs;
using google::scp::core::utils::Base64Decode;
using google::scp::core::utils::Base64Encode;
using helloworld::HelloWorld;
using std::get;
using std::make_tuple;
using std::string;
using std::tuple;

namespace {
constexpr char kHelloWorldName[] = "hello";
constexpr int kHelloWorldId = 55678413;

constexpr char kJobId[] = "job-id";

constexpr char kJobsTableName[] = "jobs";
constexpr char kJobsTablePartitionKeyName[] = "job_id";
constexpr char kJobBodyColumnName[] = "job_body";
constexpr char kJobStatusColumnName[] = "job_status";
constexpr char kCreatedTimeColumnName[] = "created_time";
constexpr char kUpdatedTimeColumnName[] = "updated_time";
constexpr char kVisibilityTimeoutColumnName[] = "visibility_timeout";
constexpr char kRetryCountColumnName[] = "retry_count";

Any CreateHelloWorldProtoAsAny(google::protobuf::Timestamp created_time) {
  HelloWorld hello_world_input;
  hello_world_input.set_name(kHelloWorldName);
  hello_world_input.set_id(kHelloWorldId);
  *hello_world_input.mutable_created_time() = created_time;

  Any any;
  any.PackFrom(hello_world_input);
  return any;
}

Item CreateJobAsDatabaseItem(
    const Any& job_body, const JobStatus& job_status,
    const google::protobuf::Timestamp& current_time,
    const google::protobuf::Timestamp& updated_time,
    const google::protobuf::Timestamp& visibility_timeout,
    const int retry_count) {
  auto job_body_in_string_or = google::scp::cpio::client_providers::
      JobClientUtils::ConvertAnyToBase64String(job_body);

  Item item;
  *item.mutable_key()->mutable_partition_key() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          kJobsTablePartitionKeyName, kJobId);
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          kJobBodyColumnName, *job_body_in_string_or);
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeIntAttribute(
          kJobStatusColumnName, job_status);
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          kCreatedTimeColumnName, TimeUtil::ToString(current_time));
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          kUpdatedTimeColumnName, TimeUtil::ToString(updated_time));
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          kVisibilityTimeoutColumnName, TimeUtil::ToString(visibility_timeout));
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeIntAttribute(
          kRetryCountColumnName, retry_count);

  return item;
}
}  // namespace

namespace google::scp::cpio::client_providers::test {

class JobClientUtilsTest : public ::testing::TestWithParam<
                               tuple<JobStatus, JobStatus, ExecutionResult>> {
 public:
  JobStatus GetCurrentStatus() const { return get<0>(GetParam()); }

  JobStatus GetUpdateStatus() const { return get<1>(GetParam()); }

  ExecutionResult GetExpectedExecutionResult() const {
    return get<2>(GetParam());
  }
};

TEST(JobClientUtilsTest, MakeStringAttribute) {
  auto name = "name";
  auto value = "value";
  auto item_attribute = JobClientUtils::MakeStringAttribute(name, value);

  EXPECT_EQ(item_attribute.name(), name);
  EXPECT_EQ(item_attribute.value_string(), value);
}

TEST(JobClientUtilsTest, MakeIntAttribute) {
  auto name = "name";
  auto value = 5;
  auto item_attribute = JobClientUtils::MakeIntAttribute(name, value);

  EXPECT_EQ(item_attribute.name(), name);
  EXPECT_EQ(item_attribute.value_int(), value);
}

TEST(JobClientUtilsTest, CreateJob) {
  auto current_time = TimeUtil::GetCurrentTime();
  auto updated_time = current_time + TimeUtil::SecondsToDuration(5);
  auto visibility_timeout = current_time + TimeUtil::SecondsToDuration(30);
  auto job_body = CreateHelloWorldProtoAsAny(current_time);
  auto job_status = JobStatus::JOB_STATUS_CREATED;
  auto retry_count = 3;

  auto job =
      JobClientUtils::CreateJob(kJobId, job_body, job_status, current_time,
                                updated_time, visibility_timeout, retry_count);

  EXPECT_EQ(job.job_id(), kJobId);
  HelloWorld hello_world_output;
  job.job_body().UnpackTo(&hello_world_output);
  EXPECT_EQ(hello_world_output.name(), kHelloWorldName);
  EXPECT_EQ(hello_world_output.id(), kHelloWorldId);
  EXPECT_EQ(hello_world_output.created_time(), current_time);
  EXPECT_EQ(job.job_status(), job_status);
  EXPECT_EQ(job.created_time(), current_time);
  EXPECT_EQ(job.updated_time(), updated_time);
  EXPECT_EQ(job.visibility_timeout(), visibility_timeout);
  EXPECT_EQ(job.retry_count(), retry_count);
}

TEST(JobClientUtilsTest, ConvertAnyToBase64String) {
  auto current_time = TimeUtil::GetCurrentTime();
  auto helloworld = CreateHelloWorldProtoAsAny(current_time);
  auto string_or = JobClientUtils::ConvertAnyToBase64String(helloworld);
  EXPECT_SUCCESS(string_or);

  string decoded_string;
  Base64Decode(*string_or, decoded_string);
  Any any_output;
  any_output.ParseFromString(decoded_string);
  HelloWorld hello_world_output;
  any_output.UnpackTo(&hello_world_output);
  EXPECT_EQ(hello_world_output.name(), kHelloWorldName);
  EXPECT_EQ(hello_world_output.id(), kHelloWorldId);
  EXPECT_EQ(hello_world_output.created_time(), current_time);
}

TEST(JobClientUtilsTest, ConvertBase64StringToAny) {
  auto current_time = TimeUtil::GetCurrentTime();
  auto helloworld = CreateHelloWorldProtoAsAny(current_time);
  string string_input;
  helloworld.SerializeToString(&string_input);
  string encoded_string;
  Base64Encode(string_input, encoded_string);
  auto any_or = JobClientUtils::ConvertBase64StringToAny(encoded_string);
  EXPECT_SUCCESS(any_or);

  HelloWorld hello_world_output;
  any_or->UnpackTo(&hello_world_output);
  EXPECT_EQ(hello_world_output.name(), kHelloWorldName);
  EXPECT_EQ(hello_world_output.id(), kHelloWorldId);
  EXPECT_EQ(hello_world_output.created_time(), current_time);
}

TEST(JobClientUtilsTest, ConvertDatabaseItemToJob) {
  auto current_time = TimeUtil::GetCurrentTime();
  auto job_body = CreateHelloWorldProtoAsAny(current_time);
  auto job_status = JobStatus::JOB_STATUS_PROCESSING;
  auto updated_time = current_time;
  auto visibility_timeout = current_time + TimeUtil::SecondsToDuration(30);
  auto retry_count = 4;
  auto job_or = JobClientUtils::ConvertDatabaseItemToJob(
      CreateJobAsDatabaseItem(job_body, job_status, current_time, updated_time,
                              visibility_timeout, retry_count));

  EXPECT_SUCCESS(job_or);

  auto job = *job_or;
  EXPECT_EQ(job.job_id(), kJobId);
  HelloWorld hello_world_output;
  job.job_body().UnpackTo(&hello_world_output);
  EXPECT_EQ(hello_world_output.name(), kHelloWorldName);
  EXPECT_EQ(hello_world_output.id(), kHelloWorldId);
  EXPECT_EQ(hello_world_output.created_time(), current_time);
  EXPECT_EQ(job.job_status(), job_status);
  EXPECT_EQ(job.created_time(), current_time);
  EXPECT_EQ(job.updated_time(), updated_time);
  EXPECT_EQ(job.visibility_timeout(), visibility_timeout);
  EXPECT_EQ(job.retry_count(), retry_count);
}

TEST(JobClientUtilsTest,
     ConvertDatabaseItemToJobWithAttributesInRandomOrderSuccess) {
  Item item;
  *item.mutable_key()->mutable_partition_key() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          kJobsTablePartitionKeyName, kJobId);

  auto current_time = TimeUtil::GetCurrentTime();
  auto retry_count = 0;
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeIntAttribute(
          kJobStatusColumnName, JobStatus::JOB_STATUS_PROCESSING);
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          kCreatedTimeColumnName, TimeUtil::ToString(current_time));
  auto job_body = CreateHelloWorldProtoAsAny(current_time);
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          kJobBodyColumnName,
          *JobClientUtils::ConvertAnyToBase64String(job_body));
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          kUpdatedTimeColumnName, TimeUtil::ToString(current_time));
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          kVisibilityTimeoutColumnName, TimeUtil::ToString(current_time));
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeIntAttribute(
          kRetryCountColumnName, retry_count);

  EXPECT_SUCCESS(JobClientUtils::ConvertDatabaseItemToJob(item));
}

TEST(JobClientUtilsTest, ConvertDatabaseItemToJobWithValidationFailure) {
  Item item;
  auto job_or = JobClientUtils::ConvertDatabaseItemToJob(item);

  EXPECT_THAT(job_or, ResultIs(FailureExecutionResult(
                          SC_JOB_CLIENT_PROVIDER_INVALID_JOB_ITEM)));
}

TEST(JobClientUtilsTest,
     ConvertDatabaseItemToJobWithColumnNamesMismatchFailure) {
  Item item;
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          "invalid_column_name1", "test");
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          "invalid_column_name2", "test");
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          "invalid_column_name3", "test");
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          "invalid_column_name4", "test");
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          "invalid_column_name5", "test");
  *item.add_attributes() =
      google::scp::cpio::client_providers::JobClientUtils::MakeStringAttribute(
          "invalid_column_name6", "test");

  auto job_or = JobClientUtils::ConvertDatabaseItemToJob(item);

  EXPECT_THAT(job_or.result(), ResultIs(FailureExecutionResult(
                                   SC_JOB_CLIENT_PROVIDER_INVALID_JOB_ITEM)));
}

TEST(JobClientUtilsTest, CreateUpsertJobRequest) {
  auto current_time = TimeUtil::GetCurrentTime();
  auto job_body_input = CreateHelloWorldProtoAsAny(current_time);
  auto job_status = JobStatus::JOB_STATUS_PROCESSING;
  auto updated_time = current_time;
  auto visibility_timeout = current_time + TimeUtil::SecondsToDuration(30);
  auto retry_count = 2;
  auto job = JobClientUtils::CreateJob(kJobId, job_body_input, job_status,
                                       current_time, updated_time,
                                       visibility_timeout, retry_count);

  auto job_body_input_or =
      JobClientUtils::ConvertAnyToBase64String(job_body_input);

  auto request = JobClientUtils::CreateUpsertJobRequest(kJobsTableName, job,
                                                        *job_body_input_or);
  EXPECT_EQ(request->key().table_name(), kJobsTableName);
  EXPECT_EQ(request->key().partition_key().name(), kJobsTablePartitionKeyName);
  EXPECT_EQ(request->key().partition_key().value_string(), kJobId);
  EXPECT_EQ(request->new_attributes().size(), 6);
  EXPECT_EQ(request->new_attributes(0).name(), kJobBodyColumnName);

  EXPECT_EQ(request->new_attributes(0).value_string(), *job_body_input_or);
  EXPECT_EQ(request->new_attributes(1).name(), kJobStatusColumnName);
  EXPECT_EQ(request->new_attributes(1).value_int(), job.job_status());
  EXPECT_EQ(request->new_attributes(2).name(), kCreatedTimeColumnName);
  EXPECT_EQ(request->new_attributes(2).value_string(),
            TimeUtil::ToString(job.created_time()));
  EXPECT_EQ(request->new_attributes(3).name(), kUpdatedTimeColumnName);
  EXPECT_EQ(request->new_attributes(3).value_string(),
            TimeUtil::ToString(job.updated_time()));
  EXPECT_EQ(request->new_attributes(4).name(), kVisibilityTimeoutColumnName);
  EXPECT_EQ(request->new_attributes(4).value_string(),
            TimeUtil::ToString(job.visibility_timeout()));
  EXPECT_EQ(request->new_attributes(5).name(), kRetryCountColumnName);
  EXPECT_EQ(request->new_attributes(5).value_int(), retry_count);
}

TEST(JobClientUtilsTest, CreateUpsertJobRequestWithPartialUpdate) {
  Job job;
  job.set_job_id(kJobId);
  job.set_job_status(JobStatus::JOB_STATUS_PROCESSING);
  *job.mutable_updated_time() = TimeUtil::GetCurrentTime();

  string job_body_as_string = job.job_body().SerializeAsString();
  auto request = JobClientUtils::CreateUpsertJobRequest(kJobsTableName, job);

  EXPECT_EQ(request->key().table_name(), kJobsTableName);
  EXPECT_EQ(request->key().partition_key().name(), kJobsTablePartitionKeyName);
  EXPECT_EQ(request->key().partition_key().value_string(), job.job_id());
  EXPECT_EQ(request->new_attributes().size(), 3);
  EXPECT_EQ(request->new_attributes(0).name(), kJobStatusColumnName);
  EXPECT_EQ(request->new_attributes(0).value_int(), job.job_status());
  EXPECT_EQ(request->new_attributes(1).name(), kUpdatedTimeColumnName);
  EXPECT_EQ(request->new_attributes(1).value_string(),
            TimeUtil::ToString(job.updated_time()));
}

TEST(JobClientUtilsTest, CreateGetJobRequest) {
  auto request = JobClientUtils::CreateGetJobRequest(kJobsTableName, kJobId);

  EXPECT_EQ(request->key().table_name(), kJobsTableName);
  EXPECT_EQ(request->key().partition_key().name(), kJobsTablePartitionKeyName);
  EXPECT_EQ(request->key().partition_key().value_string(), kJobId);
}

INSTANTIATE_TEST_SUITE_P(
    CompletedJobStatus, JobClientUtilsTest,
    testing::Values(
        make_tuple(JobStatus::JOB_STATUS_CREATED,
                   JobStatus::JOB_STATUS_PROCESSING, SuccessExecutionResult()),
        make_tuple(JobStatus::JOB_STATUS_CREATED, JobStatus::JOB_STATUS_SUCCESS,
                   SuccessExecutionResult()),
        make_tuple(JobStatus::JOB_STATUS_CREATED, JobStatus::JOB_STATUS_FAILURE,
                   SuccessExecutionResult()),
        make_tuple(JobStatus::JOB_STATUS_PROCESSING,
                   JobStatus::JOB_STATUS_PROCESSING, SuccessExecutionResult()),
        make_tuple(JobStatus::JOB_STATUS_PROCESSING,
                   JobStatus::JOB_STATUS_SUCCESS, SuccessExecutionResult()),
        make_tuple(JobStatus::JOB_STATUS_PROCESSING,
                   JobStatus::JOB_STATUS_FAILURE, SuccessExecutionResult()),
        make_tuple(
            JobStatus::JOB_STATUS_SUCCESS, JobStatus::JOB_STATUS_PROCESSING,
            FailureExecutionResult(SC_JOB_CLIENT_PROVIDER_INVALID_JOB_STATUS)),
        make_tuple(
            JobStatus::JOB_STATUS_FAILURE, JobStatus::JOB_STATUS_PROCESSING,
            FailureExecutionResult(SC_JOB_CLIENT_PROVIDER_INVALID_JOB_STATUS)),
        make_tuple(
            JobStatus::JOB_STATUS_CREATED, JobStatus::JOB_STATUS_UNKNOWN,
            FailureExecutionResult(SC_JOB_CLIENT_PROVIDER_INVALID_JOB_STATUS)),
        make_tuple(
            JobStatus::JOB_STATUS_PROCESSING, JobStatus::JOB_STATUS_CREATED,
            FailureExecutionResult(SC_JOB_CLIENT_PROVIDER_INVALID_JOB_STATUS)),
        make_tuple(JobStatus::JOB_STATUS_PROCESSING,
                   JobStatus::JOB_STATUS_UNKNOWN,
                   FailureExecutionResult(
                       SC_JOB_CLIENT_PROVIDER_INVALID_JOB_STATUS))));

TEST_P(JobClientUtilsTest, ValidateJobStatus) {
  EXPECT_THAT(
      JobClientUtils::ValidateJobStatus(GetCurrentStatus(), GetUpdateStatus()),
      ResultIs(GetExpectedExecutionResult()));
}

}  // namespace google::scp::cpio::client_providers::test
