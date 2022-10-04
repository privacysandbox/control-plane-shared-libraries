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

#include <gtest/gtest.h>

#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <thread>

#include "proxy/src/receive_worker.h"
#include "proxy/src/socks5_state.h"

using std::make_shared;
using std::thread;

namespace google::scp::proxy::test {
// This mocks a dest server with network problems
class TestServer {
 public:
  TestServer() : listen_fd_(-1), port_(0) {
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0;

    if (bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) <
        0) {
      std::cerr << "Error: Cannot bind. errno=" << errno << "\n";
      return;
    }
    if (listen(listen_fd_, 5) < 0) {
      std::cerr << "Error: Cannot listen. errno=" << errno << "\n";
      return;
    }
    socklen_t len = sizeof(sockaddr);
    if (getsockname(listen_fd_, reinterpret_cast<sockaddr*>(&addr), &len) < 0) {
      std::cerr << "Error: Cannot get bound port. errno=" << errno << "\n";
      return;
    }
    port_ = ntohs(addr.sin_port);
  }

  void ServeConnReset() {
    int conn = accept(listen_fd_, nullptr, nullptr);
    int val = 0;
    // Set SO_LINGER to 0 so that it immediately sends RST packet on close().
    setsockopt(conn, SOL_SOCKET, SO_LINGER, &val, sizeof(val));
    close(conn);
  }

  int listen_fd_;
  uint16_t port_;
};

TEST(SynchronizationTest, ServerConnResetProxyHang) {
  using BufferUnitType = uint8_t;
  TestServer server;
  ASSERT_GT(server.listen_fd_, 0);
  ASSERT_GT(server.port_, 0);
  // Make a socket pair, so that we can simulate a client to Socks5State.
  int sockfd[2];
  socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd);
  BufferUnitType buffer[] = {0x05, 0x01, 0x00,        // <- Greeting
                             0x05, 0x01, 0x00, 0x01,  // <- request header
                             0x7f, 0x00, 0x00, 0x01,  // <- addr = 127.0.0.1
                             0x00, 0x00};             // <- port placeholder
  // fill in the port
  uint16_t port = htons(server.port_);
  memcpy(buffer + sizeof(buffer) - 2, &port, sizeof(port));

  // Start a dest server thread
  auto dest_server = thread([&server]() { server.ServeConnReset(); });
  // Start receive workers
  auto worker = make_shared<ReceiveWorker>(sockfd[1]);
  worker->SetupCallbacks();
  auto socks5_worker =
      thread(&ReceiveWorker::Socks5Worker, worker, 65536 /*buffer size*/);
  worker.reset();

  timeval timeout{.tv_sec = 10, .tv_usec = 0};
  // Set a recv timeout so that this test fails on hang, instead of hanging
  // forever.
  setsockopt(sockfd[0], SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

  // Now we mimic what a socks5 client does.
  // Send handshake bytes.
  ssize_t r = send(sockfd[0], buffer, sizeof(buffer), 0);
  EXPECT_EQ(r, sizeof(buffer));
  // The dest server should've finished, as it accepts and drops.
  dest_server.join();
  // Per rfc1928, we should receive two response in following format:
  //  +----+--------+
  //  |VER | METHOD |
  //  +----+--------+
  //  | 1  |   1    |
  //  +----+--------+
  //  +----+-----+-------+------+----------+----------+
  //  |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
  //  +----+-----+-------+------+----------+----------+
  //  | 1  |  1  | X'00' |  1   | Variable |    2     |
  //  +----+-----+-------+------+----------+----------+
  // we should receive 12 bytes in total, as IPv4 BND.ADDR is 4 bytes long.
  char client_buf[12];
  // Here we should receive the handshake response bytes.
  ssize_t bytes_recv =
      recv(sockfd[0], client_buf, sizeof(client_buf), MSG_WAITALL);
  EXPECT_EQ(bytes_recv, 12) << "Bad socks5 response.";
  // recv again and we should fail this time.
  bytes_recv = recv(sockfd[0], client_buf, sizeof(client_buf), 0);
  int err = errno;
  EXPECT_EQ(bytes_recv, 0) << "Client sock not properly closed.";
  // This is the errno when timeout is reached, namely, when hang happens.
  EXPECT_NE(err, EWOULDBLOCK) << "Failed. The client might be hanging.";
  socks5_worker.detach();
}

TEST(SynchronizationTest, HandshakeTimeout) {
  using BufferUnitType = uint8_t;
  TestServer server;
  ASSERT_GT(server.listen_fd_, 0);
  ASSERT_GT(server.port_, 0);

  int sockfd[2];
  socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd);

  // Start receive workers
  auto socks5_worker =
      thread(&ReceiveWorker::Socks5Worker,
             make_shared<ReceiveWorker>(sockfd[1]), 65536 /*buffer size*/);

  timeval timeout{.tv_sec = 10, .tv_usec = 0};
  // Set a recv timeout so that this test fails on hang, instead of hanging
  // forever.
  setsockopt(sockfd[0], SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  auto client_buf = std::make_unique<BufferUnitType[]>(65536);
  // Here we haven't sent any handshake bytes, so we should recv nothing. But
  // here we only wait for the handshake to timeout on the proxy side.
  ssize_t bytes_recv = recv(sockfd[0], client_buf.get(), 65536, 0);
  int err = errno;
  EXPECT_EQ(bytes_recv, 0) << "Client sock not properly closed.";
  // This is the errno when timeout is reached, namely, when hang happens.
  EXPECT_NE(err, EWOULDBLOCK) << "Failed. The client might be hanging.";
  socks5_worker.detach();
}

}  // namespace google::scp::proxy::test
