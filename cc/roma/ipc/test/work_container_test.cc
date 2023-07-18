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

#include "roma/ipc/src/work_container.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "core/test/utils/conditional_wait.h"
#include "public/core/test/interface/execution_result_matchers.h"
#include "roma/common/src/process.h"
#include "roma/common/src/shared_memory.h"
#include "roma/common/src/shared_memory_pool.h"

using std::lock_guard;
using std::make_unique;
using std::move;
using std::mutex;
using std::set;
using std::string;
using std::thread;
using std::to_string;
using std::unique_ptr;
using std::vector;
using ::testing::HasSubstr;

namespace google::scp::roma::ipc::test {
using common::Process;
using common::RomaString;
using common::SharedMemoryPool;
using common::SharedMemorySegment;
using common::ShmAllocated;
using core::ExecutionResult;
using core::SuccessExecutionResult;
using core::test::WaitUntil;
using ipc::Request;
using ipc::Response;
using ipc::ResponseStatus;
using ipc::RomaCodeObj;
using ipc::WorkContainer;
using ipc::WorkItem;

/**
 * @brief The use case is that the dispatcher process puts work items in the
 * container, and the dispatcher process also polls the container for completed
 * items in a separate thread. Conversely, the worker process will pick up items
 * from the container and mark them as completed once done.
 */
TEST(WorkContainerTest, BasicE2E) {
  SharedMemorySegment segment;
  segment.Create(5 * 10240);
  auto* pool = new (segment.Get()) SharedMemoryPool();
  pool->Init(reinterpret_cast<uint8_t*>(segment.Get()) + sizeof(*pool),
             segment.Size() - sizeof(*pool));
  pool->SetThisThreadMemPool();

  auto* container = new WorkContainer(*pool);
  constexpr int total_items = 10;

  auto worker_process = [&container]() {
    set<string> request_ids;

    for (int i = 0; i < total_items; i++) {
      Request* request;
      auto result = container->GetRequest(request);
      EXPECT_SUCCESS(result);
      EXPECT_THAT(request->code_obj->id, HasSubstr("REQ_ID"));
      request_ids.insert(string(request->code_obj->id.c_str()));

      auto response = make_unique<Response>();
      response->status = ResponseStatus::kSucceeded;

      result = container->CompleteRequest(move(response));
      EXPECT_SUCCESS(result);
    }

    for (int i = 0; i < total_items; i++) {
      EXPECT_TRUE(request_ids.find("REQ_ID" + to_string(i)) !=
                  request_ids.end());
    }

    return SuccessExecutionResult();
  };

  pid_t worker_process_pid;
  auto result = Process::Create(worker_process, worker_process_pid);
  EXPECT_GT(worker_process_pid, 0);
  EXPECT_SUCCESS(result);

  for (int i = 0; i < total_items; i++) {
    auto work_item = make_unique<WorkItem>();
    work_item->request = make_unique<Request>();
    CodeObject code_obj = {.id = "REQ_ID" + to_string(i)};
    work_item->request->code_obj = make_unique<RomaCodeObj>(code_obj);
    EXPECT_SUCCESS(container->TryAcquireAdd());
    auto result = container->Add(move(work_item));
    EXPECT_SUCCESS(result);
  }

  std::atomic<bool> completed_work_thread_done(false);
  auto get_completed_work_thread =
      thread([&container, &completed_work_thread_done, &pool]() {
        pool->SetThisThreadMemPool();

        for (int i = 0; i < total_items; i++) {
          unique_ptr<WorkItem> completed;
          auto result = container->GetCompleted(completed);
          EXPECT_SUCCESS(result);
          EXPECT_TRUE(completed->Succeeded());
        }

        completed_work_thread_done.store(true);
      });

  int status;
  waitpid(worker_process_pid, &status, 0);
  // If WIFEXITED returns zero, then the process died abnormally
  ASSERT_NE(WIFEXITED(status), 0);
  EXPECT_EQ(WEXITSTATUS(status), 0);

  WaitUntil([&]() { return completed_work_thread_done.load(); });

  EXPECT_EQ(container->Size(), 0);

  if (get_completed_work_thread.joinable()) {
    get_completed_work_thread.join();
  }

  delete container;
}

/**
 * @brief The work container uses a circular buffer, so we want to make sure
 * that the circular nature of the container is working as intended. And also
 * validate that the Add method can be called from multiple threads.
 */
TEST(WorkContainerTest, WrapAroundSeveralTimes) {
  SharedMemorySegment segment;
  segment.Create(5 * 10240);
  auto* pool = new (segment.Get()) SharedMemoryPool();
  pool->Init(reinterpret_cast<uint8_t*>(segment.Get()) + sizeof(*pool),
             segment.Size() - sizeof(*pool));
  pool->SetThisThreadMemPool();

  auto* container = new WorkContainer(*pool, /* capacity */ 10);
  mutex push_mutex;
  vector<thread> threads;
  constexpr int num_threads = 101;
  threads.reserve(num_threads);

  // We could potentially have multiple threads pushing work
  for (int i = 0; i < num_threads; i++) {
    // Add work threads
    threads.push_back(thread([i, &container, &pool, &push_mutex]() {
      pool->SetThisThreadMemPool();

      auto work_item = make_unique<WorkItem>();
      work_item->request = make_unique<Request>();
      CodeObject code_obj = {.id = "REQ_ID" + to_string(i)};
      work_item->request->code_obj = make_unique<RomaCodeObj>(code_obj);

      // We need to spin here since we're waiting for spots on the container
      ExecutionResult result;
      while (!result.Successful()) {
        lock_guard<mutex> lk(push_mutex);
        result = container->TryAcquireAdd();
        if (result.Successful()) {
          container->Add(move(work_item));
        }
      }
    }));
  }

  // In our use case, we have only one work thread
  auto work_process_thread = thread([&container, &pool]() {
    pool->SetThisThreadMemPool();

    for (int i = 0; i < num_threads; i++) {
      Request* request;
      auto result = container->GetRequest(request);
      EXPECT_SUCCESS(result);

      auto response = make_unique<Response>();
      response->status = ResponseStatus::kSucceeded;

      result = container->CompleteRequest(move(response));
      EXPECT_SUCCESS(result);
    }
  });

  // In our use case, we have only one thread getting completed work
  auto get_completed_work_thread = thread([&container, &pool]() {
    pool->SetThisThreadMemPool();

    set<string> request_ids;

    for (int i = 0; i < num_threads; i++) {
      unique_ptr<WorkItem> completed;
      auto result = container->GetCompleted(completed);
      request_ids.insert(string(completed->request->code_obj->id.c_str()));
      EXPECT_SUCCESS(result);
      EXPECT_TRUE(completed->Succeeded());
    }

    for (int i = 0; i < num_threads; i++) {
      EXPECT_TRUE(request_ids.find("REQ_ID" + to_string(i)) !=
                  request_ids.end());
    }
  });

  if (work_process_thread.joinable()) {
    work_process_thread.join();
  }

  if (get_completed_work_thread.joinable()) {
    get_completed_work_thread.join();
  }

  for (auto& t : threads) {
    if (t.joinable()) {
      t.join();
    }
  }

  EXPECT_EQ(container->Size(), 0);
  delete container;
}

TEST(WorkContainerTest, QueueFunctionality) {
  SharedMemorySegment segment;
  segment.Create(10240);
  auto* pool = new (segment.Get()) SharedMemoryPool();
  pool->Init(reinterpret_cast<uint8_t*>(segment.Get()) + sizeof(*pool),
             segment.Size() - sizeof(*pool));
  pool->SetThisThreadMemPool();

  auto* container = new WorkContainer(*pool, /* capacity */ 10);

  // Insert requests
  for (int i = 0; i < 10; i++) {
    auto work_item = make_unique<WorkItem>();
    work_item->request = make_unique<Request>();
    CodeObject code_obj = {.id = "REQ_ID" + to_string(i)};
    work_item->request->code_obj = make_unique<RomaCodeObj>(code_obj);
    EXPECT_SUCCESS(container->TryAcquireAdd());
    container->Add(move(work_item));
  }

  // Get and process requests
  for (int i = 0; i < 10; i++) {
    Request* request;
    auto result = container->GetRequest(request);
    EXPECT_SUCCESS(result);
    auto request_id = string(request->code_obj->id.c_str());
    // Should be in the order they were inserted
    EXPECT_EQ("REQ_ID" + to_string(i), request_id);

    auto response = make_unique<Response>();
    response->status = ResponseStatus::kSucceeded;

    result = container->CompleteRequest(move(response));
    EXPECT_SUCCESS(result);
  }

  // Get completed requests
  for (int i = 0; i < 10; i++) {
    unique_ptr<WorkItem> completed;
    auto result = container->GetCompleted(completed);
    auto request_id = string(completed->request->code_obj->id.c_str());
    // Should be in the order they were inserted
    EXPECT_EQ("REQ_ID" + to_string(i), request_id);
    EXPECT_SUCCESS(result);
    EXPECT_TRUE(completed->Succeeded());
  }

  EXPECT_EQ(container->Size(), 0);
  delete container;
}

TEST(WorkContainerTest, TryAcquireAddShouldFailWhenTheContainerIsFull) {
  SharedMemorySegment segment;
  segment.Create(10240);
  auto* pool = new (segment.Get()) SharedMemoryPool();
  pool->Init(reinterpret_cast<uint8_t*>(segment.Get()) + sizeof(*pool),
             segment.Size() - sizeof(*pool));
  pool->SetThisThreadMemPool();

  auto* container = new WorkContainer(*pool, /* capacity */ 10);

  // Insert requests
  for (int i = 0; i < 10; i++) {
    auto work_item = make_unique<WorkItem>();
    work_item->request = make_unique<Request>();
    CodeObject code_obj = {.id = "REQ_ID" + to_string(i)};
    work_item->request->code_obj = make_unique<RomaCodeObj>(code_obj);
    EXPECT_SUCCESS(container->TryAcquireAdd());
    container->Add(move(work_item));
  }

  // Container is full
  EXPECT_EQ(container->Size(), 10);

  EXPECT_FALSE(container->TryAcquireAdd().Successful());

  delete container;
}

TEST(WorkContainerTest, OverflowRequestsPushedToWorkContianer) {
  SharedMemorySegment segment;
  segment.Create(1024000);
  auto* pool = new (segment.Get()) SharedMemoryPool();
  pool->Init(reinterpret_cast<uint8_t*>(segment.Get()) + sizeof(*pool),
             segment.Size() - sizeof(*pool));
  pool->SetThisThreadMemPool();

  auto* container = new WorkContainer(*pool, /* capacity */ 10);
  int total_request = 1000;

  std::thread handle_request([&container, &pool, total_request]() {
    pool->SetThisThreadMemPool();
    for (int i = 0; i < total_request; ++i) {
      Request* request;
      auto result = container->GetRequest(request);
      EXPECT_SUCCESS(result);

      auto response = make_unique<Response>();
      response->status = ResponseStatus::kSucceeded;

      result = container->CompleteRequest(move(response));
      EXPECT_SUCCESS(result);
    }
  });

  std::thread get_response([&container, &pool, total_request]() {
    pool->SetThisThreadMemPool();
    for (int i = 0; i < total_request; ++i) {
      unique_ptr<WorkItem> completed;
      auto result = container->GetCompleted(completed);
      EXPECT_SUCCESS(result);
      EXPECT_TRUE(completed->Succeeded());
    }
  });

  // Insert requests
  std::cout << pool->GetAllocatedSize() << std::endl;
  for (int i = 0; i < total_request; i++) {
    while (!container->TryAcquireAdd().Successful()) {}

    auto work_item = make_unique<WorkItem>();
    work_item->request = make_unique<Request>();
    CodeObject code_obj = {.id = "REQ_ID" + to_string(i)};
    work_item->request->code_obj = make_unique<RomaCodeObj>(code_obj);

    container->Add(move(work_item));

    if (i % 10 == 0) {
      std::cout << "request " << i
                << " allocated size: " << pool->GetAllocatedSize() << std::endl;
    }
  }

  std::cout << pool->GetAllocatedSize() << std::endl;

  handle_request.join();
  get_response.join();
  // Container is empty
  EXPECT_EQ(container->Size(), 0);

  delete container;
}

TEST(WorkContainerTest, SimulateWorkerContainerWork) {
  SharedMemorySegment segment;
  segment.Create(1024000);
  auto* pool = new (segment.Get()) SharedMemoryPool();
  pool->Init(reinterpret_cast<uint8_t*>(segment.Get()) + sizeof(*pool),
             segment.Size() - sizeof(*pool));
  pool->SetThisThreadMemPool();

  auto* container = new WorkContainer(*pool, /* capacity */ 1);
  int total_request = 100;

  auto handle_request = [&container, &pool, total_request]() {
    pool->SetThisThreadMemPool();
    for (int i = 0; i < total_request; ++i) {
      Request* request;
      auto result = container->GetRequest(request);
      EXPECT_SUCCESS(result);
      auto response = make_unique<Response>();
      response->status = ResponseStatus::kSucceeded;

      result = container->CompleteRequest(move(response));
      EXPECT_SUCCESS(result);
    }
    return SuccessExecutionResult();
  };

  pid_t pid1 = -1;
  int child_status1;
  auto result1 = Process::Create(handle_request, pid1);

  std::thread get_response([&container, &pool, total_request]() {
    pool->SetThisThreadMemPool();

    for (int i = 0; i < total_request; ++i) {
      unique_ptr<WorkItem> completed;
      auto result = container->GetCompleted(completed);
      EXPECT_SUCCESS(result);
      EXPECT_TRUE(completed->Succeeded());
    }
  });

  for (int i = 0; i < total_request; i++) {
    while (!container->TryAcquireAdd().Successful()) {}

    auto work_item = make_unique<WorkItem>();
    work_item->request = make_unique<Request>();
    CodeObject code_obj = {.id = to_string(i)};
    work_item->request->code_obj = make_unique<RomaCodeObj>(code_obj);

    container->Add(move(work_item));

    if (i % 10 == 0) {
      std::cout << "request " << i
                << " allocated size: " << pool->GetAllocatedSize() << std::endl;
    }
  }

  waitpid(pid1, &child_status1, 0);
  EXPECT_EQ(WEXITSTATUS(child_status1), 0);

  get_response.join();
  // Container is empty
  EXPECT_EQ(container->Size(), 0);

  delete container;
}
}  // namespace google::scp::roma::ipc::test
