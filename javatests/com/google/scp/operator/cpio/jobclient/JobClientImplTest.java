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

package com.google.scp.operator.cpio.jobclient;

import static com.google.common.truth.Truth.assertThat;
import static com.google.common.truth.Truth8.assertThat;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;

import com.google.acai.Acai;
import com.google.acai.TestScoped;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.inject.AbstractModule;
import com.google.inject.Provides;
import com.google.inject.Singleton;
import com.google.protobuf.util.Durations;
import com.google.scp.operator.cpio.jobclient.JobClient.JobClientException;
import com.google.scp.operator.cpio.jobclient.JobHandlerModule.JobClientJobMaxNumAttemptsBinding;
import com.google.scp.operator.cpio.jobclient.model.Job;
import com.google.scp.operator.cpio.jobclient.model.JobResult;
import com.google.scp.operator.cpio.jobclient.testing.FakeJobGenerator;
import com.google.scp.operator.cpio.jobclient.testing.FakeJobResultGenerator;
import com.google.scp.operator.cpio.jobclient.testing.OneTimePullBackoff;
import com.google.scp.operator.cpio.lifecycleclient.LifecycleClient;
import com.google.scp.operator.cpio.lifecycleclient.local.LocalLifecycleClient;
import com.google.scp.operator.cpio.lifecycleclient.local.LocalLifecycleModule;
import com.google.scp.operator.cpio.metricclient.MetricClient;
import com.google.scp.operator.cpio.metricclient.local.LocalMetricClient;
import com.google.scp.operator.protos.shared.backend.CreateJobRequestProto.CreateJobRequest;
import com.google.scp.operator.protos.shared.backend.ErrorSummaryProto.ErrorSummary;
import com.google.scp.operator.protos.shared.backend.JobKeyProto.JobKey;
import com.google.scp.operator.protos.shared.backend.JobStatusProto.JobStatus;
import com.google.scp.operator.protos.shared.backend.RequestInfoProto.RequestInfo;
import com.google.scp.operator.protos.shared.backend.ResultInfoProto.ResultInfo;
import com.google.scp.operator.protos.shared.backend.ReturnCodeProto.ReturnCode;
import com.google.scp.operator.protos.shared.backend.jobqueue.JobQueueProto.JobQueueItem;
import com.google.scp.operator.protos.shared.backend.metadatadb.JobMetadataProto.JobMetadata;
import com.google.scp.operator.shared.dao.jobqueue.common.JobQueue;
import com.google.scp.operator.shared.dao.jobqueue.common.JobQueue.JobQueueException;
import com.google.scp.operator.shared.dao.jobqueue.testing.FakeJobQueue;
import com.google.scp.operator.shared.dao.metadatadb.common.JobMetadataDb;
import com.google.scp.operator.shared.dao.metadatadb.testing.FakeMetadataDb;
import com.google.scp.shared.proto.ProtoUtil;
import java.time.Clock;
import java.time.Duration;
import java.time.Instant;
import java.time.ZoneId;
import java.util.Optional;
import javax.inject.Inject;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public final class JobClientImplTest {

  @Rule public final Acai acai = new Acai(TestEnv.class);

  // Under test
  @Inject Clock clock;
  @Inject JobClientImpl jobClient;
  @Inject FakeJobQueue jobQueue;
  @Inject FakeMetadataDb jobMetadataDb;

  private JobQueueItem baseJobQueueItem;
  private JobMetadata baseJobMetadata;
  private Job baseJob;
  private Job expectedBaseJob;
  private RequestInfo requestInfo;

  private static final Instant requestReceivedAt = Instant.ofEpochSecond(100);
  private static final Instant requestUpdatedAt = Instant.ofEpochSecond(200);

  @Before
  public void setUp() {
    baseJobQueueItem =
        JobQueueItem.newBuilder()
            .setJobKeyString("request|abc.com")
            .setServerJobId("123")
            .setReceiptInfo("receipt")
            .setJobProcessingTimeout(Durations.fromSeconds(3600))
            .setJobProcessingStartTime(ProtoUtil.toProtoTimestamp(Instant.now()))
            .build();
    requestInfo =
        RequestInfo.newBuilder()
            .setJobRequestId("request")
            .setInputDataBlobPrefix("bar")
            .setInputDataBucketName("foo")
            .setOutputDataBlobPrefix("bar")
            .setOutputDataBucketName("foo")
            .setPostbackUrl("http://myUrl.com")
            .putAllJobParameters(
                ImmutableMap.of(
                    "attribution_report_to",
                    "abc.com",
                    "output_domain_blob_prefix",
                    "bar",
                    "output_domain_bucket_name",
                    "foo",
                    "debug_privacy_budget_limit",
                    "5"))
            .build();
    baseJobMetadata =
        JobMetadata.newBuilder()
            .setJobKey(JobKey.newBuilder().setJobRequestId("request").build())
            .setServerJobId("123")
            .setJobStatus(JobStatus.RECEIVED)
            .setRequestReceivedAt(ProtoUtil.toProtoTimestamp(requestReceivedAt))
            .setRequestUpdatedAt(ProtoUtil.toProtoTimestamp(requestUpdatedAt))
            .setNumAttempts(0)
            .setCreateJobRequest(
                CreateJobRequest.newBuilder()
                    .setJobRequestId("request")
                    .setAttributionReportTo("abc.com")
                    .setInputDataBlobPrefix("bar")
                    .setInputDataBucketName("foo")
                    .setOutputDataBlobPrefix("bar")
                    .setOutputDataBucketName("foo")
                    .setOutputDomainBucketName("fizz")
                    .setOutputDomainBlobPrefix("buzz")
                    .setPostbackUrl("http://myUrl.com")
                    .setDebugPrivacyBudgetLimit(5)
                    .putAllJobParameters(
                        ImmutableMap.of(
                            "attribution_report_to",
                            "abc.com",
                            "output_domain_blob_prefix",
                            "bar",
                            "output_domain_bucket_name",
                            "foo",
                            "debug_privacy_budget_limit",
                            "5"))
                    .build())
            .setRequestInfo(requestInfo)
            .build();

    baseJob = FakeJobGenerator.generate("foo");
    expectedBaseJob =
        Job.builder()
            .setJobKey(baseJobMetadata.getJobKey())
            .setJobProcessingTimeout(
                ProtoUtil.toJavaDuration(baseJobQueueItem.getJobProcessingTimeout()))
            .setRequestInfo(requestInfo)
            .setCreateTime(requestReceivedAt)
            .setUpdateTime(requestUpdatedAt)
            .setNumAttempts(0)
            .setJobStatus(JobStatus.RECEIVED)
            .build();
  }

  @Test
  public void getJob_getsFakeJob() throws JobClientException {
    jobQueue.setJobQueueItemToBeReceived(Optional.of(baseJobQueueItem));
    jobMetadataDb.setJobMetadataToReturn(Optional.of(baseJobMetadata));

    Optional<Job> actual = jobClient.getJob();

    assertThat(actual).isPresent();
    assertThat(actual.get()).isEqualTo(expectedBaseJob);
    assertThat(jobMetadataDb.getLastJobMetadataUpdated().getJobStatus())
        .isEqualTo(JobStatus.IN_PROGRESS);
    // Check the worker start process time also updates
    assertThat(jobMetadataDb.getLastJobMetadataUpdated().getRequestProcessingStartedAt())
        .isEqualTo(ProtoUtil.toProtoTimestamp(Instant.now(clock)));
  }

  @Test
  public void getJob_exhaustsRetry() throws JobClientException {
    jobQueue.setJobQueueItemToBeReceived(Optional.empty());
    jobMetadataDb.setJobMetadataToReturn(Optional.empty());

    Optional<Job> actual = jobClient.getJob();

    assertThat(actual).isEmpty();
  }

  @Test
  public void getJob_ignoresJobWhenJobMetadataNotFound() throws JobClientException {
    jobQueue.setJobQueueItemToBeReceived(Optional.of(baseJobQueueItem));
    jobMetadataDb.setJobMetadataToReturn(Optional.empty());

    Optional<Job> actual = jobClient.getJob();

    // ignores the queue item because no metadata entry is found,
    // then exhausts puller backoff and returns empty.
    assertThat(actual).isEmpty();
    // make sure {@code acknowledgeJobCompletion} is called
    assertThat(jobQueue.getLastJobQueueItemSent()).isEqualTo(baseJobQueueItem);
  }

  @Test
  public void getJob_deleteMessageForServerJobIdMismatch()
      throws JobClientException, JobQueueException {
    jobQueue.setJobQueueItemToBeReceived(Optional.of(baseJobQueueItem));
    jobMetadataDb.setJobMetadataToReturn(
        Optional.of(baseJobMetadata.toBuilder().setServerJobId("456").build()));

    Optional<Job> actual = jobClient.getJob();

    assertThat(actual).isEmpty();
    // make sure {@code acknowledgeJobCompletion} is called
    assertThat(jobQueue.getLastJobQueueItemSent()).isEqualTo(baseJobQueueItem);
    assertTrue(jobQueue.receiveJob().isEmpty());
  }

  @Test
  public void getJob_retriesWhenJobMetadataNotFound() throws JobClientException, JobQueueException {
    jobQueue.setJobQueueItemToBeReceived(Optional.of(baseJobQueueItem));
    jobMetadataDb.setJobMetadataToReturn(Optional.of(baseJobMetadata));
    jobMetadataDb.setInitialLookupFailureCount(5);

    Optional<Job> actual = jobClient.getJob();

    assertThat(actual).isPresent();
    assertThat(actual.get()).isEqualTo(expectedBaseJob);
    assertThat(jobMetadataDb.getLastJobMetadataUpdated().getJobStatus())
        .isEqualTo(JobStatus.IN_PROGRESS);
    // Check the worker start process time also updates when retry
    assertThat(jobMetadataDb.getLastJobMetadataUpdated().getRequestProcessingStartedAt())
        .isEqualTo(ProtoUtil.toProtoTimestamp(Instant.now(clock)));
  }

  @Test
  public void getJob_ignoresJobWhenStatusFinished() throws JobClientException, JobQueueException {
    jobQueue.setJobQueueItemToBeReceived(Optional.of(baseJobQueueItem));
    jobMetadataDb.setJobMetadataToReturn(
        Optional.of(baseJobMetadata.toBuilder().setJobStatus(JobStatus.FINISHED).build()));

    Optional<Job> actual = jobClient.getJob();

    // ignores the queue item because the job is already finished,
    // then exhausts puller backoff and returns empty.
    assertThat(actual).isEmpty();
    // make sure {@code acknowledgeJobCompletion} is called
    assertThat(jobQueue.getLastJobQueueItemSent()).isEqualTo(baseJobQueueItem);
  }

  @Test
  public void getJob_marksJobWithExhaustedAttemptsAsFailed() throws JobClientException {
    jobQueue.setJobQueueItemToBeReceived(Optional.of(baseJobQueueItem));
    jobMetadataDb.setJobMetadataToReturn(
        Optional.of(
            baseJobMetadata.toBuilder()
                .setJobStatus(JobStatus.IN_PROGRESS)
                .setNumAttempts(2)
                .build()));

    Optional<Job> actual = jobClient.getJob();

    // ignores the queue item because the job has exhausted attempts,
    // then exhausts puller backoff and returns empty.
    assertThat(actual).isEmpty();
    // make sure job metadata updated
    ResultInfo resultInfo = jobMetadataDb.getLastJobMetadataUpdated().getResultInfo();
    assertThat(resultInfo.getFinishedAt())
        .isEqualTo(ProtoUtil.toProtoTimestamp(Instant.now(clock)));
    assertThat(resultInfo.getReturnMessage()).contains("Number of retry attempts exhausted");
    assertThat(resultInfo.getReturnCode()).isEqualTo(ReturnCode.RETRIES_EXHAUSTED.name());
    // make sure {@code acknowledgeJobCompletion} is called
    assertThat(jobQueue.getLastJobQueueItemSent()).isEqualTo(baseJobQueueItem);
  }

  @Test
  public void markJobCompleted_marksJobCompletion() throws Exception {
    jobQueue.setJobQueueItemToBeReceived(Optional.of(baseJobQueueItem));
    jobMetadataDb.setJobMetadataToReturn(Optional.of(baseJobMetadata));

    Job job = jobClient.getJob().get();
    ResultInfo resultInfo =
        ResultInfo.newBuilder()
            .setFinishedAt(ProtoUtil.toProtoTimestamp(Instant.ofEpochSecond(1234)))
            .setReturnCode(ReturnCode.SUCCESS.name())
            .setReturnMessage("success")
            .setErrorSummary(ErrorSummary.getDefaultInstance())
            .build();
    JobResult result =
        JobResult.builder().setJobKey(job.jobKey()).setResultInfo(resultInfo).build();
    jobMetadataDb.setJobMetadataToReturn(Optional.of(jobMetadataDb.getLastJobMetadataUpdated()));
    jobClient.markJobCompleted(result);

    JobMetadata expectedMetadata =
        baseJobMetadata.toBuilder()
            .setJobStatus(JobStatus.FINISHED)
            .setResultInfo(resultInfo)
            .setNumAttempts(1)
            .setRequestProcessingStartedAt(
                jobMetadataDb.getLastJobMetadataUpdated().getRequestProcessingStartedAt())
            .build();
    // make sure job metadata updated
    assertThat(jobMetadataDb.getLastJobMetadataUpdated()).isEqualTo(expectedMetadata);
    // make sure job is removed from the job queue
    assertThat(jobQueue.getLastJobQueueItemSent()).isEqualTo(baseJobQueueItem);
  }

  @Test
  public void markJobCompleted_throwsMetadataNotFound() throws JobClientException {
    jobQueue.setJobQueueItemToBeReceived(Optional.of(baseJobQueueItem));
    jobMetadataDb.setJobMetadataToReturn(Optional.of(baseJobMetadata));

    Job job = jobClient.getJob().get();
    JobResult result = FakeJobResultGenerator.fromJob(job);
    jobMetadataDb.setJobMetadataToReturn(Optional.empty());

    assertThrows(JobClientException.class, () -> jobClient.markJobCompleted(result));
  }

  @Test
  public void markJobCompleted_throwsJobStatusNotInProgress() throws JobClientException {
    jobQueue.setJobQueueItemToBeReceived(Optional.of(baseJobQueueItem));
    jobMetadataDb.setJobMetadataToReturn(Optional.of(baseJobMetadata));

    Job job = jobClient.getJob().get();
    JobResult result =
        JobResult.builder()
            .setJobKey(job.jobKey())
            .setResultInfo(
                ResultInfo.newBuilder()
                    .setFinishedAt(ProtoUtil.toProtoTimestamp(Instant.ofEpochSecond(1234)))
                    .setReturnCode(ReturnCode.SUCCESS.name())
                    .setReturnMessage("success")
                    .setErrorSummary(
                        ErrorSummary.newBuilder()
                            .setNumReportsWithErrors(0)
                            .addAllErrorCounts(ImmutableList.of())
                            .build())
                    .build())
            .build();
    jobMetadataDb.setJobMetadataToReturn(
        Optional.of(baseJobMetadata.toBuilder().setJobStatus(JobStatus.FINISHED).build()));

    assertThrows(JobClientException.class, () -> jobClient.markJobCompleted(result));
  }

  @Test
  public void markJobCompleted_throwsJobNotInCache() {
    jobQueue.setJobQueueItemToBeReceived(Optional.of(baseJobQueueItem));
    jobMetadataDb.setJobMetadataToReturn(Optional.of(baseJobMetadata));
    Job job = FakeJobGenerator.generate("request");
    JobResult result = FakeJobResultGenerator.fromJob(job);

    jobMetadataDb.setJobMetadataToReturn(
        Optional.of(baseJobMetadata.toBuilder().setJobStatus(JobStatus.FINISHED).build()));

    assertThrows(JobClientException.class, () -> jobClient.markJobCompleted(result));
  }

  @Test
  public void buildJob_buildsJob() throws JobClientException {
    // no setup

    Job actual = jobClient.buildJob(baseJobQueueItem, baseJobMetadata);

    assertThat(actual).isEqualTo(expectedBaseJob);
  }

  @Test
  public void isDuplicateJob_overTimeoutReturnsFalse() {
    Job job =
        baseJob.toBuilder()
            .setJobStatus(JobStatus.IN_PROGRESS)
            .setCreateTime(Instant.parse("2021-01-01T12:24:00Z"))
            .setUpdateTime(Instant.parse("2021-01-01T12:24:59Z"))
            .setJobProcessingTimeout(Duration.ofMinutes(5))
            .build();

    assertThat(jobClient.isDuplicateJob(Optional.of(job))).isEqualTo(false);
  }

  @Test
  public void isDuplicateJob_receivedStatusReturnsFalse() {
    Job job =
        baseJob.toBuilder()
            .setJobStatus(JobStatus.RECEIVED)
            .setCreateTime(Instant.parse("2021-01-01T12:28:00Z"))
            .setUpdateTime(Instant.parse("2021-01-01T12:29:00Z"))
            .setJobProcessingTimeout(Duration.ofMinutes(5))
            .build();

    assertThat(jobClient.isDuplicateJob(Optional.of(job))).isEqualTo(false);
  }

  @Test
  public void isDuplicateJob_duplicateReturnsTrue() {
    Job job =
        baseJob.toBuilder()
            .setJobStatus(JobStatus.IN_PROGRESS)
            .setCreateTime(Instant.parse("2021-01-01T12:28:00Z"))
            .setUpdateTime(Instant.parse("2021-01-01T12:29:00Z"))
            .setJobProcessingTimeout(Duration.ofMinutes(5))
            .build();

    assertThat(jobClient.isDuplicateJob(Optional.of(job))).isEqualTo(true);
  }

  private static final class TestEnv extends AbstractModule {

    @Override
    protected void configure() {
      bind(FakeMetadataDb.class).in(TestScoped.class);
      bind(FakeJobQueue.class).in(TestScoped.class);
      bind(JobQueue.class).to(FakeJobQueue.class);
      bind(JobMetadataDb.class).to(FakeMetadataDb.class);
      bind(JobPullBackoff.class).to(OneTimePullBackoff.class);
      bind(Integer.class).annotatedWith(JobClientJobMaxNumAttemptsBinding.class).toInstance(1);
      install(new JobValidatorModule());
      install(new LocalLifecycleModule());
      bind(LifecycleClient.class).to(LocalLifecycleClient.class);
      bind(MetricClient.class).to(LocalMetricClient.class);
    }

    @Provides
    @Singleton
    Clock provideClock() {
      return Clock.fixed(Instant.parse("2021-01-01T12:30:00Z"), ZoneId.systemDefault());
    }
  }
}
