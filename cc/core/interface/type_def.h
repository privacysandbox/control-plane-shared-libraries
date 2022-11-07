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

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "core/common/proto/common.pb.h"

namespace google::scp::core {
typedef uint64_t Timestamp;
typedef uint64_t TimeDuration;
typedef char Byte;
typedef uint64_t JournalId;
typedef uint64_t CheckpointId;

/// Struct that stores byte array and the metadata associated with it.
struct BytesBuffer {
  BytesBuffer() : BytesBuffer(0) {}

  explicit BytesBuffer(size_t size)
      : bytes(std::make_shared<std::vector<Byte>>(size)), capacity(size) {}

  std::shared_ptr<std::vector<Byte>> bytes;
  size_t length = 0;
  size_t capacity = 0;

  template <size_t N>
  inline void AssignBody(const char (&str)[N]) {
    bytes->assign(str, str + N - 1);  // mind the trailing null
    length = N - 1;
  }
};

typedef std::string PublicPrivateKeyPairId;

/// Struct that stores version metadata.
struct Version {
  uint64_t major = 0;
  uint64_t minor = 0;

  bool operator==(const Version& other) const {
    return major == other.major && minor == other.minor;
  }
};

// The http header for the client activity id
static constexpr char kClientActivityIdHeader[] = "x-gscp-client-activity-id";
static constexpr char kClaimedIdentityHeader[] = "x-gscp-claimed-identity";
static constexpr const char kAuthHeader[] = "x-auth-token";

struct LoadableObject {
  LoadableObject() : is_loaded(false), needs_loader(false) {}

  virtual ~LoadableObject() = default;

  std::atomic<bool> is_loaded;
  std::atomic<bool> needs_loader;
};

static constexpr TimeDuration kAsyncContextExpirationDurationInSeconds = 90;
}  // namespace google::scp::core
