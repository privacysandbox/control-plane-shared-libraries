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

/** Enum class representing all the transaction execution phases. */
public enum TransactionExecutionPhase {
  BEGIN(1),
  PREPARE(2),
  COMMIT(3),
  NOTIFY(4),
  ABORT(5),
  END(6),
  UNKNOWN(1000);

  private int numValue;

  TransactionExecutionPhase(int val) {
    this.numValue = val;
  }

  public int getNumValue() {
    return this.numValue;
  }
};
