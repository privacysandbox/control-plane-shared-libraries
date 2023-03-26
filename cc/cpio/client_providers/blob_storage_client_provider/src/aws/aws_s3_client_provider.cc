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

#include "aws_s3_client_provider.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <aws/core/Aws.h>
#include <aws/core/utils/Outcome.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>

#include "absl/strings/str_cat.h"
#include "core/async_executor/src/aws/aws_async_executor.h"
#include "core/utils/src/base64.h"
#include "core/utils/src/hashing.h"
#include "cpio/client_providers/blob_storage_client_provider/src/aws/aws_s3_utils.h"
#include "cpio/client_providers/blob_storage_client_provider/src/common/error_codes.h"
#include "cpio/common/src/aws/aws_utils.h"

using Aws::MakeShared;
using Aws::String;
using Aws::StringStream;
using Aws::Client::AsyncCallerContext;
using Aws::Client::ClientConfiguration;
using Aws::S3::S3Client;
using Aws::S3::Model::DeleteObjectOutcome;
using Aws::S3::Model::DeleteObjectRequest;
using Aws::S3::Model::DeleteObjectResult;
using Aws::S3::Model::GetObjectOutcome;
using Aws::S3::Model::GetObjectRequest;
using Aws::S3::Model::GetObjectResult;
using Aws::S3::Model::ListObjectsOutcome;
using Aws::S3::Model::ListObjectsRequest;
using Aws::S3::Model::ListObjectsResult;
using Aws::S3::Model::PutObjectOutcome;
using Aws::S3::Model::PutObjectRequest;
using Aws::S3::Model::PutObjectResult;
using google::cmrt::sdk::blob_storage_service::v1::Blob;
using google::cmrt::sdk::blob_storage_service::v1::BlobMetadata;
using google::cmrt::sdk::blob_storage_service::v1::DeleteBlobRequest;
using google::cmrt::sdk::blob_storage_service::v1::DeleteBlobResponse;
using google::cmrt::sdk::blob_storage_service::v1::GetBlobRequest;
using google::cmrt::sdk::blob_storage_service::v1::GetBlobResponse;
using google::cmrt::sdk::blob_storage_service::v1::GetBlobStreamRequest;
using google::cmrt::sdk::blob_storage_service::v1::GetBlobStreamResponse;
using google::cmrt::sdk::blob_storage_service::v1::ListBlobsMetadataRequest;
using google::cmrt::sdk::blob_storage_service::v1::ListBlobsMetadataResponse;
using google::cmrt::sdk::blob_storage_service::v1::PutBlobRequest;
using google::cmrt::sdk::blob_storage_service::v1::PutBlobResponse;
using google::cmrt::sdk::blob_storage_service::v1::PutBlobStreamRequest;
using google::cmrt::sdk::blob_storage_service::v1::PutBlobStreamResponse;
using google::scp::core::AsyncContext;
using google::scp::core::AsyncExecutorInterface;
using google::scp::core::AsyncPriority;
using google::scp::core::ClientStreamingContext;
using google::scp::core::ExecutionResult;
using google::scp::core::ExecutionResultOr;
using google::scp::core::FailureExecutionResult;
using google::scp::core::ServerStreamingContext;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::async_executor::aws::AwsAsyncExecutor;
using google::scp::core::common::kZeroUuid;
using google::scp::core::errors::SC_BLOB_STORAGE_PROVIDER_ERROR_GETTING_BLOB;
using google::scp::core::errors::SC_BLOB_STORAGE_PROVIDER_INVALID_ARGS;
using google::scp::core::errors::SC_BLOB_STORAGE_PROVIDER_RETRIABLE_ERROR;
using google::scp::core::utils::Base64Encode;
using google::scp::core::utils::CalculateMd5Hash;
using std::bind;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::string;
using std::vector;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

static constexpr char kAwsS3Provider[] = "AwsS3ClientProvider";
static constexpr size_t kMaxConcurrentConnections = 1000;
static constexpr size_t kListBlobsMetadataMaxResults = 1000;

