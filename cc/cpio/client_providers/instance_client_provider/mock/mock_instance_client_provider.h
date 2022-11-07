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

#include "cpio/client_providers/interface/instance_client_provider_interface.h"

namespace google::scp::cpio::client_providers::mock {
class MockInstanceClientProvider : public InstanceClientProviderInterface {
 public:
  core::ExecutionResult Init() noexcept override {
    return core::SuccessExecutionResult();
  }

  core::ExecutionResult Run() noexcept override {
    return core::SuccessExecutionResult();
  }

  core::ExecutionResult Stop() noexcept override {
    return core::SuccessExecutionResult();
  }

  std::string instance_id_mock;
  core::ExecutionResult get_instance_id_result_mock =
      core::SuccessExecutionResult();

  core::ExecutionResult GetInstanceId(
      std::string& instance_id) noexcept override {
    if (get_instance_id_result_mock != core::SuccessExecutionResult()) {
      return get_instance_id_result_mock;
    }
    instance_id = instance_id_mock;
    return core::SuccessExecutionResult();
  }

  std::string region_mock;
  core::ExecutionResult get_region_result_mock = core::SuccessExecutionResult();

  core::ExecutionResult GetRegion(std::string& region) noexcept override {
    if (get_region_result_mock != core::SuccessExecutionResult()) {
      return get_region_result_mock;
    }
    region = region_mock;
    return core::SuccessExecutionResult();
  }

  std::string environment_name_mock;
  core::ExecutionResult get_environment_name_result_mock =
      core::SuccessExecutionResult();

  core::ExecutionResult GetEnvironmentName(
      std::string& name, const std::string& environment_tag,
      const std::string& instance_id) noexcept override {
    if (get_environment_name_result_mock != core::SuccessExecutionResult()) {
      return get_environment_name_result_mock;
    }
    name = environment_name_mock;
    return core::SuccessExecutionResult();
  }

  core::ExecutionResult GetInstancePublicIpv4Address(
      std::string& instance_public_ipv4_address) noexcept override {
    return core::SuccessExecutionResult();
  }

  core::ExecutionResult GetInstancePrivateIpv4Address(
      std::string& instance_private_ipv4_address) noexcept override {
    return core::SuccessExecutionResult();
  }
};
}  // namespace google::scp::cpio::client_providers::mock
