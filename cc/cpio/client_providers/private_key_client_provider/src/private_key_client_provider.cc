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

#include "private_key_client_provider.h"

#include <functional>
#include <memory>
#include <utility>

#include "core/interface/async_context.h"
#include "core/interface/message_router_interface.h"
#include "cpio/client_providers/interface/private_key_client_provider_interface.h"
#include "cpio/client_providers/interface/type_def.h"
#include "cpio/proto/private_key_client.pb.h"
#include "google/protobuf/any.pb.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/interface/private_key_client/type_def.h"

using google::protobuf::Any;
using google::scp::core::AsyncContext;
using google::scp::core::ExecutionResult;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::common::kZeroUuid;
using google::scp::cpio::private_key_client::ListPrivateKeysByIdsProtoRequest;
using google::scp::cpio::private_key_client::ListPrivateKeysByIdsProtoResponse;
using std::bind;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::placeholders::_1;

namespace google::scp::cpio::client_providers {
ExecutionResult PrivateKeyClientProvider::Init() noexcept {
  if (message_router_) {
    ListPrivateKeysByIdsProtoRequest list_private_keys_by_ids_proto_request;
    Any any_request;
    any_request.PackFrom(list_private_keys_by_ids_proto_request);
    return message_router_->Subscribe(
        any_request.type_url(),
        bind(&PrivateKeyClientProvider::OnListPrivateKeysByIds, this, _1));
  }
  return SuccessExecutionResult();
}

ExecutionResult PrivateKeyClientProvider::Run() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult PrivateKeyClientProvider::Stop() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult PrivateKeyClientProvider::ListPrivateKeysByIds(
    AsyncContext<ListPrivateKeysByIdsProtoRequest,
                 ListPrivateKeysByIdsProtoResponse>& context) noexcept {
  auto response = make_shared<ListPrivateKeysByIdsProtoResponse>();
  auto result = SuccessExecutionResult();

  // TODO: Implementation

  context.response = move(response);
  context.result = result;
  context.Finish();

  return SuccessExecutionResult();
}

void PrivateKeyClientProvider::OnListPrivateKeysByIds(
    AsyncContext<Any, Any> any_context) noexcept {
  auto request = make_shared<ListPrivateKeysByIdsProtoRequest>();
  any_context.request->UnpackTo(request.get());
  AsyncContext<ListPrivateKeysByIdsProtoRequest,
               ListPrivateKeysByIdsProtoResponse>
      context(move(request),
              bind(CallbackToPackAnyResponse<ListPrivateKeysByIdsProtoRequest,
                                             ListPrivateKeysByIdsProtoResponse>,
                   any_context, _1));
  context.result = ListPrivateKeysByIds(context);
}

}  // namespace google::scp::cpio::client_providers
