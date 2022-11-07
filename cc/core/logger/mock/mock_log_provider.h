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

#include <cstdarg>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "core/common/uuid/src/uuid.h"
#include "core/logger/interface/log_provider_interface.h"
#include "core/logger/src/log_utils.h"

namespace google::scp::core::logger::mock {
class MockLogProvider : public LogProviderInterface {
 public:
  ExecutionResult Init() noexcept { return SuccessExecutionResult(); }

  ExecutionResult Run() noexcept { return SuccessExecutionResult(); }

  ExecutionResult Stop() noexcept { return SuccessExecutionResult(); }

  void Log(const LogLevel& level, const common::Uuid& parent_activity_id,
           const common::Uuid& activity_id,
           const std::string_view& component_name,
           const std::string_view& machine_name,
           const std::string_view& cluster_name,
           const std::string_view& location, const std::string_view& message,
           va_list args) noexcept override {
    std::string text = ToString(parent_activity_id) + "|" +
                       ToString(activity_id) + "|" + component_name.data() +
                       "|" + machine_name.data() + "|" + cluster_name.data() +
                       "|" + location.data() + "|" + level + ": " +
                       message.data();
    messages_.push_back(text);
  }

  std::vector<std::string> messages_;
};
}  // namespace google::scp::core::logger::mock
