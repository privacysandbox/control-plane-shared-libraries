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

package com.google.scp.operator.shared.dao.jobqueue.model;

import com.google.auto.value.AutoValue;
import java.time.Duration;
import java.time.Instant;

/** Class to represent an item on the job queue. */
@AutoValue
@Deprecated
abstract class JobQueueItem {

  /** Returns a new instance of the builder for this class. */
  public static Builder newBuilder() {
    return new AutoValue_JobQueueItem.Builder().setServerJobId("");
  }

  /** Set the key of the job as a String. */
  public abstract String jobKeyString();

  /**
   * Set the processing time-out of the job, after which the message will be visible to other
   * workers.
   */
  public abstract Duration jobProcessingTimeout();

  /** Set the processing start time. */
  public abstract Instant jobProcessingStartTime();

  /**
   * Set the item receipt info that will be used to acknowledge processing of the item with the
   * JobQueue.
   *
   * <p>At present all the JobQueue implementations can use a string for this field, but in the
   * future it can be replaced by a more complex object or AutoOneOf if needed.
   */
  public abstract String receiptInfo();

  public abstract String serverJobId();

  /** Builder class for the {@code JobQueueItem} class. */
  @AutoValue.Builder
  public abstract static class Builder {

    /** Get the key of the job as a String. */
    public abstract Builder setJobKeyString(String jobKeyString);

    /**
     * Get the processing time-out of the job, after which the message will be visible to other
     * workers.
     */
    public abstract Builder setJobProcessingTimeout(Duration jobProcessingTimeout);

    /** Get the processing start time of the job. */
    public abstract Builder setJobProcessingStartTime(Instant jobProcessingStartTime);

    /**
     * Get the item receipt info that will be used to acknowledge processing of the item with the
     * JobQueue.
     *
     * <p>At present all the JobQueue implementations can use a string for this field, but in the
     * future it can be replaced by a more complex object or AutoOneOf if needed.
     */
    public abstract Builder setReceiptInfo(String receiptInfo);

    public abstract Builder setServerJobId(String serverJobId);

    /** Returns a new instance of the {@code JobQueueItem} class from the builder. */
    public abstract JobQueueItem build();
  }
}
