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
import com.google.common.collect.ImmutableMap;
import java.util.Map;
import java.util.Optional;

/**
 * The request info for the job requested by the operator.
 *
 * <p>NOTE: Classes exist that mirror this, please update those classes if you are adding fields to
 * this class.
 */
@AutoValue
@JsonDeserialize(builder = RequestInfo.Builder.class)
@JsonSerialize(as = RequestInfo.class)
@Deprecated
abstract class RequestInfo {

  /** Creates a builder instance for this class. */
  public static RequestInfo.Builder builder() {
    return RequestInfo.Builder.builder();
  }

  /** Creates a Builder with values copied over. Used for creating an updated object. */
  public abstract RequestInfo.Builder toBuilder();

  /** Unique identifier provided by the operator. */
  @JsonProperty("job_request_id")
  public abstract String jobRequestId();

  /** URL of the input data file. */
  @JsonProperty("input_data_blob_prefix")
  public abstract String inputDataBlobPrefix();

  /** Bucket name to store the input data file. */
  @JsonProperty("input_data_bucket_name")
  public abstract String inputDataBlobBucket();

  /** URL of the output data file. */
  @JsonProperty("output_data_blob_prefix")
  public abstract String outputDataBlobPrefix();

  /** Bucket name to store the output data file. */
  @JsonProperty("output_data_bucket_name")
  public abstract String outputDataBlobBucket();

  /** URL to notify the result. */
  @JsonProperty("postback_url")
  public abstract Optional<String> postbackUrl();

  /** Data plane application specific parameters. */
  @JsonProperty("job_parameters")
  public abstract ImmutableMap<String, String> jobParameters();

  /** Builder class for the {@code RequestInfo} class. */
  @AutoValue.Builder
  public abstract static class Builder {

    /** Returns a new builder instance for the {@code RequestInfo} class. */
    @JsonCreator
    public static RequestInfo.Builder builder() {
      return new AutoValue_RequestInfo.Builder();
    }

    /** Set the unique identifier provided by the operator */
    @JsonProperty("job_request_id")
    public abstract RequestInfo.Builder jobRequestId(String jobRequestId);

    /** Set the URL of the input data file. */
    @JsonProperty("input_data_blob_prefix")
    public abstract RequestInfo.Builder inputDataBlobPrefix(String inputDataBlobPrefix);

    /** Set the bucket name to store the input data file. */
    @JsonProperty("input_data_bucket_name")
    public abstract RequestInfo.Builder inputDataBlobBucket(String inputDataBlobBucket);

    /** Set the URL of the output data file. */
    @JsonProperty("output_data_blob_prefix")
    public abstract RequestInfo.Builder outputDataBlobPrefix(String outputDataBlobPrefix);

    /** Set the bucket name to store the output data file. */
    @JsonProperty("output_data_bucket_name")
    public abstract RequestInfo.Builder outputDataBlobBucket(String outputDataBlobBucket);

    /** Set URL to notify the result. */
    @JsonProperty("postback_url")
    public abstract RequestInfo.Builder postbackUrl(Optional<String> postbackUrl);

    /** Set data plane application specific parameters. */
    @JsonProperty("job_parameters")
    public abstract RequestInfo.Builder jobParameters(Map<String, String> jobParameters);

    public abstract RequestInfo build();
  }
}
