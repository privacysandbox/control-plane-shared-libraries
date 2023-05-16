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
#include "core/interface/http_types.h"
#include "core/interface/partition_interface.h"
#include "core/interface/service_interface.h"
#include "core/interface/type_def.h"

namespace google::scp::core {

/**
 * @brief Request's Partition endpoint info
 *
 */
struct RequestPartitionEndpointInfo : public RequestEndpointInfo {
  RequestPartitionEndpointInfo(const std::shared_ptr<core::Uri>& uri,
                               const PartitionId& partition_id,
                               bool is_local_endpoint)
      : RequestEndpointInfo(uri, is_local_endpoint),
        partition_id_(partition_id) {}

  const PartitionId partition_id_;
};

typedef std::string PartitionAddressUri;

/**
 * @brief Information about Partition to be loaded/unloaded
 *
 */
struct PartitionMetadata {
  PartitionMetadata(PartitionId partition_id, PartitionType partition_type,
                    PartitionAddressUri partition_address_uri)
      : partition_id(partition_id),
        partition_type(partition_type),
        partition_address_uri(partition_address_uri) {}

  PartitionId Id() { return partition_id; }

  const PartitionId partition_id;
  const PartitionType partition_type;
  const PartitionAddressUri partition_address_uri;
};

/**
 * @brief Partition Manager manages partitions in the system. Upon recieving
 * signals to Load/Unload partitions, it boots up or tears down Partition
 * objects and manages lifetime of them.
 *
 */
class PartitionManagerInterface : public ServiceInterface {
 public:
  ~PartitionManagerInterface() = default;
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
   * @brief Update the partition's address of a partition.
   * When remote partition moves from one remote node to another remote node,
   * the address needs to be updated to keep track of the latest location of it
   * to forward requests to if needed.
   *
   * @param partitionInfo
   * @return ExecutionResult
   */
  virtual core::ExecutionResult RefreshPartitionAddress(
      const core::PartitionMetadata& partition_address) noexcept = 0;

  /**
   * @brief Get the Partition Address shared_ptr. Shared_ptr is returned to
   * avoid copies since this address may potentially be used for large number of
   * incoming requests.
   *
   * @param partition_id
   * @return core::ExecutionResultOr<core::PartitionAddressUri>
   */
  virtual core::ExecutionResultOr<std::shared_ptr<core::PartitionAddressUri>>
  GetPartitionAddress(core::PartitionId partition_id) noexcept = 0;

  /**
   * @brief Get the Partition Type
   *
   * @param partition_id
   * @return core::ExecutionResultOr<core::PartitionType>
   */
  virtual core::ExecutionResultOr<core::PartitionType> GetPartitionType(
      core::PartitionId partition_id) noexcept = 0;

  /**
   * @brief Get the Partition object for the Partition ID if already loaded. The
   * returned partition could be of any of PartitionType.
   *
   * @param partitionId
   * @return std::shared_ptr<Partition>
   */
  virtual ExecutionResultOr<std::shared_ptr<PartitionInterface>> GetPartition(
      PartitionId partitionId) noexcept = 0;
};
}  // namespace google::scp::core
