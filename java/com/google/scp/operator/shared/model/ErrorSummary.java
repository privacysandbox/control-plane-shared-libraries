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
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonSerialize;
import com.google.auto.value.AutoValue;
import com.google.common.collect.ImmutableList;
import java.util.List;
import java.util.OptionalLong;

/** Summary of errors to be provided to the requester as debugging information. */
@JsonDeserialize(builder = ErrorSummary.Builder.class)
@JsonSerialize(as = ErrorSummary.class)
@Deprecated
@AutoValue
abstract class ErrorSummary {
  /** An instance of the {@code ErrorSummary} class that includes no errors. */
  public static final ErrorSummary EMPTY =
      ErrorSummary.builder()
          .setNumReportsWithErrors(OptionalLong.empty())
          .setErrorCounts(ImmutableList.of())
          .build();

  /** Creates a new builder instance for the class. */
  public static Builder builder() {
    return Builder.builder();
  }

  /**
   * Count of reports that had errors and thus were not included in computing the {@code
   * NoisedAggregationResult}.
   */
  @Deprecated
  @JsonProperty("num_reports_with_errors")
  public abstract OptionalLong numReportsWithErrors();

  /** Count of errors by category. */
  @JsonProperty("error_counts")
  public abstract ImmutableList<ErrorCount> errorCounts();

  /** Builder class for the {@code ErrorSummary} class. */
  @AutoValue.Builder
  public abstract static class Builder {

    /** Creates a new builder instance for the {@code ErrorSummary} class. */
    @JsonCreator
    public static Builder builder() {
      return new AutoValue_ErrorSummary.Builder();
    }

    /**
     * Set the count of reports that had errors and thus were not included in computing the {@code
     * NoisedAggregationResult}.
     */
    @Deprecated
    @JsonProperty("num_reports_with_errors")
    public abstract Builder setNumReportsWithErrors(OptionalLong countOfReportsWithErrors);

    /** Set the count of errors by category. */
    @JsonProperty("error_counts")
    public abstract Builder setErrorCounts(List<ErrorCount> errorCounts);

    /** Creates a new instance of the {@code ErrorSummary} class from the builder. */
    public abstract ErrorSummary build();
  }
}
