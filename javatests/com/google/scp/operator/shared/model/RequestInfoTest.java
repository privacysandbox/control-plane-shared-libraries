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
import com.google.scp.operator.protos.shared.backend.RequestInfoProto;
import com.google.scp.shared.mapper.TimeObjectMapper;
import java.util.Optional;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/** These tests are to ensure json to proto interoperability. */
@RunWith(JUnit4.class)
public final class RequestInfoTest {
  @Test
  public void protoFromJson() throws JsonProcessingException, InvalidProtocolBufferException {
    // Create a POJO instance
    RequestInfo.Builder builder = RequestInfo.builder();
    builder.jobRequestId("1");
    builder.inputDataBlobPrefix("test_input_prefix");
    builder.inputDataBlobBucket("test_input_bucket");
    builder.outputDataBlobPrefix("test_output_prefix");
    builder.outputDataBlobBucket("test_output_bucket");
    builder.postbackUrl(Optional.of("http://test.org"));
    builder.jobParameters(
        ImmutableMap.<String, String>builder().put("test_key", "test_value").build());
    RequestInfo request = builder.build();

    // Serialize that instance to JSON
    ObjectMapper mapper = new TimeObjectMapper();
    String json = mapper.writeValueAsString(request);

    // Deserialize from JSON to the Proto class.
    JsonFormat.Parser parser = JsonFormat.parser();
    RequestInfoProto.RequestInfo.Builder protoBuilder = RequestInfoProto.RequestInfo.newBuilder();
    parser.merge(json, protoBuilder);
    RequestInfoProto.RequestInfo requestInfoProto = protoBuilder.build();

    // Assert that the two objects are equivalent.
    assertThat(requestInfoProto.getJobRequestId()).isEqualTo(request.jobRequestId());
    assertThat(requestInfoProto.getInputDataBlobPrefix()).isEqualTo(request.inputDataBlobPrefix());
    assertThat(requestInfoProto.getInputDataBucketName()).isEqualTo(request.inputDataBlobBucket());
    assertThat(requestInfoProto.getOutputDataBlobPrefix())
        .isEqualTo(request.outputDataBlobPrefix());
    assertThat(requestInfoProto.getOutputDataBucketName())
        .isEqualTo(request.outputDataBlobBucket());
    assertThat(requestInfoProto.getPostbackUrl()).isEqualTo(request.postbackUrl().get());
    assertThat(requestInfoProto.getJobParametersMap()).isEqualTo(request.jobParameters());
  }
}
