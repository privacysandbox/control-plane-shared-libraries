// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "proxy/src/server.h"

#include <gtest/gtest.h>

namespace google::scp::proxy::test {
// Start and stop TCP server
TEST(ServerTests, BringUp) {
  Server server(8123, 4096, false);
  EXPECT_EQ(server.IsListening(), false);

  // TODO: Use a port picker
  bool success = server.Start();
  EXPECT_EQ(success, true);
  EXPECT_EQ(server.IsListening(), true);

  server.Stop();
  EXPECT_EQ(server.IsListening(), false);
}
}  // namespace google::scp::proxy::test
