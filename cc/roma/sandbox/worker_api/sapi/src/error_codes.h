/*
 * Copyright 2023 Google LLC
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

#include "core/interface/errors.h"
#include "public/core/interface/execution_result.h"

namespace google::scp::core::errors {
REGISTER_COMPONENT_CODE(SC_ROMA_WORKER_API, 0x0B00)
DEFINE_ERROR_CODE(SC_ROMA_WORKER_API_UNINITIALIZED_WORKER, SC_ROMA_WORKER_API,
                  0x0001,
                  "A call to run code was issued with an uninitialized worker.",
                  HttpStatusCode::BAD_REQUEST)

DEFINE_ERROR_CODE(SC_ROMA_WORKER_API_COULD_NOT_INITIALIZE_SANDBOX,
                  SC_ROMA_WORKER_API, 0x0002,
                  "Could not initialize the SAPI sandbox.",
                  HttpStatusCode::BAD_REQUEST)

DEFINE_ERROR_CODE(SC_ROMA_WORKER_API_COULD_NOT_CREATE_IPC_PROTO,
                  SC_ROMA_WORKER_API, 0x0003,
                  "Could not create the IPC proto to send work to sandbox.",
                  HttpStatusCode::BAD_REQUEST)

DEFINE_ERROR_CODE(
    SC_ROMA_WORKER_API_COULD_NOT_GET_PROTO_MESSAGE_AFTER_EXECUTION,
    SC_ROMA_WORKER_API, 0x0004,
    "Could not get the IPC proto message after sandbox execution.",
    HttpStatusCode::BAD_REQUEST)

DEFINE_ERROR_CODE(SC_ROMA_WORKER_API_COULD_NOT_INITIALIZE_WRAPPER_API,
                  SC_ROMA_WORKER_API, 0x0005,
                  "Could not initialize the wrapper API.",
                  HttpStatusCode::BAD_REQUEST)

DEFINE_ERROR_CODE(SC_ROMA_WORKER_API_COULD_NOT_RUN_WRAPPER_API,
                  SC_ROMA_WORKER_API, 0x0006, "Could not run the wrapper API.",
                  HttpStatusCode::BAD_REQUEST)

DEFINE_ERROR_CODE(SC_ROMA_WORKER_API_COULD_NOT_STOP_WRAPPER_API,
                  SC_ROMA_WORKER_API, 0x0007, "Could not stop the wrapper API.",
                  HttpStatusCode::BAD_REQUEST)

DEFINE_ERROR_CODE(SC_ROMA_WORKER_API_COULD_NOT_RUN_CODE_THROUGH_WRAPPER_API,
                  SC_ROMA_WORKER_API, 0x0008,
                  "Could not run code through the wrapper API.",
                  HttpStatusCode::BAD_REQUEST)
}  // namespace google::scp::core::errors