namespace google::scp::cpio::client_providers {

ExecutionResult AwsS3ClientProvider::Init() noexcept {
  string region;
  auto result = instance_client_->GetCurrentInstanceRegion(region);
  if (!result.Successful()) {
    ERROR(kAwsS3Provider, kZeroUuid, kZeroUuid, result,
          "Failed getting region.");
    return result;
  }
  auto client_or = s3_factory_->CreateClient(region, io_async_executor_);
  if (!client_or.Successful()) {
    ERROR(kAwsS3Provider, kZeroUuid, kZeroUuid, client_or.result(),
          "Failed creating AWS S3 client.");
    return client_or.result();
  }
  s3_client_ = std::move(*client_or);
  return SuccessExecutionResult();
}

ExecutionResult AwsS3ClientProvider::Run() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult AwsS3ClientProvider::Stop() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult AwsS3ClientProvider::GetBlob(
    AsyncContext<GetBlobRequest, GetBlobResponse>& get_blob_context) noexcept {
  const auto& request = *get_blob_context.request;
  if (request.blob_metadata().bucket_name().empty() ||
      request.blob_metadata().blob_name().empty()) {
    get_blob_context.result =
        FailureExecutionResult(SC_BLOB_STORAGE_PROVIDER_INVALID_ARGS);
    ERROR_CONTEXT(kAwsS3Provider, get_blob_context, get_blob_context.result,
                  "Get blob request is missing bucket or blob name");
    get_blob_context.Finish();
    return get_blob_context.result;
  }
  if (request.has_byte_range() && request.byte_range().begin_byte_index() >
                                      request.byte_range().end_byte_index()) {
    get_blob_context.result =
        FailureExecutionResult(SC_BLOB_STORAGE_PROVIDER_INVALID_ARGS);
    ERROR_CONTEXT(kAwsS3Provider, get_blob_context, get_blob_context.result,
                  "Get blob request provides begin_byte_index that is larger "
                  "than end_byte_index");
    get_blob_context.Finish();
    return get_blob_context.result;
  }

  String bucket_name(request.blob_metadata().bucket_name());
  String blob_name(request.blob_metadata().blob_name());
  GetObjectRequest get_object_request;
  get_object_request.SetBucket(bucket_name);
  get_object_request.SetKey(blob_name);
  if (request.has_byte_range()) {
    // SetRange is inclusive on both ends.
    get_object_request.SetRange(
        absl::StrCat("bytes=", request.byte_range().begin_byte_index(), "-",
                     request.byte_range().end_byte_index()));
  }

  s3_client_->GetObjectAsync(get_object_request,
                             bind(&AwsS3ClientProvider::OnGetObjectCallback,
                                  this, get_blob_context, _1, _2, _3, _4),
                             nullptr);

  return SuccessExecutionResult();
}

void AwsS3ClientProvider::OnGetObjectCallback(
    AsyncContext<GetBlobRequest, GetBlobResponse>& get_blob_context,
    const S3Client* s3_client, const GetObjectRequest& get_object_request,
    GetObjectOutcome get_object_outcome,
    const shared_ptr<const AsyncCallerContext> async_context) noexcept {
  if (!get_object_outcome.IsSuccess()) {
    get_blob_context.result = AwsS3Utils::ConvertS3ErrorToExecutionResult(
        get_object_outcome.GetError().GetErrorType());

    ERROR_CONTEXT(kAwsS3Provider, get_blob_context, get_blob_context.result,
                  "Get blob request failed. Error code: %d, message: %s",
                  get_object_outcome.GetError().GetResponseCode(),
                  get_object_outcome.GetError().GetMessage().c_str());
    FinishContext(get_blob_context.result, get_blob_context,
                  cpu_async_executor_, AsyncPriority::High);
    return;
  }

  auto& result = get_object_outcome.GetResult();
  auto& body = result.GetBody();
  auto content_length = result.GetContentLength();

  get_blob_context.response = make_shared<GetBlobResponse>();
  get_blob_context.response->mutable_blob()->mutable_metadata()->CopyFrom(
      get_blob_context.request->blob_metadata());
  get_blob_context.response->mutable_blob()->mutable_data()->resize(
      content_length);
  get_blob_context.result = SuccessExecutionResult();

  if (!body.read(
          get_blob_context.response->mutable_blob()->mutable_data()->data(),
          content_length)) {
    get_blob_context.result =
        FailureExecutionResult(SC_BLOB_STORAGE_PROVIDER_ERROR_GETTING_BLOB);
  }
  FinishContext(get_blob_context.result, get_blob_context, cpu_async_executor_,
                AsyncPriority::High);
}

ExecutionResult AwsS3ClientProvider::GetBlobStream(
    ServerStreamingContext<GetBlobStreamRequest, GetBlobStreamResponse>&
        get_blob_stream_context) noexcept {
  // TODO implement.
  return FailureExecutionResult(SC_UNKNOWN);
}

ExecutionResult AwsS3ClientProvider::ListBlobsMetadata(
    AsyncContext<ListBlobsMetadataRequest, ListBlobsMetadataResponse>&
        list_blobs_context) noexcept {
  const auto& request = *list_blobs_context.request;
  if (request.blob_metadata().bucket_name().empty()) {
    list_blobs_context.result =
        FailureExecutionResult(SC_BLOB_STORAGE_PROVIDER_INVALID_ARGS);
    ERROR_CONTEXT(kAwsS3Provider, list_blobs_context, list_blobs_context.result,
                  "List blobs metadata request failed. Bucket name empty.");
    list_blobs_context.Finish();
    return list_blobs_context.result;
  }
  if (request.has_max_page_size() &&
      request.max_page_size() > kListBlobsMetadataMaxResults) {
    list_blobs_context.result =
        FailureExecutionResult(SC_BLOB_STORAGE_PROVIDER_INVALID_ARGS);
    ERROR_CONTEXT(kAwsS3Provider, list_blobs_context, list_blobs_context.result,
                  "List blobs metadata request failed. Max page size cannot be "
                  "greater than 1000.");
    return list_blobs_context.result;
  }
  String bucket_name(list_blobs_context.request->blob_metadata().bucket_name());

  ListObjectsRequest list_objects_request;
  list_objects_request.SetBucket(bucket_name);
  list_objects_request.SetMaxKeys(
      list_blobs_context.request->has_max_page_size()
          ? list_blobs_context.request->max_page_size()
          : kListBlobsMetadataMaxResults);

  if (!list_blobs_context.request->blob_metadata().blob_name().empty()) {
    String blob_name(list_blobs_context.request->blob_metadata().blob_name());
    list_objects_request.SetPrefix(blob_name);
  }

  if (list_blobs_context.request->has_page_token()) {
    String marker(list_blobs_context.request->page_token());
    list_objects_request.SetMarker(marker);
  }

  s3_client_->ListObjectsAsync(
      list_objects_request,
      bind(&AwsS3ClientProvider::OnListObjectsMetadataCallback, this,
           list_blobs_context, _1, _2, _3, _4),
      nullptr);

  return SuccessExecutionResult();
}

void AwsS3ClientProvider::OnListObjectsMetadataCallback(
    AsyncContext<ListBlobsMetadataRequest, ListBlobsMetadataResponse>&
        list_blobs_metadata_context,
    const S3Client* s3_client, const ListObjectsRequest& list_objects_request,
    ListObjectsOutcome list_objects_outcome,
    const shared_ptr<const AsyncCallerContext> async_context) noexcept {
  if (!list_objects_outcome.IsSuccess()) {
    list_blobs_metadata_context.result =
        AwsS3Utils::ConvertS3ErrorToExecutionResult(
            list_objects_outcome.GetError().GetErrorType());
    ERROR_CONTEXT(kAwsS3Provider, list_blobs_metadata_context,
                  list_blobs_metadata_context.result,
                  "List blobs request failed. Error code: %d, message: %s",
                  list_objects_outcome.GetError().GetResponseCode(),
                  list_objects_outcome.GetError().GetMessage().c_str());
    FinishContext(list_blobs_metadata_context.result,
                  list_blobs_metadata_context, cpu_async_executor_,
                  AsyncPriority::High);
    return;
  }

  list_blobs_metadata_context.response =
      make_shared<ListBlobsMetadataResponse>();
  auto* blob_metadatas =
      list_blobs_metadata_context.response->mutable_blob_metadatas();
  for (auto& object : list_objects_outcome.GetResult().GetContents()) {
    BlobMetadata metadata;
    metadata.set_blob_name(object.GetKey());
    metadata.set_bucket_name(
        list_blobs_metadata_context.request->blob_metadata().bucket_name());

    blob_metadatas->Add(move(metadata));
  }

  list_blobs_metadata_context.response->set_next_page_token(
      list_objects_outcome.GetResult().GetNextMarker().c_str());

  list_blobs_metadata_context.result = SuccessExecutionResult();
  FinishContext(list_blobs_metadata_context.result, list_blobs_metadata_context,
                cpu_async_executor_, AsyncPriority::High);
}

ExecutionResult AwsS3ClientProvider::PutBlob(
    AsyncContext<PutBlobRequest, PutBlobResponse>& put_blob_context) noexcept {
  const auto& request = *put_blob_context.request;
  if (request.blob().metadata().bucket_name().empty() ||
      request.blob().metadata().blob_name().empty() ||
      request.blob().data().empty()) {
    put_blob_context.result =
        FailureExecutionResult(SC_BLOB_STORAGE_PROVIDER_INVALID_ARGS);
    ERROR_CONTEXT(kAwsS3Provider, put_blob_context, put_blob_context.result,
                  "Put blob request failed. Ensure that bucket name, blob "
                  "name, and data are present.");
    put_blob_context.Finish();
    return put_blob_context.result;
  }

  String bucket_name(request.blob().metadata().bucket_name());
  String blob_name(request.blob().metadata().blob_name());

  PutObjectRequest put_object_request;
  put_object_request.SetBucket(bucket_name);
  put_object_request.SetKey(blob_name);

  string md5_checksum;
  auto execution_result = CalculateMd5Hash(request.blob().data(), md5_checksum);
  if (!execution_result.Successful()) {
    ERROR_CONTEXT(kAwsS3Provider, put_blob_context, execution_result,
                  "MD5 Hash generation failed");
    put_blob_context.result = execution_result;
    put_blob_context.Finish();
    return execution_result;
  }

  string base64_md5_checksum;
  execution_result = Base64Encode(md5_checksum, base64_md5_checksum);
  if (!execution_result.Successful()) {
    ERROR_CONTEXT(kAwsS3Provider, put_blob_context, execution_result,
                  "Encoding MD5 to base64 failed");
    put_blob_context.result = execution_result;
    put_blob_context.Finish();
    return execution_result;
  }

  auto input_data = Aws::MakeShared<Aws::StringStream>(
      "PutObjectInputStream", std::stringstream::in | std::stringstream::out |
                                  std::stringstream::binary);
  input_data->write(request.blob().data().c_str(),
                    request.blob().data().size());

  put_object_request.SetBody(input_data);
  put_object_request.SetContentMD5(base64_md5_checksum.c_str());

  s3_client_->PutObjectAsync(put_object_request,
                             bind(&AwsS3ClientProvider::OnPutObjectCallback,
                                  this, put_blob_context, _1, _2, _3, _4),
                             nullptr);

  return SuccessExecutionResult();
}

void AwsS3ClientProvider::OnPutObjectCallback(
    AsyncContext<PutBlobRequest, PutBlobResponse>& put_blob_context,
    const S3Client* s3_client, const PutObjectRequest& put_object_request,
    PutObjectOutcome put_object_outcome,
    const shared_ptr<const AsyncCallerContext> async_context) noexcept {
  if (!put_object_outcome.IsSuccess()) {
    put_blob_context.result = AwsS3Utils::ConvertS3ErrorToExecutionResult(
        put_object_outcome.GetError().GetErrorType());
    ERROR_CONTEXT(kAwsS3Provider, put_blob_context, put_blob_context.result,
                  "Put blob request failed. Error code: %d, message: %s",
                  put_object_outcome.GetError().GetResponseCode(),
                  put_object_outcome.GetError().GetMessage().c_str());
    FinishContext(put_blob_context.result, put_blob_context,
                  cpu_async_executor_, AsyncPriority::High);
    return;
  }
  put_blob_context.response = make_shared<PutBlobResponse>();
  put_blob_context.result = SuccessExecutionResult();
  FinishContext(put_blob_context.result, put_blob_context, cpu_async_executor_,
                AsyncPriority::High);
}

ExecutionResult AwsS3ClientProvider::PutBlobStream(
    ClientStreamingContext<PutBlobStreamRequest, PutBlobStreamResponse>&
        put_blob_stream_context) noexcept {
  return FailureExecutionResult(SC_UNKNOWN);
}

ExecutionResult AwsS3ClientProvider::DeleteBlob(
    AsyncContext<DeleteBlobRequest, DeleteBlobResponse>&
        delete_blob_context) noexcept {
  const auto& request = *delete_blob_context.request;
  if (request.blob_metadata().bucket_name().empty() ||
      request.blob_metadata().blob_name().empty()) {
    delete_blob_context.result =
        FailureExecutionResult(SC_BLOB_STORAGE_PROVIDER_INVALID_ARGS);
    ERROR_CONTEXT(kAwsS3Provider, delete_blob_context,
                  delete_blob_context.result,
                  "Delete blob request failed. Missing bucket or blob name.");
    delete_blob_context.Finish();
    return delete_blob_context.result;
  }
  String bucket_name(request.blob_metadata().bucket_name());
  String blob_name(request.blob_metadata().blob_name());

  DeleteObjectRequest delete_object_request;
  delete_object_request.SetBucket(bucket_name);
  delete_object_request.SetKey(blob_name);

  s3_client_->DeleteObjectAsync(
      delete_object_request,
      bind(&AwsS3ClientProvider::OnDeleteObjectCallback, this,
           delete_blob_context, _1, _2, _3, _4),
      nullptr);

  return SuccessExecutionResult();
}

void AwsS3ClientProvider::OnDeleteObjectCallback(
    AsyncContext<DeleteBlobRequest, DeleteBlobResponse>& delete_blob_context,
    const S3Client* s3_client, const DeleteObjectRequest& delete_object_request,
    DeleteObjectOutcome delete_object_outcome,
    const shared_ptr<const AsyncCallerContext> async_context) noexcept {
  if (!delete_object_outcome.IsSuccess()) {
    delete_blob_context.result = AwsS3Utils::ConvertS3ErrorToExecutionResult(
        delete_object_outcome.GetError().GetErrorType());
    ERROR_CONTEXT(kAwsS3Provider, delete_blob_context,
                  delete_blob_context.result,
                  "Delete blob request failed. Error code: %d, "
                  "message: %s",
                  delete_object_outcome.GetError().GetResponseCode(),
                  delete_object_outcome.GetError().GetMessage().c_str());
    FinishContext(delete_blob_context.result, delete_blob_context,
                  cpu_async_executor_, AsyncPriority::High);
    return;
  }
  delete_blob_context.response = make_shared<DeleteBlobResponse>();
  delete_blob_context.result = SuccessExecutionResult();
  FinishContext(delete_blob_context.result, delete_blob_context,
                cpu_async_executor_, AsyncPriority::High);
}

ExecutionResultOr<shared_ptr<S3Client>> AwsS3Factory::CreateClient(
    const string& region,
    shared_ptr<AsyncExecutorInterface> async_executor) noexcept {
  auto client_config =
      common::CreateClientConfiguration(make_shared<string>(region));
  client_config->maxConnections = kMaxConcurrentConnections;
  client_config->executor = make_shared<AwsAsyncExecutor>(async_executor);

  return make_shared<S3Client>(*client_config);
}

shared_ptr<BlobStorageClientProviderInterface>
BlobStorageClientProviderFactory::Create(
    shared_ptr<InstanceClientProviderInterface> instance_client,
    shared_ptr<core::AsyncExecutorInterface> cpu_async_executor,
    shared_ptr<core::AsyncExecutorInterface> io_async_executor) noexcept {
  return make_shared<AwsS3ClientProvider>(instance_client, cpu_async_executor,
                                          io_async_executor);
}
}  // namespace google::scp::cpio::client_providers
