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

#include "core/interface/async_context.h"
#include "core/interface/service_interface.h"
#include "public/core/interface/execution_result.h"

namespace google::scp::cpio::client_providers {
/**
 * @brief Interface responsible for fetching instance data.
 */
class InstanceClientProviderInterface : public core::ServiceInterface {
 public:
  /**
   * @brief Fetches the environment name for the given environment tag and
   * instance ID.
   *
   * @param env_name returned environment name.
   * @param env_tag the given environment tag.
   * @param instance_id the given instance ID.
   * @return core::ExecutionResult execution result.
   */
  virtual core::ExecutionResult GetEnvironmentName(
      std::string& env_name, const std::string& env_tag,
      const std::string& instance_id) noexcept = 0;

  /**
   * @brief Gets the Instance Id.
   *
   * @param instance_id returned instance ID.
   * @return core::ExecutionResult execution result.
   */
  virtual core::ExecutionResult GetInstanceId(
      std::string& instance_id) noexcept = 0;

  /**
   * @brief Gets the Region.
   *
   * @param region returned region.
   * @return core::ExecutionResult execution result.
   */
  virtual core::ExecutionResult GetRegion(std::string& region) noexcept = 0;

  /**
   * @brief Gets the public IP address for the nstance.
   *
   * @param instance_id returned public IP address.
   * @return core::ExecutionResult execution result.
   */
  virtual core::ExecutionResult GetInstancePublicIpv4Address(
      std::string& instance_public_ipv4_address) noexcept = 0;

  /**
   * @brief Gets the private IP address for the nstance.
   *
   * @param instance_id returned private IP address.
   * @return core::ExecutionResult execution result.
   */
  virtual core::ExecutionResult GetInstancePrivateIpv4Address(
      std::string& instance_private_ipv4_address) noexcept = 0;
};

class InstanceClientProviderFactory {
 public:
  /**
   * @brief Factory to create InstanceClientProvider.
   *
   * @return std::shared_ptr<InstanceClientProviderInterface> created
   * InstanceClientProvider.
   */
  static std::shared_ptr<InstanceClientProviderInterface> Create();
};
}  // namespace google::scp::cpio::client_providers
