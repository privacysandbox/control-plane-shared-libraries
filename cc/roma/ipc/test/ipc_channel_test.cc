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

#include "roma/ipc/src/ipc_channel.h"

#include <gtest/gtest.h>

#include <memory>
#include <thread>

#include "core/test/utils/auto_init_run_stop.h"
#include "public/core/test/interface/execution_result_matchers.h"
#include "roma/common/src/process.h"
#include "roma/common/src/shared_memory.h"
#include "roma/common/src/shared_memory_pool.h"

using google::scp::core::SuccessExecutionResult;
using google::scp::core::test::AutoInitRunStop;
using std::make_unique;
using std::move;
using std::thread;
using std::unique_ptr;

namespace {
constexpr size_t kWorkerQueueCapacity = 100;
}

namespace google::scp::roma::ipc::test {
using common::Process;
using common::SharedMemoryPool;
using common::SharedMemorySegment;

class IpcChannelTest : public ::testing::Test {
 protected:
  void SetUp() override { segment_.Create(100240); }

  void TearDown() override { segment_.Unmap(); }

  SharedMemorySegment segment_;
};

TEST_F(IpcChannelTest, ShouldReturnFailureWhenLastCodeObjectIsEmpty) {
  IpcChannel channel(segment_, kWorkerQueueCapacity);
  AutoInitRunStop auto_init_run_stop(channel);
  channel.GetMemPool().SetThisThreadMemPool();

  unique_ptr<RomaCodeObj> last_code_obj;
  EXPECT_FALSE(channel.GetLastRecordedCodeObjectWithoutInputs(last_code_obj)
                   .Successful());
}

TEST_F(IpcChannelTest, ShouldReturnLastCodeObjectAfterItsRecorded) {
  IpcChannel channel(segment_, kWorkerQueueCapacity);
  AutoInitRunStop auto_init_run_stop(channel);
  channel.GetMemPool().SetThisThreadMemPool();

  auto code_obj = make_unique<CodeObject>();
  code_obj->id = "MyId123";
  code_obj->version_num = 1;
  code_obj->js = "JS";
  Callback callback;
  auto request_to_push = make_unique<Request>(move(code_obj), callback);
  EXPECT_SUCCESS(channel.TryAcquirePushRequest());
  EXPECT_SUCCESS(channel.PushRequest(move(request_to_push)));

  // Should still be empty
  unique_ptr<RomaCodeObj> last_code_obj;
  EXPECT_FALSE(channel.GetLastRecordedCodeObjectWithoutInputs(last_code_obj)
                   .Successful());

  Request* request;
  // The pop makes it be recorded
  EXPECT_SUCCESS(channel.PopRequest(request));

  EXPECT_TRUE(channel.GetLastRecordedCodeObjectWithoutInputs(last_code_obj)
                  .Successful());
  EXPECT_STREQ(last_code_obj->id.c_str(), "MyId123");
}

TEST_F(IpcChannelTest, ShouldNotUpdateLastCodeObjectIfEmpty) {
  IpcChannel channel(segment_, kWorkerQueueCapacity);
  AutoInitRunStop auto_init_run_stop(channel);
  channel.GetMemPool().SetThisThreadMemPool();

  // Empty code object (no JS or WASM)
  auto code_obj = make_unique<CodeObject>();
  code_obj->id = "MyId123";
  code_obj->version_num = 1;
  Callback callback;
  auto request_to_push = make_unique<Request>(move(code_obj), callback);
  EXPECT_SUCCESS(channel.TryAcquirePushRequest());
  EXPECT_SUCCESS(channel.PushRequest(move(request_to_push)));

  // Should be empty
  unique_ptr<RomaCodeObj> last_code_obj;
  EXPECT_FALSE(channel.GetLastRecordedCodeObjectWithoutInputs(last_code_obj)
                   .Successful());

  Request* request;
  EXPECT_SUCCESS(channel.PopRequest(request));

  // Should still be empty
  EXPECT_FALSE(channel.GetLastRecordedCodeObjectWithoutInputs(last_code_obj)
                   .Successful());
}

TEST_F(IpcChannelTest, ShouldUpdateLastCodeObjectIfVersionChanges) {
  IpcChannel channel(segment_, kWorkerQueueCapacity);
  AutoInitRunStop auto_init_run_stop(channel);
  channel.GetMemPool().SetThisThreadMemPool();

  auto code_obj = make_unique<CodeObject>();
  code_obj->id = "MyId123";
  code_obj->version_num = 1;
  code_obj->js = "JS";
  Callback callback;
  auto request_to_push = make_unique<Request>(move(code_obj), callback);
  EXPECT_SUCCESS(channel.TryAcquirePushRequest());
  EXPECT_SUCCESS(channel.PushRequest(move(request_to_push)));

  Request* request;
  EXPECT_SUCCESS(channel.PopRequest(request));
  auto resp = make_unique<Response>();
  // Respond to request to be able to pop next available request
  EXPECT_TRUE(channel.PushResponse(move(resp)));

  unique_ptr<RomaCodeObj> last_code_obj;

  EXPECT_TRUE(channel.GetLastRecordedCodeObjectWithoutInputs(last_code_obj)
                  .Successful());
  EXPECT_STREQ(last_code_obj->id.c_str(), "MyId123");
  EXPECT_EQ(last_code_obj->version_num, 1);
  EXPECT_STREQ(last_code_obj->js.c_str(), "JS");

  code_obj = make_unique<CodeObject>();
  code_obj->id = "MyId123";
  code_obj->version_num = 2;
  code_obj->js = "NewJS";
  request_to_push = make_unique<Request>(move(code_obj), callback);
  EXPECT_SUCCESS(channel.TryAcquirePushRequest());
  EXPECT_SUCCESS(channel.PushRequest(move(request_to_push)));

  EXPECT_SUCCESS(channel.PopRequest(request));

  EXPECT_TRUE(channel.GetLastRecordedCodeObjectWithoutInputs(last_code_obj)
                  .Successful());
  EXPECT_STREQ(last_code_obj->id.c_str(), "MyId123");
  EXPECT_EQ(last_code_obj->version_num, 2);
  EXPECT_STREQ(last_code_obj->js.c_str(), "NewJS");
}

TEST_F(IpcChannelTest, ShouldNotUpdateLastCodeObjectIfVersionDoesNotChange) {
  IpcChannel channel(segment_, kWorkerQueueCapacity);
  AutoInitRunStop auto_init_run_stop(channel);
  channel.GetMemPool().SetThisThreadMemPool();

  auto code_obj = make_unique<CodeObject>();
  code_obj->id = "MyId123";
  code_obj->version_num = 1;
  code_obj->js = "OldJS";
  Callback callback;
  auto request_to_push = make_unique<Request>(move(code_obj), callback);
  EXPECT_SUCCESS(channel.TryAcquirePushRequest());
  EXPECT_SUCCESS(channel.PushRequest(move(request_to_push)));

  Request* request;
  EXPECT_SUCCESS(channel.PopRequest(request));
  auto resp = make_unique<Response>();
  // Respond to request to be able to pop next available request
  EXPECT_TRUE(channel.PushResponse(move(resp)));

  unique_ptr<RomaCodeObj> last_code_obj;

  EXPECT_TRUE(channel.GetLastRecordedCodeObjectWithoutInputs(last_code_obj)
                  .Successful());
  EXPECT_STREQ(last_code_obj->id.c_str(), "MyId123");
  EXPECT_EQ(last_code_obj->version_num, 1);
  EXPECT_STREQ(last_code_obj->js.c_str(), "OldJS");

  code_obj = make_unique<CodeObject>();
  code_obj->id = "MyId123";
  // Same version
  code_obj->version_num = 1;
  code_obj->js = "NewJS";
  request_to_push = make_unique<Request>(move(code_obj), callback);
  EXPECT_SUCCESS(channel.TryAcquirePushRequest());
  EXPECT_SUCCESS(channel.PushRequest(move(request_to_push)));

  EXPECT_SUCCESS(channel.PopRequest(request));

  EXPECT_TRUE(channel.GetLastRecordedCodeObjectWithoutInputs(last_code_obj)
                  .Successful());
  EXPECT_STREQ(last_code_obj->id.c_str(), "MyId123");
  EXPECT_EQ(last_code_obj->version_num, 1);
  EXPECT_STREQ(last_code_obj->js.c_str(), "OldJS");
}

TEST_F(IpcChannelTest, ShouldWorkForSmallSizeWorkQueueWithMultiThread) {
  IpcChannel channel(segment_, 1 /*worker_container queue size*/);
  AutoInitRunStop auto_init_run_stop(channel);
  channel.GetMemPool().SetThisThreadMemPool();

  int total_request = 100;

  std::thread handle_request([&channel, total_request]() {
    channel.GetMemPool().SetThisThreadMemPool();
    for (int i = 0; i < total_request; ++i) {
      Request* request;
      auto result = channel.PopRequest(request);
      EXPECT_SUCCESS(result);
      auto response = make_unique<Response>();
      response->result = SuccessExecutionResult();
      result = channel.PushResponse(move(response));
      EXPECT_SUCCESS(result);
    }
    return SuccessExecutionResult();
  });

  std::thread get_response([&channel, total_request]() {
    channel.GetMemPool().SetThisThreadMemPool();

    for (int i = 0; i < total_request; ++i) {
      std::unique_ptr<Response> response;
      auto result = channel.PopResponse(response);
      EXPECT_SUCCESS(result);
      EXPECT_SUCCESS(response->result);
    }
  });

  for (int i = 0; i < total_request; i++) {
    while (!channel.TryAcquirePushRequest().Successful()) {}

    auto code_obj = make_unique<CodeObject>();
    code_obj->id = std::to_string(i);
    Callback callback;
    auto request_to_push = make_unique<Request>(move(code_obj), callback);

    EXPECT_SUCCESS(channel.PushRequest(move(request_to_push)));
  }

  handle_request.join();
  get_response.join();
}
}  // namespace google::scp::roma::ipc::test
