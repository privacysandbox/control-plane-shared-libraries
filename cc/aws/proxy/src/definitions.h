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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <sys/types.h>

#include <cstdint>

// Socket connection state
// clang-format off
enum class ConnectionState {
  Unknown,
  Connecting,
  Connected,
  Disconnected
 };
// clang-format on

// Linux socket handle - defined as a specific type for readability on API
typedef int SocketHandle;

#endif  // DEFINITIONS_H_
