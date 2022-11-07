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

#include <memory>
#include <string>

#include <aws/core/Aws.h>
#include <aws/core/client/ClientConfiguration.h>

#include "cpio/client_providers/config_client_provider/aws/src/aws_config_client_provider.h"
#include "public/cpio/interface/config_client/type_def.h"
#include "public/cpio/test/test_aws_config_client_options.h"

namespace google::scp::cpio::client_providers {
/*! @copydoc AwsConfigClientInterface
 */
class TestAwsConfigClientProvider : public AwsConfigClientProvider {
 public:
  TestAwsConfigClientProvider(
      const std::shared_ptr<TestAwsConfigClientOptions>& config_client_options,
      const std::shared_ptr<InstanceClientProviderInterface>&
          instance_client_provider,
      const std::shared_ptr<core::MessageRouterInterface<
          google::protobuf::Any, google::protobuf::Any>>& message_router)
      : AwsConfigClientProvider(config_client_options, instance_client_provider,
                                message_router),
        ssm_endpoint_override_(config_client_options->ssm_endpoint_override) {}

 protected:
  core::ExecutionResult CreateClientConfiguration(
      std::shared_ptr<Aws::Client::ClientConfiguration>& client_config) noexcept
      override;

  std::shared_ptr<std::string> ssm_endpoint_override_;
};
}  // namespace google::scp::cpio::client_providers
