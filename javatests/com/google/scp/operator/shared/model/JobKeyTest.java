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
import com.google.protobuf.InvalidProtocolBufferException;
import com.google.protobuf.util.JsonFormat;
import com.google.scp.operator.protos.shared.backend.JobKeyProto;
import com.google.scp.shared.mapper.TimeObjectMapper;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/** These tests are to ensure json to proto interoperability. */
@RunWith(JUnit4.class)
public final class JobKeyTest {

  @Test
  public void testJobKeyString() {
    String jobRequestId = "abc";
    JobKey jobKey = JobKey.builder().setJobRequestId(jobRequestId).build();

    String jobKeyString = jobKey.toKeyString();

    assertThat(jobKeyString).isEqualTo("abc");
  }

  @Test
  public void protoFromJson() throws JsonProcessingException, InvalidProtocolBufferException {
    // Create a POJO instance
    JobKey.Builder builder = JobKey.builder();
    builder.setJobRequestId("test id");
    JobKey jobKey = builder.build();

    // Serialize that instance to JSON
    ObjectMapper mapper = new TimeObjectMapper();
    String json = mapper.writeValueAsString(jobKey);

    // Deserialize from JSON to the Proto class.
    JsonFormat.Parser parser = JsonFormat.parser();
    JobKeyProto.JobKey.Builder protoBuilder = JobKeyProto.JobKey.newBuilder();
    parser.merge(json, protoBuilder);
    JobKeyProto.JobKey jobKeyProto = protoBuilder.build();

    // Assert that the two objects are equivalent.
    assertThat(jobKeyProto.getJobRequestId()).isEqualTo(jobKey.jobRequestId());
  }
}
