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

package com.google.scp.operator.frontend.service;

import com.google.cmrt.sdk.job_service.v1.GetJobByIdRequest;
import com.google.cmrt.sdk.job_service.v1.GetJobByIdResponse;
import com.google.cmrt.sdk.job_service.v1.Job;
import com.google.cmrt.sdk.job_service.v1.PutJobRequest;
import com.google.cmrt.sdk.job_service.v1.PutJobResponse;
import com.google.common.base.Converter;
import com.google.inject.Inject;
import com.google.scp.operator.frontend.tasks.CreateJobTask;
import com.google.scp.operator.frontend.tasks.GetJobByIdTask;
import com.google.scp.operator.frontend.tasks.GetJobTask;
import com.google.scp.operator.frontend.tasks.PutJobTask;
import com.google.scp.operator.protos.frontend.api.v1.CreateJobRequestProto.CreateJobRequest;
import com.google.scp.operator.protos.frontend.api.v1.CreateJobResponseProto.CreateJobResponse;
import com.google.scp.operator.protos.frontend.api.v1.GetJobResponseProto.GetJobResponse;
import com.google.scp.operator.protos.shared.backend.RequestInfoProto.RequestInfo;
import com.google.scp.operator.protos.shared.backend.metadatadb.JobMetadataProto.JobMetadata;
import com.google.scp.shared.api.exception.ServiceException;

/** Handles business logic for the frontend service */
public final class FrontendServiceImpl implements FrontendService {

  private final Converter<CreateJobRequest, RequestInfo> createJobRequestToRequestInfoConverter;
  private final Converter<JobMetadata, GetJobResponse> getJobResponseConverter;
  private final CreateJobTask createJobTask;
  private final GetJobTask getJobTask;
  private final PutJobTask putJobTask;
  private final GetJobByIdTask getJobByIdTask;

  /** Creates a new instance of the {@code FrontendServiceImpl} class. */
  @Inject
  FrontendServiceImpl(
      CreateJobTask createJobTask,
      GetJobTask getJobTask,
      PutJobTask putJobTask,
      GetJobByIdTask getJobByIdTask,
      Converter<JobMetadata, GetJobResponse> getJobResponseConverter,
      Converter<CreateJobRequest, RequestInfo> createJobRequestToRequestInfoConverter) {
    this.createJobTask = createJobTask;
    this.getJobTask = getJobTask;
    this.putJobTask = putJobTask;
    this.getJobByIdTask = getJobByIdTask;
    this.getJobResponseConverter = getJobResponseConverter;
    this.createJobRequestToRequestInfoConverter = createJobRequestToRequestInfoConverter;
  }

  /** Creates the job from the request, then returns the response. */
  public CreateJobResponse createJob(CreateJobRequest createJobRequest) throws ServiceException {
    createJobTask.createJob(createJobRequestToRequestInfoConverter.convert(createJobRequest));
    return CreateJobResponse.newBuilder().build();
  }

  /** Gets the job with the provided ID. */
  public GetJobResponse getJob(String jobRequestId) throws ServiceException {
    return this.getJobResponseConverter.convert(getJobTask.getJob(jobRequestId));
  }

  /** Puts the job from the request, then returns the response. */
  public PutJobResponse putJob(PutJobRequest putJobRequest) throws ServiceException {
    Job job = putJobTask.putJob(putJobRequest.getJobId(), putJobRequest.getJobBody());
    return PutJobResponse.newBuilder().setJob(job).build();
  }

  /** Gets the job with the job ID. */
  public GetJobByIdResponse getJobById(GetJobByIdRequest getJobByIdRequest)
      throws ServiceException {
    Job job = getJobByIdTask.getJobById(getJobByIdRequest.getJobId());
    return GetJobByIdResponse.newBuilder().setJob(job).build();
  }
}
