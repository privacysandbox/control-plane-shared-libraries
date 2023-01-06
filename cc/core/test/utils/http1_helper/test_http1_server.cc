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
#include "test_http1_server.h"

#include <unistd.h>

#include <string>
#include <utility>

#include "core/test/utils/conditional_wait.h"
#include "core/test/utils/http1_helper/errors.h"
#include "public/core/interface/execution_result.h"

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

using std::atomic_bool;
using std::move;
using std::string;
using std::thread;

namespace google::scp::core::test {
namespace {
void HandleErrorIfPresent(const boost::system::error_code& ec, string stage) {
  if (ec) {
    std::cerr << stage << " failed: " << ec << std::endl;
    exit(EXIT_FAILURE);
  }
}
}  // namespace

// Uses the C socket library to bind to an unused port, close that socket then
// return that port number.
ExecutionResultOr<in_port_t> GetUnusedPortNumber() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    return FailureExecutionResult(
        errors::SC_TEST_HTTP1_SERVER_ERROR_GETTING_SOCKET);
  }
  sockaddr_in server_addr;
  socklen_t server_len = sizeof(server_addr);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = 0;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(sockfd, reinterpret_cast<sockaddr*>(&server_addr), server_len) < 0) {
    return FailureExecutionResult(errors::SC_TEST_HTTP1_SERVER_ERROR_BINDING);
  }
  if (getsockname(sockfd, reinterpret_cast<sockaddr*>(&server_addr),
                  &server_len) < 0) {
    return FailureExecutionResult(
        errors::SC_TEST_HTTP1_SERVER_ERROR_GETTING_SOCKET_NAME);
  }
  close(sockfd);
  return server_addr.sin_port;
}

TestHttp1Server::TestHttp1Server() {
  auto port_number_or = GetUnusedPortNumber();
  if (!port_number_or.Successful()) {
    std::cerr << errors::GetErrorMessage(port_number_or.result().status_code)
              << std::endl;
    exit(EXIT_FAILURE);
  }
  port_number_ = *port_number_or;
  atomic_bool ready(false);
  thread_ = thread([this, &ready]() {
    boost::asio::io_context ioc{/*concurrency_hint=*/1};
    auto address = boost::asio::ip::make_address("0.0.0.0");
    tcp::acceptor acceptor{ioc, {address, port_number_}};
    tcp::socket socket{ioc};

    acceptor.async_accept(socket, [this, &socket](beast::error_code ec) {
      if (!ec) {
        ReadFromSocketAndWriteResponse(move(socket));
      } else {
        std::cerr << "accept failed: " << ec << std::endl;
        exit(EXIT_FAILURE);
      }
    });
    ready = true;
    ioc.run();
  });
  WaitUntil([&ready]() { return ready.load(); });
}

// Initiate the asynchronous operations associated with the connection.
void TestHttp1Server::ReadFromSocketAndWriteResponse(tcp::socket socket) {
  // The buffer for performing reads.
  beast::flat_buffer buffer{1024};
  beast::error_code ec;
  http::read(socket, buffer, request_, ec);
  HandleErrorIfPresent(ec, "read");

  // The response message.
  http::response<http::dynamic_body> response;
  response.version(request_.version());
  response.keep_alive(false);

  response.result(response_status_);
  response.set(http::field::server, "Beast");

  response.set(http::field::content_type, "text/html");
  beast::ostream(response.body()) << response_body_;
  response.content_length(response.body().size());

  http::write(socket, response, ec);
  HandleErrorIfPresent(ec, "write");

  socket.shutdown(tcp::socket::shutdown_send, ec);
  HandleErrorIfPresent(ec, "shutdown");
  socket.close(ec);
  HandleErrorIfPresent(ec, "close");
}

in_port_t TestHttp1Server::PortNumber() const {
  return port_number_;
}

// Returns the request object that this server received.
const http::request<http::dynamic_body>& TestHttp1Server::Request() const {
  return request_;
}

// Sets the HTTP response status to return to clients - default is OK.
void TestHttp1Server::SetResponseStatus(http::status status) {
  response_status_ = status;
}

// Sets the HTTP response body to return to clients - default is
// kBase64EncodedResponse.
void TestHttp1Server::SetResponseBody(string body) {
  response_body_ = body;
}

TestHttp1Server::~TestHttp1Server() {
  thread_.join();
}

}  // namespace google::scp::core::test
