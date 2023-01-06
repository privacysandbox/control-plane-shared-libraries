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

#include <netinet/in.h>

#include <string>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include "public/core/interface/execution_result.h"

namespace google::scp::core::test {

// Returns an unused TCP port number.
ExecutionResultOr<in_port_t> GetUnusedPortNumber();

// Lightweight Boost HTTP/1.1 server to run the token fetcher against.
// TODO adjust the mock server to be more flexible.
class TestHttp1Server {
 public:
  // Run the mock server on a random unused port.
  TestHttp1Server();

  // Initiate the asynchronous operations associated with the connection.
  void ReadFromSocketAndWriteResponse(boost::asio::ip::tcp::socket socket);

  in_port_t PortNumber() const;

  // Returns the request object that this server received.
  const boost::beast::http::request<boost::beast::http::dynamic_body>& Request()
      const;

  // Sets the HTTP response status to return to clients - default is OK.
  void SetResponseStatus(boost::beast::http::status status);

  // Sets the HTTP response body to return to clients - default is
  // kBase64EncodedResponse.
  void SetResponseBody(std::string body);

  ~TestHttp1Server();

 private:
  boost::beast::http::request<boost::beast::http::dynamic_body> request_;

  boost::beast::http::status response_status_ = boost::beast::http::status::ok;

  std::string response_body_;

  std::thread thread_;
  in_port_t port_number_ = 0;
};

}  // namespace google::scp::core::test
