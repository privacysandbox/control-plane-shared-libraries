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
#include "http1_curl_client.h"

#include <utility>

#include "http1_curl_wrapper.h"

using google::scp::core::common::RetryStrategy;
using google::scp::core::common::RetryStrategyType;
using std::make_shared;
using std::move;
using std::shared_ptr;

namespace google::scp::core {

Http1CurlClient::Http1CurlClient(
    shared_ptr<AsyncExecutorInterface>& async_executor,
    shared_ptr<Http1CurlWrapperProvider> curl_wrapper_provider,
    RetryStrategyType retry_strategy_type, TimeDuration time_duration_ms,
    size_t total_retries)
    : curl_wrapper_provider_(curl_wrapper_provider),
      async_executor_(async_executor),
      operation_dispatcher_(
          async_executor,
          RetryStrategy(retry_strategy_type, time_duration_ms, total_retries)) {
}

ExecutionResult Http1CurlClient::Init() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult Http1CurlClient::Run() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult Http1CurlClient::Stop() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult Http1CurlClient::PerformRequest(
    AsyncContext<HttpRequest, HttpResponse>& http_context) noexcept {
  auto wrapper_or = curl_wrapper_provider_->MakeWrapper();
  RETURN_IF_FAILURE(wrapper_or.result());
  operation_dispatcher_.Dispatch<AsyncContext<HttpRequest, HttpResponse>>(
      http_context, [this, wrapper = *wrapper_or](auto& http_context) {
        auto response_or = wrapper->PerformRequest(*http_context.request);
        http_context.result = response_or.result();
        RETURN_IF_FAILURE(response_or.result());

        http_context.response = make_shared<HttpResponse>(move(*response_or));

        FinishContext(SuccessExecutionResult(), http_context, async_executor_);

        return SuccessExecutionResult();
      });
  return SuccessExecutionResult();
}

}  // namespace google::scp::core
