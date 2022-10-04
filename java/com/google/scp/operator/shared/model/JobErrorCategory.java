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

package com.google.scp.operator.shared.model;

/**
 * Describes a category of error message, used to distinguish between various decryption or
 * validation failures reported by the {@link ErrorSummary} class.
 */
@Deprecated
enum JobErrorCategory {
  DECRYPTION_ERROR,
  HYBRID_KEY_ID_MISSING,
  UNSUPPORTED_OPERATION,
  GENERAL_ERROR,
  // TODO: Remove when removing numReportsWithErrors from shared ErrorSummary
  NUM_REPORTS_WITH_ERRORS
}
