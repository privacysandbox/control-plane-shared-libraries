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

#include "core/interface/request_route_resolver_interface.h"
#include "core/interface/service_interface.h"

namespace google::scp::core {
/**
 * @brief An interface to implement a request router to an external endpoint.
 * The implementation has to act as a man in the middle for the request,
 * handling both request forwarding, and response flow back to the client.
 *
 * @tparam TRequest
 * @tparam TResponse
 */
template <typename TRequest, typename TResponse>
class RequestRouterInterface : public ServiceInterface {
 public:
  virtual ~RequestRouterInterface() = default;
  /**
   * @brief Send request to remote endpoint provided at 'endpoint_info',
   * populate the response received, and finish the context.
   *
   * @param context
   * @return ExecutionResult
   */
  virtual ExecutionResult RouteRequestToRemoteEndpoint(
      AsyncContext<TRequest, TResponse> context,
      const RequestEndpointInfo& endpoint_info) noexcept = 0;
};
}  // namespace google::scp::core
