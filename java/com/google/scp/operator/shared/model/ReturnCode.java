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

/** Return codes for aggregation worker. Based on (go/aggregation-service-return-code). */
@Deprecated
enum ReturnCode {
  // Job was successful.
  SUCCESS,
  // An unspecified fatal error occurred.
  UNSPECIFIED_ERROR,
  // The aggregation request failed because it took too long.
  TIMEOUT,
  // The aggregation job retried too many times by different workers.
  RETRIES_EXHAUSTED,
  // The aggregation service could not download the input data from cloud storage or the data was
  // malformed and could not be read
  INPUT_DATA_READ_FAILED,
  // The input contained too few reports to be aggregated.
  INPUT_DATA_TOO_SMALL,
  // The aggregation service could not write the aggregated result to cloud storage.
  OUTPUT_DATAWRITE_FAILED,
  // Job failed due to an internal issue in aggregation service and can be retried
  INTERNAL_ERROR
}
