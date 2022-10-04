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

#include "proxy/src/socket.h"

#include <gtest/gtest.h>

#include <sys/socket.h>
#include <unistd.h>

#include <memory>
#include <thread>

using std::make_unique;
using std::thread;

namespace google::scp::proxy::test {
TEST(SocketTest, ReadWrite) {
  int sock_fd[2];
  socketpair(AF_UNIX, SOCK_STREAM, 0, sock_fd);
  Socket sock1(sock_fd[0]);
  Socket sock2(sock_fd[1]);
  Buffer buff1;
  auto data = make_unique<int64_t[]>(1024 * 1024);
  for (int64_t i = 0; i < 1024 * 1024; ++i) {
    data[i] = i;
  }
  size_t data_size = 1024 * 1024 * sizeof(int64_t);
  buff1.CopyIn(data.get(), data_size);
  Buffer buff2;
  auto data_compare = make_unique<int64_t[]>(1024 * 1024);
  uint8_t* ptr = reinterpret_cast<uint8_t*>(data_compare.get());
  thread sender([&]() {
    while (buff1.data_size() > 0) {
      sock1.WriteSome(buff1);
    }
  });
  thread receiver([&]() {
    size_t read_size = 0;
    while (read_size < data_size) {
      sock2.ReadSome(buff2);
      auto sz = buff2.CopyOut(ptr + read_size, data_size);
      read_size += sz;
    }
  });
  sender.join();
  receiver.join();
  EXPECT_EQ(memcmp(data.get(), data_compare.get(), data_size), 0);
}

}  // namespace google::scp::proxy::test
