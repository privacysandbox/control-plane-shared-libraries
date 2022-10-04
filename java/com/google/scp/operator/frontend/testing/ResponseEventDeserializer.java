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

package com.google.scp.operator.frontend.testing;

import com.amazonaws.services.lambda.runtime.events.APIGatewayProxyResponseEvent;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.scp.shared.mapper.TimeObjectMapper;

/** Deserializes the response body from an APIGatewayResponseEvent. */
public final class ResponseEventDeserializer {
  private static final ObjectMapper objectMapper = new TimeObjectMapper();

  /**
   * Deserializes the response body from the input apiGatewayProxyResponseEvent to an object of type
   * TResponse.
   */
  public static <TResponse> TResponse Deserialize(
      APIGatewayProxyResponseEvent apiGatewayProxyResponseEvent, Class<TResponse> classType)
      throws JsonProcessingException {
    return objectMapper.readValue(apiGatewayProxyResponseEvent.getBody(), classType);
  }
}
