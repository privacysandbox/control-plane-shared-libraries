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

#ifndef RECEIVE_WORKER_H_
#define RECEIVE_WORKER_H_

#include <memory>

#include "proxy/src/definitions.h"
#include "proxy/src/socks5_state.h"

namespace google::scp::proxy {

class ReceiveWorker : public std::enable_shared_from_this<ReceiveWorker> {
 public:
  explicit ReceiveWorker(SocketHandle client_sock);

  ~ReceiveWorker() {
    close(client_sock_);
    close(dest_sock_);
  }

  void SetupCallbacks();

  // The thread worker for reading from client, handling handshake, and
  // forwarding traffic to dest hosts.
  void Socks5Worker(size_t buffer_size);

  // The thread worker for forwarding traffic from dest to client.
  void DestToClientForwarder(size_t buffer_size);

 private:
  Socks5State state_;
  Buffer upstream_buffer_;
  Buffer downstream_buffer_;
  SocketHandle client_sock_;
  SocketHandle dest_sock_;
};

}  // namespace google::scp::proxy
#endif  // RECEIVE_WORKER_H_
