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

#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>

#include "buffer.h"

namespace google::scp::proxy {
// A wrapper class for non-blocking stream sockets.
class Socket {
 public:
  static constexpr size_t kReadSize = 64 * 1024;

  explicit Socket(int fd = -1)
      : sock_(fd), read_errno_(0), write_errno_(0), read_eof_(false) {
    SetNonBlocking();
  }

  ~Socket() { close(sock_); }

  void WrapSocket(int fd) {
    sock_ = fd;
    SetNonBlocking();
  }

  // Return the file descriptor of this socket.
  int NativeHandle() const { return sock_; }

  // Set the socket to be non-blocking.
  void SetNonBlocking(bool nonblocking = true);

  // Read some bytes into \a to_buffer. We'll read as much as we can while not
  // blocking, which essentially drains the OS buffer.
  void ReadSome(Buffer& to_buffer);

  // Write some bytes from \a from_buffer. We'll write as much as we can while
  // not blocking.
  void WriteSome(Buffer& from_buffer);

  void ShutDown(int flags = SHUT_RDWR) { shutdown(sock_, flags); }

  // Close the socket.
  void Close() {
    close(sock_);
    sock_ = -1;
  }

  // Returns true if the socket is still readable.
  bool Readable() const { return !read_eof_ && BenignErrno(read_errno_); }

  // Returns true if the socket is still writable.
  // By standard, an EOF from reading the socket (i.e. a FIN packet) does not
  // necessarily mean the socket is not writable ("half closed socket").
  // However, that's rarely the case and usually not supported by end devices
  // and routing devices on common networks. So when read hits error, we
  // consider the socket is no longer writable either. On the contrary, a write
  // error does not mean the socket is not readable, as we may have remaining
  // bytes in the OS buffer to read.
  bool Writable() const { return Readable() && BenignErrno(write_errno_); }

  // Returns true if we've read to EOF.
  bool ReadEof() const { return read_eof_; }

  // Returns the errno we hit during read.
  int ReadErrno() const { return read_errno_; }

  // Returns the errno we hit during write.
  int WriteErrno() const { return write_errno_; }

  bool NeedPollRead() const {
    return read_errno_ == EWOULDBLOCK || read_errno_ == EAGAIN;
  }

  bool NeedPollWrite() const {
    return write_errno_ == EWOULDBLOCK || write_errno_ == EAGAIN;
  }

 private:
  static bool BenignErrno(int errno_value) {
    return errno_value == 0 || errno_value == EWOULDBLOCK ||
           errno_value == EAGAIN || errno_value == EINTR;
  }

  // The actual socket file descriptor.
  int sock_;
  int read_errno_;
  int write_errno_;
  bool read_eof_;
};
}  // namespace google::scp::proxy
