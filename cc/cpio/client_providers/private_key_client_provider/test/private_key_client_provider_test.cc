
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

#include "cpio/client_providers/private_key_client_provider/src/private_key_client_provider.h"

#include <gtest/gtest.h>

#include <functional>
#include <memory>
#include <string>

#include "core/interface/async_context.h"
#include "core/message_router/src/message_router.h"
#include "core/test/utils/conditional_wait.h"
#include "cpio/proto/private_key_client.pb.h"
#include "public/core/interface/execution_result.h"

using google::protobuf::Any;
using google::scp::core::AsyncContext;
using google::scp::core::MessageRouter;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::test::WaitUntil;
using google::scp::cpio::private_key_client::ListPrivateKeysByIdsProtoRequest;
using google::scp::cpio::private_key_client::ListPrivateKeysByIdsProtoResponse;
using std::atomic;
using std::make_shared;
using std::make_unique;
using std::move;
using std::shared_ptr;
using std::string;
using std::unique_ptr;

namespace google::scp::cpio::client_providers::test {
class PrivateKeyClientProviderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    message_router = make_shared<MessageRouter>();
    auto private_key_client_options = make_shared<PrivateKeyClientOptions>();

    PrivateKeyVendingEndpoint primary_private_key_vending_endpoint;
    primary_private_key_vending_endpoint.kms_key_name = "TestKey";
    primary_private_key_vending_endpoint.kms_region = "TestRegion";
    primary_private_key_vending_endpoint.private_key_vending_service_endpoint =
        "TestEndpoint";
    PrivateKeyVendingEndpoint secondary_private_key_vending_endpoint;
    secondary_private_key_vending_endpoint.kms_key_name = "TestKey2";
    secondary_private_key_vending_endpoint.kms_region = "TestRegion2";
    secondary_private_key_vending_endpoint
        .private_key_vending_service_endpoint = "TestEndpoint2";

    private_key_client_options->account_identity = "Test";
    private_key_client_options->primary_private_key_vending_endpoint =
        primary_private_key_vending_endpoint;
    private_key_client_options->secondary_private_key_vending_endpoints
        .emplace_back(secondary_private_key_vending_endpoint);

    private_key_client_provider = make_unique<PrivateKeyClientProvider>(
        private_key_client_options, message_router);

    EXPECT_EQ(private_key_client_provider->Init(), SuccessExecutionResult());
  }

  void TearDown() override {
    if (private_key_client_provider) {
      EXPECT_EQ(private_key_client_provider->Stop(), SuccessExecutionResult());
    }
  }

  shared_ptr<MessageRouter> message_router;
  unique_ptr<PrivateKeyClientProvider> private_key_client_provider;
};

TEST_F(PrivateKeyClientProviderTest, Run) {
  EXPECT_EQ(private_key_client_provider->Run(), SuccessExecutionResult());

  ListPrivateKeysByIdsProtoRequest list_private_keys_by_ids_request;
  auto any_request = make_shared<Any>();
  any_request->PackFrom(list_private_keys_by_ids_request);
  atomic<bool> condition = false;
  auto any_context = make_shared<AsyncContext<Any, Any>>(
      move(any_request), [&](AsyncContext<Any, Any>& any_context) {
        EXPECT_EQ(any_context.result, SuccessExecutionResult());
        condition = true;
      });

  message_router->OnMessageReceived(any_context);
  WaitUntil([&]() { return condition.load(); });
}

TEST_F(PrivateKeyClientProviderTest, ListPrivateKeysByIds) {
  auto request = make_shared<ListPrivateKeysByIdsProtoRequest>();
  request->add_key_ids("key_id");

  atomic<bool> condition = false;

  AsyncContext<ListPrivateKeysByIdsProtoRequest,
               ListPrivateKeysByIdsProtoResponse>
      context(request,
              [&](AsyncContext<ListPrivateKeysByIdsProtoRequest,
                               ListPrivateKeysByIdsProtoResponse>& context) {
                EXPECT_EQ(context.result, SuccessExecutionResult());
                condition = true;
                return SuccessExecutionResult();
              });

  EXPECT_EQ(private_key_client_provider->ListPrivateKeysByIds(context),
            SuccessExecutionResult());
  WaitUntil([&]() { return condition.load(); });
}

}  // namespace google::scp::cpio::client_providers::test
