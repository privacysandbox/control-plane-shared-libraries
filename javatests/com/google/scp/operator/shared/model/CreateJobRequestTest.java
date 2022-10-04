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

import static com.google.common.truth.Truth.assertThat;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.common.collect.ImmutableMap;
import com.google.protobuf.InvalidProtocolBufferException;
import com.google.protobuf.util.JsonFormat;
import com.google.scp.operator.protos.shared.backend.CreateJobRequestProto;
import com.google.scp.shared.mapper.TimeObjectMapper;
import java.util.Optional;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/** These tests are to ensure json to proto interoperability. */
@RunWith(JUnit4.class)
public final class CreateJobRequestTest {
  @Test
  public void protoFromJson() throws JsonProcessingException, InvalidProtocolBufferException {
    // Create a POJO instance
    CreateJobRequest.Builder builder = CreateJobRequest.builder();
    builder.jobRequestId("1");
    builder.inputDataBlobPrefix("test_input_prefix");
    builder.inputDataBlobBucket("test_input_bucket");
    builder.outputDataBlobPrefix("test_output_prefix");
    builder.outputDataBlobBucket("test_output_bucket");
    builder.outputDomainBlobPrefix(Optional.of("test_output_domain_prefix"));
    builder.outputDomainBlobBucket(Optional.of("test_output_domain_bucket"));
    builder.postbackUrl(Optional.of("http://test.org"));
    builder.attributionReportTo("test_attribution_report_to");
    builder.debugPrivacyBudgetLimit(Optional.of(2));
    builder.jobParameters(
        ImmutableMap.<String, String>builder().put("test_key", "test_value").build());
    CreateJobRequest request = builder.build();

    // Serialize that instance to JSON
    ObjectMapper mapper = new TimeObjectMapper();
    String json = mapper.writeValueAsString(request);

    // Deserialize from JSON to the Proto class.
    JsonFormat.Parser parser = JsonFormat.parser();
    CreateJobRequestProto.CreateJobRequest.Builder protoBuilder =
        CreateJobRequestProto.CreateJobRequest.newBuilder();
    parser.merge(json, protoBuilder);
    CreateJobRequestProto.CreateJobRequest createJobRequestProto = protoBuilder.build();

    // Assert that the two objects are equivalent.
    assertThat(createJobRequestProto.getJobRequestId()).isEqualTo(request.jobRequestId());
    assertThat(createJobRequestProto.getInputDataBlobPrefix())
        .isEqualTo(request.inputDataBlobPrefix());
    assertThat(createJobRequestProto.getInputDataBucketName())
        .isEqualTo(request.inputDataBlobBucket());
    assertThat(createJobRequestProto.getOutputDataBlobPrefix())
        .isEqualTo(request.outputDataBlobPrefix());
    assertThat(createJobRequestProto.getOutputDataBucketName())
        .isEqualTo(request.outputDataBlobBucket());
    assertThat(createJobRequestProto.getOutputDomainBlobPrefix())
        .isEqualTo(request.outputDomainBlobPrefix().get());
    assertThat(createJobRequestProto.getOutputDomainBucketName())
        .isEqualTo(request.outputDomainBlobBucket().get());
    assertThat(createJobRequestProto.getPostbackUrl()).isEqualTo(request.postbackUrl().get());
    assertThat(createJobRequestProto.getAttributionReportTo())
        .isEqualTo(request.attributionReportTo());
    assertThat(createJobRequestProto.getDebugPrivacyBudgetLimit())
        .isEqualTo(request.debugPrivacyBudgetLimit().get());
    assertThat(createJobRequestProto.getJobParametersMap()).isEqualTo(request.jobParameters());
  }
}
