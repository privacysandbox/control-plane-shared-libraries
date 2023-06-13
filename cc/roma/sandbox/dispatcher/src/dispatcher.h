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

#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "core/async_executor/src/async_executor.h"
#include "core/interface/service_interface.h"
#include "public/core/interface/execution_result.h"
#include "roma/interface/roma.h"
#include "roma/sandbox/worker_api/src/worker_api.h"
#include "roma/sandbox/worker_pool/src/worker_pool.h"

#include "error_codes.h"
#include "request_converter.h"
#include "request_validator.h"

namespace google::scp::roma::sandbox::dispatcher {
class Dispatcher : public core::ServiceInterface {
 public:
  Dispatcher(std::shared_ptr<core::AsyncExecutor>& async_executor,
             std::shared_ptr<worker_pool::WorkerPool>& worker_pool,
             size_t max_pending_requests)
      : async_executor_(async_executor),
        worker_pool_(worker_pool),
        worker_index_(0),
        pending_requests_(0),
        max_pending_requests_(max_pending_requests),
        allow_dispatch_(true) {}

  core::ExecutionResult Init() noexcept override;

  core::ExecutionResult Run() noexcept override;

  core::ExecutionResult Stop() noexcept override;

  /**
   * @brief Enqueues a request to be handled by the workers.
   *
   * @tparam RequestT The type of the request.
   * @param request The request.
   * @param callback The function to call once the request completes.
   * @return core::ExecutionResult Whether the enqueue operation succeeded or
   * not.
   */
  template <typename RequestT>
  core::ExecutionResult Dispatch(std::unique_ptr<RequestT> request,
                                 Callback callback) noexcept {
    if (!allow_dispatch_.load()) {
      return core::FailureExecutionResult(
          core::errors::
              SC_ROMA_DISPATCHER_DISPATCH_DISALLOWED_DUE_TO_ONGOING_LOAD);
    }
    return InternalDispatch(std::move(request), callback);
  }

  /**
   * @brief Dispatch a set of requests. This funciton will block until all the
   * requests have been dispatched. This uses Dispatch.
   *
   * @tparam RequestT The type of the request.
   * @param batch The input bacth of request to enqueue.
   * @param batch_callback The callback to invoke once the batch is done.
   * @return core::ExecutionResult Whether the dispatch batch operation
   * succeeded or failed.
   */
  template <typename RequestT>
  core::ExecutionResult DispatchBatch(std::vector<RequestT>& batch,
                                      BatchCallback batch_callback) noexcept {
    if (!allow_dispatch_.load()) {
      return core::FailureExecutionResult(
          core::errors::
              SC_ROMA_DISPATCHER_DISPATCH_DISALLOWED_DUE_TO_ONGOING_LOAD);
    }

    auto batch_size = batch.size();
    auto batch_response =
        std::make_shared<std::vector<absl::StatusOr<ResponseObject>>>(
            batch_size, absl::StatusOr<ResponseObject>());
    auto finished_counter = std::make_shared<std::atomic<size_t>>(0);

    for (size_t index = 0; index < batch_size; ++index) {
      auto callback =
          [batch_response, finished_counter, batch_callback, index](
              std::unique_ptr<absl::StatusOr<ResponseObject>> obj_response) {
            batch_response->at(index) = *obj_response;
            auto finished_value = finished_counter->fetch_add(1);
            if (finished_value + 1 == batch_response->size()) {
              batch_callback(*batch_response);
            }
          };

      auto request = std::make_unique<RequestT>(batch[index]);
      while (!Dispatcher::Dispatch(std::move(request), callback).Successful()) {
        request = std::make_unique<RequestT>(batch[index]);
      }
    }

    return core::SuccessExecutionResult();
  }

  /**
   * @brief Execute a "load" request against all worker in the pool.
   *
   * @param code_object The code object to load.
   * @param broadcast_callback The callback to invoke once the opeartion has
   * completed.
   * @return core::ExecutionResult Whether the broadcast succeeded or failed.
   */
  core::ExecutionResult Broadcast(std::unique_ptr<CodeObject> code_object,
                                  Callback broadcast_callback) noexcept;

 private:
  /**
   * @brief The internal dispatch function which puts a request into a worker
   * queue.
   *
   * @tparam RequestT The request type.
   * @param request The request.
   * @param callback The callback to invoke once the operation finishes.
   * @return core::ExecutionResult Whether the dispatch call succeeded or
   * failed.
   */
  template <typename RequestT>
  core::ExecutionResult InternalDispatch(std::unique_ptr<RequestT> request,
                                         Callback callback) noexcept {
    if (pending_requests_.load() >= max_pending_requests_) {
      return core::FailureExecutionResult(
          core::errors::SC_ROMA_DISPATCHER_DISPATCH_DISALLOWED_DUE_TO_CAPACITY);
    }

    auto validation_result =
        request_validator::RequestValidator<RequestT>::Validate(request);
    if (!validation_result.Successful()) {
      return validation_result;
    }

    auto num_workers = worker_pool_->GetPoolSize();
    auto index = worker_index_.fetch_add(1);
    worker_index_ = worker_index_.load() % num_workers;

    // This is a workaround to be able to register a lambda that needs to
    // capture a non-copy-constructible input (request)
    auto shared_request =
        std::make_shared<std::unique_ptr<RequestT>>(std::move(request));

    auto schedule_result = async_executor_->Schedule(
        [this, index, shared_request, callback] {
          auto request = std::move(*shared_request);
          std::unique_ptr<absl::StatusOr<ResponseObject>> response_or;

          auto worker_or = worker_pool_->GetWorker(index);
          if (!worker_or.result().Successful()) {
            response_or = std::make_unique<absl::StatusOr<ResponseObject>>(
                absl::Status(absl::StatusCode::kInternal,
                             core::errors::GetErrorMessage(
                                 worker_or.result().status_code)));
            callback(::std::move(response_or));
            pending_requests_--;
            return;
          }

          auto run_code_request_or =
              request_converter::RequestConverter<RequestT>::FromUserProvided(
                  request);
          if (!run_code_request_or.result().Successful()) {
            response_or = std::make_unique<absl::StatusOr<ResponseObject>>(
                absl::Status(absl::StatusCode::kInternal,
                             core::errors::GetErrorMessage(
                                 run_code_request_or.result().status_code)));
            callback(::std::move(response_or));
            pending_requests_--;
            return;
          }

          auto run_code_response_or =
              (*worker_or)->RunCode(*run_code_request_or);
          if (!run_code_response_or.result().Successful()) {
            response_or = std::make_unique<absl::StatusOr<ResponseObject>>(
                absl::Status(absl::StatusCode::kInternal,
                             core::errors::GetErrorMessage(
                                 run_code_response_or.result().status_code)));
            callback(::std::move(response_or));
            pending_requests_--;
            return;
          }

          ResponseObject response_object;
          response_object.id = request->id;
          response_object.resp = *run_code_response_or->response;
          response_or =
              std::make_unique<absl::StatusOr<ResponseObject>>(response_object);
          callback(::std::move(response_or));
          pending_requests_--;
        },
        core::AsyncPriority::Normal);

    if (schedule_result.Successful()) {
      pending_requests_++;
    }

    return schedule_result;
  }

  std::shared_ptr<core::AsyncExecutor> async_executor_;
  std::shared_ptr<worker_pool::WorkerPool> worker_pool_;
  std::atomic<size_t> worker_index_;
  std::atomic<size_t> pending_requests_;
  const size_t max_pending_requests_;
  std::atomic<bool> allow_dispatch_;
};
}  // namespace google::scp::roma::sandbox::dispatcher
