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

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonSerialize;
import com.google.auto.value.AutoValue;

@AutoValue
@JsonDeserialize(builder = JobMessage.Builder.class)
@JsonSerialize(as = JobMessage.class)
@JsonIgnoreProperties(ignoreUnknown = true)
@Deprecated
abstract class JobMessage {

  /** Creates a new builder instance for this class. */
  public static JobMessage.Builder builder() {
    return new AutoValue_JobMessage.Builder();
  }

  /** User provided ID for request tracking. */
  @JsonProperty("jobRequestId")
  public abstract String jobRequestId();

  /** Server-side provided ID for request tracking. */
  @JsonProperty("serverJobId")
  public abstract String serverJobId();

  /** Builder class for the {@code JobMessage} class. */
  @JsonIgnoreProperties(ignoreUnknown = true)
  @AutoValue.Builder
  public abstract static class Builder {

    @JsonCreator
    public static JobMessage.Builder builder() {
      return JobMessage.builder();
    }

    /** Set the job request id. */
    @JsonProperty("jobRequestId")
    public abstract JobMessage.Builder setJobRequestId(String jobRequestId);

    /** Set the server job id. */
    @JsonProperty("serverJobId")
    public abstract JobMessage.Builder setServerJobId(String serverJobId);

    /** Creates a new instance of the {@code JobMessage} class from the builder. */
    public abstract JobMessage build();
  }
}
