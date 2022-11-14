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

#ifndef SCP_CPIO_LOCAL_LIB_CPIO_H_
#define SCP_CPIO_LOCAL_LIB_CPIO_H_

#include "public/core/interface/execution_result.h"

#include "local_cpio_options.h"

namespace google::scp::cpio {

/**
 * @brief To initialize and shutdown global CPIO objects for local testing.
 */
class LocalLibCpio {
 public:
  /**
   * @brief Initializes global CPIO objects for local testing.
   *
   * @param options global configurations for local testing.
   * @return core::ExecutionResult result of initializing CPIO.
   */
  static core::ExecutionResult InitCpio(LocalCpioOptions options);

  /**
   * @brief Shuts down global CPIO objects for local testing.
   *
   * @param options global configurations for local testing.
   * @return core::ExecutionResult result of terminating CPIO.
   */
  static core::ExecutionResult ShutdownCpio(LocalCpioOptions options);
};
}  // namespace google::scp::cpio

#endif  // SCP_CPIO_LOCAL_CPIO_OPTIONS_H_
