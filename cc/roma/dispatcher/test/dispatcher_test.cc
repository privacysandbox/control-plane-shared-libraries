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

#include "roma/dispatcher/src/dispatcher.h"

#include <gtest/gtest.h>

#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "core/test/utils/auto_init_run_stop.h"
#include "core/test/utils/conditional_wait.h"
#include "public/core/test/interface/execution_result_matchers.h"

using absl::Status;
using absl::StatusOr;
using google::scp::core::test::AutoInitRunStop;
using google::scp::core::test::WaitUntil;
using std::atomic;
using std::make_unique;
using std::move;
using std::string;
using std::unique_ptr;
using std::vector;

namespace google::scp::roma::dispatcher::test {
using core::FailureExecutionResult;
using core::SuccessExecutionResult;
using roma::common::RoleId;
using roma::ipc::IpcManager;
using roma::ipc::Request;
using roma::ipc::RequestType;
using roma::ipc::Response;
using roma::ipc::RomaCodeResponse;

TEST(DispatcherTest, testDispatch) {
  Config config;
  config.number_of_workers = 1;
  // using a unique_ptr so that we deallocate after test done
  unique_ptr<IpcManager> ipc_manager(IpcManager::Create(config));
  AutoInitRunStop auto_init_run_stop(*ipc_manager);
  Dispatcher dispatcher(*ipc_manager);
  dispatcher.Init();
  dispatcher.Run();
  pid_t worker_pid = fork();
  if (worker_pid == 0) {  // child
    ipc_manager->SetUpIpcForMyProcess(RoleId(0, false));
    auto& ipc_channel = ipc_manager->GetIpcChannel();
    Request* req;

    auto result = ipc_channel.PopRequest(req);
    EXPECT_SUCCESS(result);
    EXPECT_EQ(req->code_obj->input[0], "test");

    auto resp = make_unique<Response>();
    resp->result = SuccessExecutionResult();
    resp->response = make_unique<RomaCodeResponse>();

    result = ipc_channel.PushResponse(move(resp));
    EXPECT_SUCCESS(result);

    exit(0);
  }

  auto code_obj = make_unique<InvocationRequestStrInput>();
  code_obj->input.emplace_back("test");
  atomic<bool> finished;
  finished.store(false);

  auto result = dispatcher.Dispatch(
      move(code_obj),
      [&](unique_ptr<StatusOr<ResponseObject>>) { finished.store(true); });
  EXPECT_SUCCESS(result);

  WaitUntil([&]() { return finished.load(); });

  EXPECT_TRUE(finished.load());
  ipc_manager->ReleaseLocks();
  dispatcher.Stop();
}

TEST(DispatcherTest, testDispatchSharedInput) {
  Config config;
  config.number_of_workers = 1;
  // using a unique_ptr so that we deallocate after test done
  unique_ptr<IpcManager> ipc_manager(IpcManager::Create(config));
  AutoInitRunStop auto_init_run_stop(*ipc_manager);
  Dispatcher dispatcher(*ipc_manager);
  dispatcher.Init();
  dispatcher.Run();
  pid_t worker_pid = fork();
  if (worker_pid == 0) {  // child
    ipc_manager->SetUpIpcForMyProcess(RoleId(0, false));
    auto& ipc_channel = ipc_manager->GetIpcChannel();
    Request* req;

    auto result = ipc_channel.PopRequest(req);
    EXPECT_SUCCESS(result);
    EXPECT_EQ(req->code_obj->input[0], "test");

    auto resp = make_unique<Response>();
    resp->result = SuccessExecutionResult();
    resp->response = make_unique<RomaCodeResponse>();

    result = ipc_channel.PushResponse(move(resp));
    EXPECT_SUCCESS(result);

    exit(0);
  }

  auto code_obj = make_unique<InvocationRequestSharedInput>();
  code_obj->input.emplace_back(make_unique<string>("test"));
  atomic<bool> finished;
  finished.store(false);

  auto result = dispatcher.Dispatch(
      move(code_obj),
      [&](unique_ptr<StatusOr<ResponseObject>>) { finished.store(true); });
  EXPECT_SUCCESS(result);

  WaitUntil([&]() { return finished.load(); });

  EXPECT_TRUE(finished.load());
  ipc_manager->ReleaseLocks();
  dispatcher.Stop();
}

TEST(DispatcherTest, testRoundRobin) {
  Config config;
  config.number_of_workers = 2;
  // using a unique_ptr so that we deallocate after test done
  unique_ptr<IpcManager> ipc_manager(IpcManager::Create(config));
  AutoInitRunStop auto_init_run_stop(*ipc_manager);
  Dispatcher dispatcher(*ipc_manager);
  dispatcher.Init();
  dispatcher.Run();
  pid_t worker_pid0 = fork();
  if (worker_pid0 == 0) {  // child
    ipc_manager->SetUpIpcForMyProcess(RoleId(0, false));
    auto& ipc_channel = ipc_manager->GetIpcChannel();
    Request* req;

    auto result = ipc_channel.PopRequest(req);
    EXPECT_SUCCESS(result);

    auto resp = make_unique<Response>();
    resp->result = SuccessExecutionResult();
    resp->response = make_unique<RomaCodeResponse>();
    resp->response->id = "0";
    ipc_channel.PushResponse(move(resp));

    exit(0);
  }

  pid_t worker_pid1 = fork();
  if (worker_pid1 == 0) {  // child
    ipc_manager->SetUpIpcForMyProcess(RoleId(1, false));
    auto& ipc_channel = ipc_manager->GetIpcChannel();
    Request* req;

    auto result = ipc_channel.PopRequest(req);
    EXPECT_SUCCESS(result);

    auto resp = make_unique<Response>();
    resp->result = SuccessExecutionResult();
    resp->response = make_unique<RomaCodeResponse>();
    resp->response->id = "1";

    result = ipc_channel.PushResponse(move(resp));
    EXPECT_SUCCESS(result);

    exit(0);
  }

  auto code_obj = make_unique<InvocationRequestStrInput>();
  atomic<bool> finished_0{false};
  auto result = dispatcher.Dispatch(
      move(code_obj), [&](unique_ptr<StatusOr<ResponseObject>> resp) {
        auto& code_resp = **resp;
        EXPECT_EQ(code_resp.id, "0");
        finished_0.store(true);
      });

  atomic<bool> finished_1{false};
  code_obj = make_unique<InvocationRequestStrInput>();
  result = dispatcher.Dispatch(move(code_obj),
                               [&](unique_ptr<StatusOr<ResponseObject>> resp) {
                                 auto& code_resp = **resp;
                                 EXPECT_EQ(code_resp.id, "1");
                                 finished_1.store(true);
                               });

  WaitUntil([&]() { return finished_0.load(); });
  WaitUntil([&]() { return finished_1.load(); });
  EXPECT_TRUE(finished_0.load());
  EXPECT_TRUE(finished_1.load());

  ipc_manager->ReleaseLocks();
  dispatcher.Stop();
}

TEST(DispatcherTest, testDispatchBatch) {
  Config config;
  config.number_of_workers = 5;
  // using a unique_ptr so that we deallocate after test done
  unique_ptr<IpcManager> ipc_manager(IpcManager::Create(config));
  AutoInitRunStop auto_init_run_stop(*ipc_manager);
  Dispatcher dispatcher(*ipc_manager);
  dispatcher.Init();
  dispatcher.Run();

  for (size_t idx = 0; idx < 5; ++idx) {
    pid_t worker_pid = fork();
    if (worker_pid == 0) {  // child
      ipc_manager->SetUpIpcForMyProcess(RoleId(idx, false));
      auto& ipc_channel = ipc_manager->GetIpcChannel();
      Request* req;

      auto result = ipc_channel.PopRequest(req);
      EXPECT_SUCCESS(result);

      EXPECT_EQ(req->code_obj->input[0], "test");

      auto resp = make_unique<Response>();
      resp->result = SuccessExecutionResult();
      resp->response = make_unique<RomaCodeResponse>();
      resp->response->id = static_cast<char>(idx);

      result = ipc_channel.PushResponse(move(resp));
      EXPECT_SUCCESS(result);

      exit(0);
    }
  }

  auto code_obj = InvocationRequestStrInput();
  code_obj.input.emplace_back("test");
  vector<InvocationRequestStrInput> batch(5, code_obj);
  atomic<bool> finished(false);
  auto result = dispatcher.DispatchBatch(
      batch,
      [&](const std::vector<absl::StatusOr<ResponseObject>>& batch_response) {
        finished.store(true);
      });

  EXPECT_SUCCESS(result);
  WaitUntil([&]() { return finished.load(); });
  EXPECT_TRUE(finished.load());
  ipc_manager->ReleaseLocks();
  dispatcher.Stop();
}

TEST(DispatcherTest, testDispatchBatchSharedInput) {
  Config config;
  config.number_of_workers = 5;
  // using a unique_ptr so that we deallocate after test done
  unique_ptr<IpcManager> ipc_manager(IpcManager::Create(config));
  AutoInitRunStop auto_init_run_stop(*ipc_manager);
  Dispatcher dispatcher(*ipc_manager);
  dispatcher.Init();
  dispatcher.Run();

  for (size_t idx = 0; idx < 5; ++idx) {
    pid_t worker_pid = fork();
    if (worker_pid == 0) {  // child
      ipc_manager->SetUpIpcForMyProcess(RoleId(idx, false));
      auto& ipc_channel = ipc_manager->GetIpcChannel();
      Request* req;

      auto result = ipc_channel.PopRequest(req);
      EXPECT_SUCCESS(result);
      EXPECT_EQ(req->code_obj->input[0], "test");

      auto resp = make_unique<Response>();
      resp->result = SuccessExecutionResult();
      resp->response = make_unique<RomaCodeResponse>();
      resp->response->id = static_cast<char>(idx);

      result = ipc_channel.PushResponse(move(resp));
      EXPECT_SUCCESS(result);

      exit(0);
    }
  }

  auto code_obj = InvocationRequestSharedInput();
  code_obj.input.emplace_back(make_unique<string>("test"));
  vector<InvocationRequestSharedInput> batch(5, code_obj);
  atomic<bool> finished(false);
  auto result = dispatcher.DispatchBatch(
      batch,
      [&](const std::vector<absl::StatusOr<ResponseObject>>& batch_response) {
        finished.store(true);
      });

  EXPECT_SUCCESS(result);
  WaitUntil([&]() { return finished.load(); });
  EXPECT_TRUE(finished.load());
  ipc_manager->ReleaseLocks();
  dispatcher.Stop();
}

TEST(DispatcherTest, testDispatchBatchFailedWithQueueFull) {
  Config config;
  config.number_of_workers = 1;
  config.worker_queue_max_items = 5;
  // using a unique_ptr so that we deallocate after test done
  unique_ptr<IpcManager> ipc_manager(IpcManager::Create(config));
  AutoInitRunStop auto_init_run_stop(*ipc_manager);
  Dispatcher dispatcher(*ipc_manager);
  dispatcher.Init();
  dispatcher.Run();

  auto code_obj = InvocationRequestSharedInput();
  code_obj.input.emplace_back(make_unique<string>("test"));
  vector<InvocationRequestSharedInput> batch(5, code_obj);
  auto result = dispatcher.DispatchBatch(
      batch,
      [&](const std::vector<absl::StatusOr<ResponseObject>>& batch_response) {
      });
  EXPECT_SUCCESS(result);

  // DispatchBatch will failure as the worker queue is full.
  result = dispatcher.DispatchBatch(
      batch,
      [&](const std::vector<absl::StatusOr<ResponseObject>>& batch_response) {
      });
  EXPECT_FALSE(result.Successful());

  ipc_manager->ReleaseLocks();
  dispatcher.Stop();
}

TEST(DispatcherTest, testBroadcastSuccess) {
  Config config;
  config.number_of_workers = 5;
  // using a unique_ptr so that we deallocate after test done
  unique_ptr<IpcManager> ipc_manager(IpcManager::Create(config));
  AutoInitRunStop auto_init_run_stop(*ipc_manager);
  Dispatcher dispatcher(*ipc_manager);
  dispatcher.Init();
  dispatcher.Run();

  for (size_t idx = 0; idx < 5; ++idx) {
    pid_t worker_pid = fork();
    if (worker_pid == 0) {  // child
      ipc_manager->SetUpIpcForMyProcess(RoleId(idx, false));
      auto& ipc_channel = ipc_manager->GetIpcChannel();
      Request* req;

      auto result = ipc_channel.PopRequest(req);
      EXPECT_SUCCESS(result);
      EXPECT_EQ(req->type, RequestType::kUpdate);

      auto resp = make_unique<Response>();
      resp->result = SuccessExecutionResult();
      resp->response = make_unique<RomaCodeResponse>();
      resp->response->id = static_cast<char>(idx);

      result = ipc_channel.PushResponse(move(resp));
      EXPECT_SUCCESS(result);

      exit(0);
    }
  }

  auto code_obj = make_unique<CodeObject>();
  atomic<bool> finished(false);
  auto result = dispatcher.Broadcast(
      move(code_obj), [&](unique_ptr<StatusOr<ResponseObject>> resp) {
        EXPECT_TRUE(resp->ok());
        finished.store(true);
      });

  EXPECT_SUCCESS(result);
  WaitUntil([&]() { return finished.load(); });
  EXPECT_TRUE(finished.load());
  ipc_manager->ReleaseLocks();
  dispatcher.Stop();
}

TEST(DispatcherTest, testBroadcastFailed) {
  Config config;
  config.number_of_workers = 5;
  // using a unique_ptr so that we deallocate after test done
  unique_ptr<IpcManager> ipc_manager(IpcManager::Create(config));
  AutoInitRunStop auto_init_run_stop(*ipc_manager);
  Dispatcher dispatcher(*ipc_manager);
  dispatcher.Init();
  dispatcher.Run();

  for (size_t idx = 0; idx < 5; ++idx) {
    pid_t worker_pid = fork();
    if (worker_pid == 0) {  // child
      ipc_manager->SetUpIpcForMyProcess(RoleId(idx, false));
      auto& ipc_channel = ipc_manager->GetIpcChannel();
      Request* req;

      auto result = ipc_channel.PopRequest(req);
      // Broadcast got the failure result, and the unit test parent process
      // called ipc_manager->ReleaseLocks().
      if (!result.Successful()) {
        exit(0);
      }

      EXPECT_SUCCESS(result);
      EXPECT_EQ(req->type, RequestType::kUpdate);

      auto resp = make_unique<Response>();
      // Failed one worker.
      if (idx == 1) {
        resp->result = FailureExecutionResult(SC_UNKNOWN);
      } else {
        resp->result = SuccessExecutionResult();
      }
      resp->response = make_unique<RomaCodeResponse>();
      resp->response->id = static_cast<char>(idx);

      result = ipc_channel.PushResponse(move(resp));
      EXPECT_SUCCESS(result);

      exit(0);
    }
  }
  auto code_obj = make_unique<CodeObject>();
  atomic<bool> finished(false);
  auto result = dispatcher.Broadcast(
      move(code_obj), [&](unique_ptr<StatusOr<ResponseObject>> resp) {
        EXPECT_TRUE(!resp->ok());
        finished.store(true);
      });

  EXPECT_SUCCESS(result);
  WaitUntil([&]() { return finished.load(); });
  EXPECT_TRUE(finished.load());
  ipc_manager->ReleaseLocks();
  dispatcher.Stop();
}

}  // namespace google::scp::roma::dispatcher::test
