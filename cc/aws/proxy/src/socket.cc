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

#include "socket.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>

namespace google::scp::proxy {

void Socket::SetNonBlocking(bool nonblocking) {
  if (sock_ < 0) {
    return;
  }
  int flags = fcntl(sock_, F_GETFL);
  if (nonblocking) {
    flags |= O_NONBLOCK;
  } else {
    flags &= ~O_NONBLOCK;
  }
  fcntl(sock_, F_SETFL, flags);
}

void Socket::ReadSome(Buffer& to_buffer) {
  ssize_t read_size = 0;
  // Keep reading until failure or EOF. This is required for edge triggering
  // epoll event handling.
  while (true) {
    auto bufs = to_buffer.ReserveAtLeast<iovec>(kReadSize);
    size_t num_bufs = bufs.size() > IOV_MAX ? IOV_MAX : bufs.size();
    read_size = readv(sock_, &bufs[0], num_bufs);
    if (read_size <= 0) {
      to_buffer.Commit(0);
      break;
    }
    to_buffer.Commit(static_cast<size_t>(read_size));
  }
  // read_size == 0 means we've hit EOF. We leave the read_errno_ untouched to
  // indicate EOF.
  if (read_size == 0) {
    read_eof_ = true;
    read_errno_ = 0;
    return;
  }

  // read_size < 0, we've hit some errors. EWOULDBLOCK or EAGAIN means we've
  // completely drained the OS buffer, and it would block if it were a blocking
  // socket.  Otherwise, some real error happened. Record the errno.
  read_errno_ = errno;
}

void Socket::WriteSome(Buffer& from_buffer) {
  ssize_t write_size = 0;
  while (true) {
    auto bufs = from_buffer.Peek<iovec>();
    size_t num_bufs = bufs.size() > IOV_MAX ? IOV_MAX : bufs.size();
    write_size = writev(sock_, &bufs[0], num_bufs);
    if (write_size < 0) {
      // Drain(0) to mark the write is complete.
      from_buffer.Drain(0);
      // write_size < 0, we've hit some errors. Similar to ReadSome(),
      // EWOULDBLOCK and EAGAIN are benign errors that only indicates the OS
      // buffer is full, and continue writing would block if it were a blocking
      // socket.
      write_errno_ = errno;
      return;
    }
    from_buffer.Drain(static_cast<size_t>(write_size));
    if (from_buffer.data_size() == 0) {
      // We've written everything in the buffer. Nothing left, so return.
      write_errno_ = 0;
      return;
    }
  }
}

}  // namespace google::scp::proxy
