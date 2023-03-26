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

#pragma once

#include <chrono>
#include <functional>
#include <memory>

#include "core/common/concurrent_queue/src/concurrent_queue.h"

#include "async_context.h"

namespace google::scp::core {

/**
 * @brief Base class for holding cancellation mechanics for streaming contexts.
 */
template <typename TRequest, typename TResponse>
struct StreamingContext : public AsyncContext<TRequest, TResponse> {
 private:
  using BaseClass = AsyncContext<TRequest, TResponse>;

 public:
  using BaseClass::BaseClass;

  StreamingContext(const StreamingContext& right) : BaseClass(right) {
    is_cancelled = right.is_cancelled;
  }

  /**
   * @brief Attempts to cancel the current operation. This is a best effort
   * cancellation. If cancellation succeeds, it is expected that the callee will
   * still end up calling Finish on this context, although this is not
   * guaranteed.
   *
   */
  virtual void TryCancel() noexcept = 0;

  bool IsCancelled() const noexcept { return is_cancelled->load(); }

 protected:
  std::shared_ptr<std::atomic_bool> is_cancelled =
      std::make_shared<std::atomic_bool>(false);
};

/**
 * @brief ServerStreamingContext is used to control the lifecycle of any
 * streaming operations. The caller will set the request, response, the
 * process_callback, and the response_queue and components will use it to
 * transition from one async state another. process_callback is now called once
 * for each new message placed in the queue and once more when the call has
 * completed. If response_queue IsDone, then it is the final call.
 * AsyncContext::callback is now unused.
 *
 * General form of process_callback:
 *
 * context.process_callback = [](auto& context) {
 *   // It is important that TryDequeue is called before checking IsDone
 *   // otherwise 2 threads may compete to get the last element and enter a bad
 *   // state.
 *   Element e;
 *   if (context.response_queue->TryDequeue(e).Successful()) {
 *     // Handle this Element.                                             // #1
 *   } else {
 *     if (!context.response_queue->IsDone()) {
 *       // Generally, this should be impossible.
 *     }
 *     if (!context.result.Successful()) {     // #2
 *       // Handle unsuccess.
 *       return;
 *     }
 *     // Handle success.
 *   }
 * };
 *
 * NOTE: There is an edge case where one thread is at #1 and another is at
 * thread #2 (the final element has been dequeued and Finish() has been
 * called.). In this scenario, the thread at #2 *might* assume that all elements
 * in the queue have been dequeued *and* completed processing but the reality
 * may be that processing has not yet completed. If this would result in thread
 * #2 continuing incorrectly, some additional condition should be checked before
 * thread #2 proceeds.
 *
 * @tparam TRequest request template param
 * @tparam TResponse response template param. This is the type of request being
 * used in response_queue.
 */
template <typename TRequest, typename TResponse>
struct ServerStreamingContext : public StreamingContext<TRequest, TResponse> {
 private:
  using BaseClass = StreamingContext<TRequest, TResponse>;

 public:
  using BaseClass::BaseClass;

  ServerStreamingContext(const ServerStreamingContext& right)
      : BaseClass(right) {
    response_queue = right.response_queue;
    process_callback = right.process_callback;
  }

  void TryCancel() noexcept override {
    this->is_cancelled->store(true);
    response_queue->MarkDone();
  }

  using ProcessCallback = typename std::function<void(
      ServerStreamingContext<TRequest, TResponse>&, bool)>;

  // Is called each time a new message is placed in response_queue AND when the
  // async operation is completed. The first argument is *this. The second
  // argument is whether this->result contains the true result of the operation.
  // This is for users to differentiate between ProcessNextMessage calls which
  // do not have meaningful values in this->result, and Finish calls which do.
  ProcessCallback process_callback;

  /// ConcurrentQueue used by the callee to communicate messages back to the
  /// caller.
  std::shared_ptr<common::ConcurrentQueue<TResponse>> response_queue;

  /// Processes the next message in the queue.
  void ProcessNextMessage() noexcept { this->process_callback(*this, false); }

  /// Finishes the async operation by calling the callback.
  void Finish() noexcept override {
    if (process_callback) {
      if (!this->result.Successful()) {
        // typeid(TRequest).name() is an approximation of the context's template
        // types mangled in compiler defined format, mainly for debugging
        // purposes.
        ERROR_CONTEXT("AsyncContext", (*this), this->result,
                      "AsyncContext Finished. Mangled RequestType: '%s', "
                      "Mangled ResponseType: '%s'",
                      typeid(TRequest).name(), typeid(TResponse).name());
      }
      process_callback(*this, true);
    }
  }
};

/**
 * @brief ClientStreamingContext is used to control the lifecycle of any
 * streaming operations. The caller will set the request, response, the
 * callbacks, and the response_queue and components will use it to transition
 * from one async state another. AsyncContext's request field should contain the
 * initial request, all subsequent requests (not including the initial) should
 * be communicated via request_queue.
 *
 * @tparam TRequest request template param. This is the type of request being
 * used in response_queue.
 * @tparam TResponse response template param
 */
template <typename TRequest, typename TResponse>
struct ClientStreamingContext : public StreamingContext<TRequest, TResponse> {
 private:
  using BaseClass = StreamingContext<TRequest, TResponse>;

 public:
  using BaseClass::BaseClass;

  ClientStreamingContext(const ClientStreamingContext& right)
      : BaseClass(right) {
    request_queue = right.request_queue;
  }

  void TryCancel() noexcept override {
    this->is_cancelled->store(true);
    request_queue->MarkDone();
  }

  /// MessageQueue used by the caller to communicate messages to the
  /// callee.
  std::shared_ptr<common::ConcurrentQueue<TRequest>> request_queue;
};

/**
 * @brief Finish Context on a thread on the provided AsyncExecutor thread pool.
 * Assigns the result to the context, schedules Finish(), and
 * returns the result. If the context cannot be finished async, it will be
 * finished synchronously on the current thread. Before finishing the context,
 * we mark the response_queue or request_queue as done if present.
 * @param result execution result of operation.
 * @param context the async context to be completed.
 * @param async_executor the executor (thread pool) for the async context to
 * be completed on.
 * @param priority the priority for the executor. Defaults to High.
 */
template <template <typename...> typename TContext, typename TRequest,
          typename TResponse>
void FinishStreamingContext(
    const ExecutionResult& result, TContext<TRequest, TResponse>& context,
    const std::shared_ptr<AsyncExecutorInterface>& async_executor,
    AsyncPriority priority = AsyncPriority::High) {
  constexpr bool is_server_streaming =
      std::is_base_of_v<ServerStreamingContext<TRequest, TResponse>,
                        TContext<TRequest, TResponse>>;
  constexpr bool is_client_streaming =
      std::is_base_of_v<ClientStreamingContext<TRequest, TResponse>,
                        TContext<TRequest, TResponse>>;
  static_assert(is_server_streaming || is_client_streaming);

  context.result = result;
  if constexpr (is_server_streaming) {
    if (context.response_queue) {
      context.response_queue->MarkDone();
    }
  }
  if constexpr (is_client_streaming) {
    if (context.request_queue) {
      context.request_queue->MarkDone();
    }
  }

  // Make a copy of context - this way we know async_executor's handle will
  // never go out of scope.
  if (!async_executor
           ->Schedule([context]() mutable { context.Finish(); }, priority)
           .Successful()) {
    context.Finish();
  }
}

}  // namespace google::scp::core
