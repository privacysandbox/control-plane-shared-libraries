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

#include "proxy/src/receive_worker.h"

#include <netdb.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <thread>

#include "proxy/src/logging.h"
#include "proxy/src/send.h"
#include "proxy/src/socks5_state.h"

#include "buffer.h"

using std::thread;
namespace google::scp::proxy {
// NOLINTNEXTLINE(readability/todo)
// TODO: Refactor the threading logic, use non-blocking io multiplexing
// and get rid of this file completely.

static constexpr int64_t kSocketTimeoutSec = 5;

ReceiveWorker::ReceiveWorker(SocketHandle client_sock)
    : client_sock_(client_sock), dest_sock_(-1) {}

void ReceiveWorker::SetupCallbacks() {
  state_.SetConnectCallback([this](const sockaddr* addr, size_t size) {
    dest_sock_ = socket(addr->sa_family, SOCK_STREAM, 0);
    if (dest_sock_ < 0) {
      return Socks5State::kStatusFail;
    }
    auto ret = connect(dest_sock_, addr, size);
    if (ret < 0) {
      close(dest_sock_);
      dest_sock_ = -1;
      return Socks5State::kStatusFail;
    }
    int nodelay = 1;
    setsockopt(dest_sock_, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
    return Socks5State::kStatusOK;
  });

  state_.SetResponseCallback([this](const void* data, size_t len) {
    ssize_t sent_size = send(client_sock_, data, len, 0);
    if (sent_size != static_cast<ssize_t>(len)) {
      return Socks5State::kStatusFail;
    }
    return Socks5State::kStatusOK;
  });

  state_.SetDestAddressCallback([this](sockaddr* addr, size_t* len) {
    socklen_t socklen = static_cast<socklen_t>(*len);
    auto ret = getsockname(dest_sock_, addr, &socklen);
    if (ret < 0) {
      return Socks5State::kStatusFail;
    }
    *len = socklen;
    return Socks5State::kStatusOK;
  });
}

void ReceiveWorker::Socks5Worker(size_t buffer_size) {
  auto self = shared_from_this();
  timeval timeout{.tv_sec = kSocketTimeoutSec, .tv_usec = 0};
  if (setsockopt(client_sock_, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                 sizeof(timeout))) {
    LogError("Client setsockopt failed, errno=", errno);
    return;
  }
  while (true) {
    size_t to_reserve = buffer_size;
    auto bufs = upstream_buffer_.ReserveAtLeast<iovec>(to_reserve);
    ssize_t bytes_recv = readv(client_sock_, &bufs[0], bufs.size());
    if (bytes_recv < 0) {
      upstream_buffer_.Commit(0);
      if (errno == EINTR) {
        continue;
      }
      if (errno == EWOULDBLOCK || errno == EAGAIN) {
        // This is the timeout case. Check the status here. If we haven't
        // completed handshake, or the other side is gone, then we close.
        if (state_.state() != Socks5State::kSuccess) {
          LogError("Client connection ", client_sock_, " handshake timeout.");
          break;
        }
        if (state_.DownstreamDone()) {
          LogError("Closing client connection ", client_sock_,
                   " as the other side is gone.");
          break;
        }
        // Otherwise keep receiving.
        continue;
      }
      LogError("Client connection ", client_sock_,
               " read failed. errno=", errno);
      break;
    }  // if (bytes_recv < 0)
    upstream_buffer_.Commit(static_cast<size_t>(bytes_recv));
    if (bytes_recv == 0) {
      LogError("Client connection ", client_sock_, " closed by peer.");
      break;
    }
    // If handshake has completed
    if (state_.state() == Socks5State::kSuccess) {
      ssize_t size = static_cast<ssize_t>(upstream_buffer_.data_size());
      bufs = upstream_buffer_.Peek<iovec>();
      if (writev(dest_sock_, &bufs[0], bufs.size()) != size) {
        upstream_buffer_.Drain(0);
        LogError("Dest connection ", dest_sock_,
                 " write failed, errno=", errno);
        break;
      }
      upstream_buffer_.Drain(upstream_buffer_.data_size());
      continue;
    }

    // Otherwise, do handshake.
    while (state_.state() != Socks5State::kSuccess &&
           state_.Proceed(upstream_buffer_)) {}
    if (state_.state() == Socks5State::kFail) {
      break;
    }
    if (state_.state() == Socks5State::kSuccess) {
      thread t(&ReceiveWorker::DestToClientForwarder, self, buffer_size);
      t.detach();
      continue;
    }
    if (state_.InsufficientBuffer(upstream_buffer_)) {
      continue;
    }
  }
  state_.SetUpstreamDone();
}

void ReceiveWorker::DestToClientForwarder(size_t buffer_size) {
  auto self = shared_from_this();
  timeval timeout{.tv_sec = kSocketTimeoutSec, .tv_usec = 0};
  if (setsockopt(dest_sock_, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                 sizeof(timeout))) {
    LogError("Dest setsockopt failed, errno=", errno);
    return;
  }
  while (true) {
    size_t to_reserve = buffer_size;
    auto bufs = downstream_buffer_.ReserveAtLeast<iovec>(to_reserve);
    ssize_t bytes_recv = readv(dest_sock_, &bufs[0], bufs.size());
    if (bytes_recv < 0) {
      downstream_buffer_.Commit(0);
      if (errno == EINTR) {
        continue;
      }
      if (errno == EWOULDBLOCK || errno == EAGAIN) {
        // This is the timeout case. Check the status here.
        if (state_.UpstreamDone()) {
          LogError("Closing dest connection ", dest_sock_,
                   ", as the other side is gone");
          break;
        } else {
          // Otherwise keep receiving.
          continue;
        }
      }
      LogError("Dest Connection ", dest_sock_, " errno=", errno,
               ", closing connection");
      break;
    }
    downstream_buffer_.Commit(static_cast<size_t>(bytes_recv));
    if (bytes_recv == 0) {
      LogError("Dest Connection ", dest_sock_, " closed by peer.");
      break;
    }
    ssize_t size = static_cast<ssize_t>(downstream_buffer_.data_size());
    bufs = downstream_buffer_.Peek<iovec>();
    if (writev(client_sock_, &bufs[0], bufs.size()) != size) {
      downstream_buffer_.Drain(0);
      LogError("Client connection ", client_sock_,
               " write failed. errno=", errno);
      break;
    }
    downstream_buffer_.Drain(downstream_buffer_.data_size());
  }
  state_.SetDownstreamDone();
}
}  // namespace google::scp::proxy
