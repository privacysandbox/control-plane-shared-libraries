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

/** Count of errors for a given category. */
@JsonDeserialize(builder = ErrorCount.Builder.class)
@JsonSerialize(as = ErrorCount.class)
@Deprecated
@AutoValue
abstract class ErrorCount {
  /** Creates a builder instance for the class. */
  public static ErrorCount.Builder builder() {
    return ErrorCount.Builder.builder();
  }

  /** The category of the error. */
  @JsonProperty("category")
  public abstract String category();

  /** The count of errors in the specified category. */
  @JsonProperty("count")
  public abstract Long count();

  /** Builder class for the {@code ErrorCount} class. */
  @AutoValue.Builder
  public abstract static class Builder {

    /** Creates a builder instance for the {@code ErrorCount} class. */
    @JsonCreator
    public static ErrorCount.Builder builder() {
      return new AutoValue_ErrorCount.Builder();
    }

    /** Set the category of the error. */
    @JsonProperty("category")
    public abstract Builder setCategory(String value);

    /** Set the count of errors in the specified category. */
    @JsonProperty("count")
    public abstract Builder setCount(Long value);

    /** Returns a new instance of the {@code ErrorCount} class from the builder. */
    public abstract ErrorCount build();
  }
}
