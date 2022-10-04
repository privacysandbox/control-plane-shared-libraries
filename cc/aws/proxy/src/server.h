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

#ifndef SERVER_H_
#define SERVER_H_

#include <mutex>
#include <set>
#include <string>
#include <thread>

#include "proxy/src/definitions.h"

namespace google::scp::proxy {
// Manages a server TCP socket (or KVM hypervisor vsocket) and all its
// associated client connections.
class Server {
 public:
  explicit Server(uint16_t port, size_t buffer_size = 4096,
                  bool use_vsock = false)
      : default_size_(buffer_size), port_(port), vsock_(use_vsock) {}

  // Starts a server socket listening in the given address and port. If
  // in_enclave is false, a VSOCK is used instead of plain TCP. Once started,
  // the server socket is closed upon process termination. The use_vsock
  // parameter determines which type of socket is used. use_vsock = true means
  // that this instance will use VSOCK, and use_vsock = false means that this
  // instance will use regular TCP sockets.
  bool Start(uint16_t port = 0);

  void Serve();

  // Stops listener thread and closes all existing connections.
  void Stop();

  // Returns true if the server is listening for new connections.
  bool IsListening();

 protected:
  // State of listener thread
  ConnectionState listener_status_ = ConnectionState::Unknown;

  // Listener socket handle
  SocketHandle listener_handle_ = -1;

  // Listener thread
  std::thread listener_thread_;

  // Receive buffer size for client connections
  const size_t default_size_ = 4096;

  // Port to listen on.
  // TODO: make this const after refactoring.
  uint16_t port_;
  // Whether to listen on vsock.
  const bool vsock_;

  // Auxiliary function that accepts incoming connections and spawns threads
  // to handle them. Incoming connections can come from TCP clients or from
  // VSOCK. Each function handles a specific type of connection.
  void AcceptVSocketConnection();
  void AcceptSocketConnection();

  // Spawns worker thread
  void SpawnReceiveWorker(SocketHandle client_handle);

  // Worker thread function that listens to new incoming connections. The
  // use_vsock parameter determines if sockets or vsockets are allowed to
  // connect to this instance (true = vsocket, false = socket).
  void ListenerWorker();
};
}  // namespace google::scp::proxy
#endif  // SERVER_H_
