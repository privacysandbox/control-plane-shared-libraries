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

package com.google.scp.operator.cpio.distributedprivacybudgetclient;

/** Status code returned from the execution of an operation. */
public enum StatusCode {
  UNKNOWN,
  OK,
  MALFORMED_DATA,
  MAP_ENTRY_DOES_NOT_EXIST,
  TRANSACTION_MANAGER_CANNOT_INITIALIZE,
  TRANSACTION_MANAGER_ALREADY_STARTED,
  TRANSACTION_MANAGER_NOT_STARTED,
  TRANSACTION_MANAGER_CANNOT_ACCEPT_NEW_REQUESTS,
  TRANSACTION_MANAGER_INVALID_MAX_CONCURRENT_TRANSACTIONS_VALUE,
  TRANSACTION_MANAGER_RETRIES_EXCEEDED,
  TRANSACTION_ENGINE_CANNOT_START_TRANSACTION,
  TRANSACTION_ENGINE_CANNOT_ACCEPT_TRANSACTION,
  TRANSACTION_ENGINE_INVALID_DISTRIBUTED_COMMAND,
  TRANSACTION_ENGINE_TRANSACTION_IS_EXPIRED,
  TRANSACTION_ENGINE_TRANSACTION_FAILED,
  TRANSACTION_MANAGER_ALREADY_STOPPED,
  TRANSACTION_MANAGER_NOT_FINISHED,
  TRANSACTION_MANAGER_TRANSACTION_NOT_FOUND,
  TRANSACTION_MANAGER_INVALID_TRANSACTION_LOG,
  TRANSACTION_MANAGER_TERMINATABLE_TRANSACTION,
  TRANSACTION_MANAGER_INVALID_TRANSACTION_PHASE,
  TRANSACTION_MANAGER_CURRENT_TRANSACTION_IS_RUNNING,
  TRANSACTION_MANAGER_TRANSACTION_UNKNOWN
}
