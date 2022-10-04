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

#include <signal.h>

#include <thread>

#include "proxy/src/config.h"
#include "proxy/src/logging.h"
#include "proxy/src/server.h"

using google::scp::proxy::Config;
using google::scp::proxy::LogError;
using google::scp::proxy::LogInfo;
using google::scp::proxy::Server;

// Main loop - it all starts here...
int main(int argc, char* argv[]) {
  LogInfo("Nitro Enclave Proxy (c) Google 2022\n");

  {
    // Ignore SIGPIPE.
    struct sigaction act {};

    act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &act, nullptr);
  }

  // Process command line parameters
  auto config = Config::Parse(argc, argv);
  if (config.bad_) {
    return 1;
  }

  Server server(config.socks5_port_, config.buffer_size_, config.vsock_);

  // NOLINTNEXTLINE(readability/todo)
  // TODO: get rid of these magic numbers and make them configurable
  int retries = 5;
  int delay = 500;
  while (!server.Start() && retries-- > 0) {
    delay *= 2;
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
  }
  if (retries < 0) {
    LogError("ERROR: cannot start SOCKS5 server at port ", config.socks5_port_);
    return 1;
  }

  server.Serve();

  LogError("ERROR: A fatal error has occurred, terminating proxy instance");
  return 1;
}
