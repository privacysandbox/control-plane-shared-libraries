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
 * The request object for the Aggregation Service Frontend API that allows ad-techs to request
 * aggregation reports for conversion data.
 *
 * <p>NOTE: Classes exist that mirror this, please update those classes if you are adding fields to
 * this class.
 */
@JsonDeserialize(builder = CreateJobRequest.Builder.class)
@JsonSerialize(as = CreateJobRequest.class)
@Deprecated
@AutoValue
abstract class CreateJobRequest {

  /** Creates a builder instance for this class. */
  public static Builder builder() {
    return Builder.builder();
  }

  /** Creates a builder instance with values copied over. Used for creating an updated object. */
  public abstract Builder toBuilder();

  /** Unique identifier provided by the ad-tech. */
  @JsonProperty("job_request_id")
  public abstract String jobRequestId();

  /** Name of the input prefix data file. */
  @JsonProperty("input_data_blob_prefix")
  public abstract String inputDataBlobPrefix();

  /** Bucket name for the input data file. */
  @JsonProperty("input_data_bucket_name")
  public abstract String inputDataBlobBucket();

  /** Name of the output prefix data file. */
  @JsonProperty("output_data_blob_prefix")
  public abstract String outputDataBlobPrefix();

  /** Bucket name for the output data file. */
  @JsonProperty("output_data_bucket_name")
  public abstract String outputDataBlobBucket();

  /** URL to the output domain file. This is derived from {@code jobParameters}. */
  @JsonProperty("output_domain_blob_prefix")
  public abstract Optional<String> outputDomainBlobPrefix();

  /** URL to the output domain file. This is derived from {@code jobParameters}. */
  @JsonProperty("output_domain_bucket_name")
  public abstract Optional<String> outputDomainBlobBucket();

  /** URL to notify the result. */
  @JsonProperty("postback_url")
  public abstract Optional<String> postbackUrl();

  /** Ad-tech origin where reports will be sent. This is derived from {@code jobParameters}. */
  @JsonProperty("attribution_report_to")
  public abstract String attributionReportTo();

  /**
   * Optional debugging privacy-budget-limit to be passed to privacy-budget-service during the
   * origin trial period. This is derived from {@code jobParameters}.
   */
  @JsonProperty("debug_privacy_budget_limit")
  public abstract Optional<Integer> debugPrivacyBudgetLimit();

  /** Data plane application specific parameters. */
  @JsonProperty("job_parameters")
  public abstract ImmutableMap<String, String> jobParameters();

  /** Builder class for the {@code CreateJobRequest} class. */
  @AutoValue.Builder
  public abstract static class Builder {
    /** Returns a new builder instance for the {@code CreateJobRequest} class. */
    @JsonCreator
    public static Builder builder() {
      return new AutoValue_CreateJobRequest.Builder();
    }

    /** Set the unique identifier provided by the ad-tech. */
    @JsonProperty("job_request_id")
    public abstract Builder jobRequestId(String jobRequestId);

    /** Set the name of the input prefix data file. */
    @JsonProperty("input_data_blob_prefix")
    public abstract Builder inputDataBlobPrefix(String inputDataBlobPrefix);

    /** Set the bucket name for the input data file. */
    @JsonProperty("input_data_bucket_name")
    public abstract Builder inputDataBlobBucket(String inputDataBlobBucket);

    /** Set the name of the output prefix data file. */
    @JsonProperty("output_data_blob_prefix")
    public abstract Builder outputDataBlobPrefix(String outputDataBlobPrefix);

    /** Set the bucket name for the output data file. */
    @JsonProperty("output_data_bucket_name")
    public abstract Builder outputDataBlobBucket(String outputDataBlobBucket);

    /** Set the URL to the output domain file. This is derived from {@code jobParameters}. */
    @JsonProperty("output_domain_blob_prefix")
    public abstract Builder outputDomainBlobPrefix(Optional<String> outputDomainBlobPrefix);

    /** Set the URL to the output domain file. This is derived from {@code jobParameters}. */
    @JsonProperty("output_domain_bucket_name")
    public abstract Builder outputDomainBlobBucket(Optional<String> outputDomainBlobBucket);

    /** Set the URL to notify the result. */
    @JsonProperty("postback_url")
    public abstract Builder postbackUrl(Optional<String> postbackUrl);

    /**
     * Set the ad-tech origin where reports will be sent. This is derived from {@code
     * jobParameters}.
     */
    @JsonProperty("attribution_report_to")
    public abstract Builder attributionReportTo(String attributionReportTo);

    /**
     * Set the optional debugging privacy-budget-limit to be passed to privacy-budget-service during
     * the origin trial period. This is derived from {@code jobParameters}.
     */
    @JsonProperty("debug_privacy_budget_limit")
    public abstract Builder debugPrivacyBudgetLimit(Optional<Integer> debugPrivacyBudgetLimit);

    /** Set the data plane application specific parameters. */
    @JsonProperty("job_parameters")
    public abstract Builder jobParameters(Map<String, String> jobParameters);

    /** Returns a new instance of the {@code CreateJobRequest} class from the builder. */
    public abstract CreateJobRequest build();
  }
}
