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
#include "console_log_provider.h"

#include <iostream>
#include <string>
#include <string_view>

#include "core/common/uuid/src/uuid.h"
#include "core/logger/src/log_utils.h"
using google::scp::core::common::ToString;
using google::scp::core::common::Uuid;
using std::cout;
using std::endl;
using std::string_view;

namespace google::scp::core::logger {

ExecutionResult ConsoleLogProvider::Init() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult ConsoleLogProvider::Run() noexcept {
  return SuccessExecutionResult();
}

ExecutionResult ConsoleLogProvider::Stop() noexcept {
  return SuccessExecutionResult();
}

void ConsoleLogProvider::Log(
    const LogLevel& level, const Uuid& parent_activity_id,
    const Uuid& activity_id, const string_view& component_name,
    const string_view& machine_name, const string_view& cluster_name,
    const string_view& location, const string_view& message,
    va_list args) noexcept {
  cout << ToString(parent_activity_id) + "|" + ToString(activity_id) << "|"
       << component_name << "|" << machine_name << "|" << cluster_name << "|"
       << location << "|" << static_cast<int>(level) << ": " << message << endl;
}
}  // namespace google::scp::core::logger
