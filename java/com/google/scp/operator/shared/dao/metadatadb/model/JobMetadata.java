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

package com.google.scp.operator.shared.dao.metadatadb.model;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.auto.value.AutoValue;
import com.google.scp.operator.protos.shared.backend.CreateJobRequestProto.CreateJobRequest;
import com.google.scp.operator.protos.shared.backend.JobKeyProto.JobKey;
import com.google.scp.operator.protos.shared.backend.JobStatusProto.JobStatus;
import com.google.scp.operator.protos.shared.backend.RequestInfoProto.RequestInfo;
import com.google.scp.operator.protos.shared.backend.ResultInfoProto.ResultInfo;
import java.time.Instant;
import java.util.Optional;
import java.util.OptionalInt;
import java.util.OptionalLong;

/**
 * The metadata for a job request.
 *
 * <p>NOTE: Classes exist that mirror this, please update those classes if you are adding fields to
 * this class.
 */
@Deprecated
@AutoValue
abstract class JobMetadata {

  /** Returns a new builder instance for this class. */
  public static Builder builder() {
    return new AutoValue_JobMetadata.Builder().setServerJobId("");
  }

  /** Creates a JobMetadata.Builder with values copied over. Used for creating an updated object. */
  public abstract Builder toBuilder();

  /** Unique key to identify the job. */
  @JsonProperty("job_key")
  public abstract JobKey jobKey();

  /** The time the request was received. */
  @JsonProperty("request_received_at")
  public abstract Instant requestReceivedAt();

  /** The time the request was updated. */
  @JsonProperty("request_updated_at")
  public abstract Instant requestUpdatedAt();

  /** Number of times the job has been attempted for processing. */
  @JsonProperty("num_attempts")
  public abstract int numAttempts();

  /** Enum value to represent the current status of the job. */
  @JsonProperty("job_status")
  public abstract JobStatus jobStatus();

  /** Server generated unique id to identify the job. */
  @JsonProperty("server_job_id")
  public abstract String serverJobId();

  /**
   * The body of the request that created the job.
   *
   * @deprecated use {@link RequestInfo} instead. Populated for backwards compatibility.
   */
  @Deprecated
  @JsonProperty("create_job_request")
  public abstract Optional<CreateJobRequest> createJobRequest();

  /**
   * The request info for the job.
   *
   * <p>Currently, an optional field for backwards compatibility but should always exist for
   * requests made.
   */
  @JsonProperty("request_info")
  public abstract Optional<RequestInfo> requestInfo();

  /**
   * The result information the worker generated when processing the job. Optional since this is
   * only set for jobs that have finished.
   */
  @JsonProperty("result_info")
  public abstract Optional<ResultInfo> resultInfo();

  /**
   * Version attribute for optimistic locking (compare-and-swap updates). This should never be
   * updated or accessed by anything outside of the JobMetadataDb implementation.
   */
  @JsonProperty("record_version")
  public abstract OptionalInt recordVersion();

  /** Expiration time for the record. */
  @JsonProperty("ttl")
  public abstract OptionalLong ttl();

  /** Performs equality check ignoring recordVersion and ttl. */
  public final boolean equalsIgnoreDbFields(Object other) {
    if (!(other instanceof JobMetadata)) {
      return false;
    }
    JobMetadata thisNoVersion =
        this.toBuilder().setRecordVersion(OptionalInt.empty()).setTtl(OptionalLong.empty()).build();
    JobMetadata otherNoVersion =
        ((JobMetadata) other)
            .toBuilder().setRecordVersion(OptionalInt.empty()).setTtl(OptionalLong.empty()).build();

    return thisNoVersion.equals(otherNoVersion);
  }

  /**
   * Returns createJobRequest value, or null if it is empty. This is used in conversion of model to
   * schemas which accept null instead of Optional.
   */
  public final CreateJobRequest getCreateJobRequestValue() {
    return createJobRequest().isEmpty() ? null : createJobRequest().get();
  }

  /**
   * Returns requestInfo value, or null if it is empty. This is used in conversion of model to
   * schemas which accept null instead of Optional.
   */
  public final RequestInfo getRequestInfoValue() {
    return requestInfo().isEmpty() ? null : requestInfo().get();
  }

  /**
   * Returns resultInfo value, or null if it is empty. This is used in conversion of model to
   * schemas which accept null instead of Optional.
   */
  public final ResultInfo getResultInfoValue() {
    return resultInfo().isEmpty() ? null : resultInfo().get();
  }

  /** Builder class for the {@code JobMetadata} class. */
  @AutoValue.Builder
  public abstract static class Builder {
    /** Set the unique key to identify the job. */
    public abstract Builder setJobKey(JobKey jobKey);

    /** Set the time the request was received. */
    public abstract Builder setRequestReceivedAt(Instant requestReceivedAt);

    /** Set the time the request was updated. */
    public abstract Builder setRequestUpdatedAt(Instant requestUpdatedAt);

    /** Set the number of times the job has been attempted for processing. */
    public abstract Builder setNumAttempts(int numAttempts);

    /** Set the enum value to represent the current status of the job. */
    public abstract Builder setJobStatus(JobStatus jobStatus);

    /** Set the server generated unique id to identify the job. */
    public abstract Builder setServerJobId(String serverJobId);

    /** Set the body of the request that created the job. */
    @Deprecated
    public abstract Builder setCreateJobRequest(CreateJobRequest createJobRequest);

    /**
     * Set the body of the request that created the job. Argument type Optional<ResultInfo> to
     * simplify conversion logic.
     */
    @Deprecated
    public abstract Builder setCreateJobRequest(Optional<CreateJobRequest> createJobRequest);

    /** Set the body of the request for the job. */
    public abstract Builder setRequestInfo(RequestInfo requestInfo);

    /**
     * Set the body of the request for the job. Argument type Optional<RequestInfo> to simplify
     * conversion logic.
     */
    public abstract Builder setRequestInfo(Optional<RequestInfo> requestInfo);

    /** Set the result information the worker generated when processing the job. */
    public abstract Builder setResultInfo(ResultInfo resultInfo);

    /**
     * Set the result information the worker generated when processing the job. Argument type
     * Optional<ResultInfo> to simplify conversion logic.
     */
    public abstract Builder setResultInfo(Optional<ResultInfo> resultInfo);

    /**
     * Set the version attribute for optimistic locking (compare-and-swap updates). This should
     * never used outside of the JobMetadataDb implementation.
     */
    public abstract Builder setRecordVersion(OptionalInt recordVersion);

    /** Set the expiration time for the record. */
    public abstract Builder setTtl(OptionalLong ttl);

    /** Returns a new instance of the {@code JobMetadata} class from the builder. */
    public abstract JobMetadata build();
  }
}
