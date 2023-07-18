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

#include "aws_private_key_fetcher_provider.h"

#include <utility>
#include <vector>

#include <boost/system/error_code.hpp>
#include <nghttp2/asio_http2.h>

#include "core/interface/http_client_interface.h"
#include "cpio/client_providers/interface/auth_token_provider_interface.h"
#include "cpio/client_providers/interface/role_credentials_provider_interface.h"
#include "cpio/client_providers/private_key_fetcher_provider/src/private_key_fetcher_provider_utils.h"

#include "error_codes.h"

using boost::system::error_code;
using google::scp::core::AsyncContext;
using google::scp::core::AwsV4Signer;
using google::scp::core::ExecutionResult;
using google::scp::core::FailureExecutionResult;
using google::scp::core::HttpClientInterface;
using google::scp::core::HttpHeaders;
using google::scp::core::HttpMethod;
using google::scp::core::HttpRequest;
using google::scp::core::HttpResponse;
using google::scp::core::SuccessExecutionResult;
using google::scp::core::Uri;
using google::scp::core::common::kZeroUuid;
using google::scp::core::errors::
    SC_AWS_PRIVATE_KEY_FETCHER_PROVIDER_CREDENTIALS_PROVIDER_NOT_FOUND;
using google::scp::core::errors::
    SC_AWS_PRIVATE_KEY_FETCHER_PROVIDER_INVALID_URI;
using google::scp::core::errors::
    SC_AWS_PRIVATE_KEY_FETCHER_PROVIDER_REGION_NOT_FOUND;
using nghttp2::asio_http2::host_service_from_uri;
using std::bind;
using std::make_shared;
using std::move;
using std::shared_ptr;
using std::string;
using std::vector;
using std::placeholders::_1;

static constexpr char kAwsPrivateKeyFetcherProvider[] =
    "AwsPrivateKeyFetcherProvider";
/// Generic AWS service name.
static constexpr char kServiceName[] = "execute-api";

namespace google::scp::cpio::client_providers {

ExecutionResult AwsPrivateKeyFetcherProvider::Init() noexcept {
  RETURN_IF_FAILURE(PrivateKeyFetcherProvider::Init());

  if (!role_credentials_provider_) {
    auto execution_result = FailureExecutionResult(
        SC_AWS_PRIVATE_KEY_FETCHER_PROVIDER_CREDENTIALS_PROVIDER_NOT_FOUND);
    SCP_ERROR(kAwsPrivateKeyFetcherProvider, kZeroUuid, execution_result,
              "Failed to get credentials provider.");
    return execution_result;
  }

  return SuccessExecutionResult();
}

ExecutionResult AwsPrivateKeyFetcherProvider::SignHttpRequest(
    AsyncContext<PrivateKeyFetchingRequest, HttpRequest>&
        sign_request_context) noexcept {
  auto request = make_shared<GetRoleCredentialsRequest>();
  request->account_identity = make_shared<string>(
      sign_request_context.request->key_vending_endpoint->account_identity);
  AsyncContext<GetRoleCredentialsRequest, GetRoleCredentialsResponse>
      get_session_credentials_context(
          move(request),
          bind(&AwsPrivateKeyFetcherProvider::
                   CreateSessionCredentialsCallbackToSignHttpRequest,
               this, sign_request_context, _1),
          sign_request_context);
  return role_credentials_provider_->GetRoleCredentials(
      get_session_credentials_context);
}

void AwsPrivateKeyFetcherProvider::
    CreateSessionCredentialsCallbackToSignHttpRequest(
        AsyncContext<PrivateKeyFetchingRequest, HttpRequest>&
            sign_request_context,
        AsyncContext<GetRoleCredentialsRequest, GetRoleCredentialsResponse>&
            get_session_credentials_context) noexcept {
  auto execution_result = get_session_credentials_context.result;
  if (!execution_result.Successful()) {
    SCP_ERROR_CONTEXT(kAwsPrivateKeyFetcherProvider, sign_request_context,
                      execution_result, "Failed to get AWS credentials.");
    sign_request_context.result = get_session_credentials_context.result;
    sign_request_context.Finish();
    return;
  }

  auto http_request = make_shared<HttpRequest>();
  PrivateKeyFetchingClientUtils::CreateHttpRequest(
      *sign_request_context.request, *http_request);

  execution_result = SignHttpRequestUsingV4Signer(
      http_request, *get_session_credentials_context.response->access_key_id,
      *get_session_credentials_context.response->access_key_secret,
      *get_session_credentials_context.response->security_token,
      sign_request_context.request->key_vending_endpoint->service_region);

  if (execution_result.Successful()) {
    sign_request_context.response = http_request;
  }
  sign_request_context.result = execution_result;
  sign_request_context.Finish();
}

ExecutionResult AwsPrivateKeyFetcherProvider::SignHttpRequestUsingV4Signer(
    shared_ptr<HttpRequest>& http_request, const string& access_key,
    const string& secret_key, const string& security_token,
    const string& region) noexcept {
  http_request->headers = make_shared<HttpHeaders>();
  error_code http2_error_code;
  string scheme;
  string host;
  string service;
  if (host_service_from_uri(http2_error_code, scheme, host, service,
                            *http_request->path)) {
    auto execution_result =
        FailureExecutionResult(SC_AWS_PRIVATE_KEY_FETCHER_PROVIDER_INVALID_URI);
    SCP_ERROR(kAwsPrivateKeyFetcherProvider, kZeroUuid, execution_result,
              "Failed to sign HTTP request for an invalid URI.");
    return execution_result;
  }
  http_request->headers->insert({string("Host"), host});
  AwsV4Signer signer(access_key, secret_key, security_token,
                     string(kServiceName), region);
  vector<string> headers_to_sign{"Host", "X-Amz-Date"};
  return signer.SignRequest(*http_request, headers_to_sign);
}

#ifndef TEST_CPIO
std::shared_ptr<PrivateKeyFetcherProviderInterface>
PrivateKeyFetcherProviderFactory::Create(
    const shared_ptr<HttpClientInterface>& http_client,
    const shared_ptr<RoleCredentialsProviderInterface>&
        role_credentials_provider,
    const shared_ptr<AuthTokenProviderInterface>& auth_token_provider) {
  return make_shared<AwsPrivateKeyFetcherProvider>(http_client,
                                                   role_credentials_provider);
}
#endif
}  // namespace google::scp::cpio::client_providers
