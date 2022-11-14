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

#include "core/interface/service_interface.h"
#include "cpio/client_providers/interface/instance_client_provider_interface.h"
#include "public/core/interface/execution_result.h"
#include "public/cpio/local/local_cpio_options.h"

namespace google::scp::cpio::client_providers {
/**
 * @copydoc InstanceClientProviderInterface.
 */
class LocalInstanceClientProvider : public InstanceClientProviderInterface {
 public:
  LocalInstanceClientProvider(
      const std::shared_ptr<LocalCpioOptions>& local_cpio_options)
      : local_cpio_options_(local_cpio_options) {}

  core::ExecutionResult Init() noexcept override;

  core::ExecutionResult Run() noexcept override;

  core::ExecutionResult Stop() noexcept override;

  core::ExecutionResult GetEnvironmentName(
      std::string& env_name, const std::string& env_tag,
      const std::string& instance_id) noexcept override;

  core::ExecutionResult GetInstanceId(
      std::string& instance_id) noexcept override;

  core::ExecutionResult GetRegion(std::string& region) noexcept override;

  core::ExecutionResult GetInstancePublicIpv4Address(
      std::string& instance_public_ipv4_address) noexcept override;

  core::ExecutionResult GetInstancePrivateIpv4Address(
      std::string& instance_private_ipv4_address) noexcept override;

 private:
  std::shared_ptr<LocalCpioOptions> local_cpio_options_;
};
}  // namespace google::scp::cpio::client_providers
