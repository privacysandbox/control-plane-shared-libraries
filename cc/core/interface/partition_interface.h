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

#include "core/interface/initializable_interface.h"
#include "core/interface/partition_interface.h"
#include "core/transaction_manager/interface/transaction_engine_interface.h"

namespace google::scp::core {

typedef common::Uuid PartitionId;

/**
 * @brief Partition represents encapsulation of components that operate on a
 * partitioned space of data/namespace.
 */
class PartitionInterface : public InitializableInterface {
 public:
  /**
   * @brief Initialize required data for loading the partition.
   *
   * @return ExecutionResult
   */
  virtual ExecutionResult Init() noexcept = 0;

  /**
   * @brief Load a partition from store and bring it into life.
   *
   * @return ExecutionResult
   */
  virtual ExecutionResult Load() noexcept = 0;

  /**
   * @brief Teardown partition by stopping partition's work and discarding any
   * pending work.
   *
   * @return ExecutionResult
   */
  virtual ExecutionResult Unload() noexcept = 0;

  /**
   * @brief Check if the partition has completed unloading.
   *
   * @return ExecutionResult
   */
  virtual bool IsUnloaded() noexcept = 0;
};

}  // namespace google::scp::core
