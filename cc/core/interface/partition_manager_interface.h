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

#include "core/common/uuid/src/uuid.h"
#include "core/interface/partition_interface.h"
#include "core/interface/service_interface.h"
#include "core/interface/type_def.h"

namespace google::scp::core {

typedef std::string PartitionAddressUri;
/**
 * @brief Represents types of Partitions that can be loaded.
 *
 * Local: The Partition's home address is this instance.
 * Remote: The Partition's home address is another instance.
 *
 * NOTE: If lease is obtained on a partition by this instance, then it is
 * considered to be home on this instance.
 */
enum class PartitionType { Local, Remote };

/**
 * @brief Information about Partition to be loaded/unloaded
 *
 */
struct PartitionMetadata {
  PartitionMetadata(PartitionId partition_id, PartitionType partition_type,
                    PartitionAddressUri partition_address_uri)
      : partition_id_(partition_id),
        partition_type_(partition_type),
        partition_home_address_uri_(partition_address_uri) {}

  const PartitionId partition_id_;
  const PartitionType partition_type_;
  /// @brief home address would be empty if PartitionType is Local
  const PartitionAddressUri partition_home_address_uri_;
};

/**
 * @brief Partition Manager manages partitions in the system. Upon recieving
 * signals to Load/Unload partitions, it boots up or tears down Partition
 * objects and manages lifetime of them.
 *
 */
class PartitionManagerInterface : public ServiceInterface {
 public:
  /**
   * @brief Loads a partition.
   *
   * @param partitionInfo
   * @return ExecutionResult
   */
  virtual ExecutionResult LoadPartition(
      PartitionMetadata partitionInfo) noexcept = 0;

  /**
   * @brief Unloads a partition.
   *
   * @param partitionInfo
   * @return ExecutionResult
   */
  virtual ExecutionResult UnloadPartition(
      PartitionMetadata partitionInfo) noexcept = 0;

  /**
   * @brief Get the Partition object for the Partition ID if already loaded. The
   * returned partition could be of any of PartitionType.
   *
   * @param partitionId
   * @return std::shared_ptr<Partition>
   */
  virtual std::shared_ptr<PartitionInterface> GetPartition(
      PartitionId partitionId) noexcept = 0;
};
}  // namespace google::scp::core
