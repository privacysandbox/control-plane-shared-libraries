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

import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.auto.value.AutoValue;

/** Unique identifier for the job, used for identifying entries in the database and Pub/Sub. */
@Deprecated
@AutoValue
abstract class JobKey {

  /** Creates a new builder instance for this class. */
  public static Builder builder() {
    return new AutoValue_JobKey.Builder();
  }

  /** ID for request tracking. */
  @JsonProperty("job_request_id")
  public abstract String jobRequestId();

  /** String representation of the key. */
  public final String toKeyString() {
    return jobRequestId();
  }

  /** Builder class for the {@code JobKey} class. */
  @AutoValue.Builder
  public abstract static class Builder {

    /** Set the ID for request tracking. */
    public abstract Builder setJobRequestId(String jobRequestId);

    /** Creates a new instance of the {@code JobKey} class from the builder. */
    public abstract JobKey build();
  }
}
