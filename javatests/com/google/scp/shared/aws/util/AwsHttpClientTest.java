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

package com.google.scp.shared.aws.util;

import static com.google.common.truth.Truth.assertThat;
import static org.junit.Assert.assertThrows;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import com.google.common.collect.ImmutableMap;
import java.io.IOException;
import java.util.Map;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.apache.hc.client5.http.async.methods.SimpleHttpResponse;
import org.apache.hc.client5.http.impl.async.CloseableHttpAsyncClient;
import org.apache.hc.core5.http.ConnectionRequestTimeoutException;
import org.apache.hc.core5.http.Header;
import org.apache.hc.core5.http.HttpStatus;
import org.apache.hc.core5.http.message.BasicHeader;
import org.apache.hc.core5.reactor.IOReactorStatus;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;

@RunWith(JUnit4.class)
public final class AwsHttpClientTest {
  @Rule public final MockitoRule mockito = MockitoJUnit.rule();

  @Mock CloseableHttpAsyncClient httpClient;
  @Mock Future<SimpleHttpResponse> responseFuture;
  private final String endpointUrl = "https://www.google.com/";
  private final String relativePath = "phasePath";

  private AwsHttpClient awsHttpClient;
  private Header[] responseHeaders;

  @Before
  public void setup() throws IOException {
    when(httpClient.getStatus()).thenReturn(IOReactorStatus.ACTIVE);
    awsHttpClient = new AwsHttpClient(httpClient);
    this.responseHeaders =
        new BasicHeader[] {
          new BasicHeader("resKey1", "resVal1"), new BasicHeader("resKey2", "resVal2")
        };
  }

  @Test
  public void executeGet_success() throws IOException, ExecutionException, InterruptedException {
    SimpleHttpResponse response1 = SimpleHttpResponse.create(HttpStatus.SC_OK, "");
    response1.setHeaders(responseHeaders);
    Map<String, String> requestHeaders =
        ImmutableMap.of("reqKey1", "reqVal1", "reqKey2", "reqVal2");
    when(httpClient.execute(any(), any())).thenReturn(responseFuture);
    when(responseFuture.get()).thenReturn(response1);

    HttpClientResponse response =
        awsHttpClient.executeGet(endpointUrl + relativePath, requestHeaders);

    assertThat(response.statusCode()).isEqualTo(HttpStatus.SC_OK);
    assertThat(response.headers()).isEqualTo(headerArrayToMap(responseHeaders));
    assertThat(response.responseBody()).isEmpty();
    verify(httpClient, never()).start();
    verify(httpClient, times(1)).execute(any(), any());
    verify(responseFuture, times(1)).get();
  }

  @Test
  public void executePost_success() throws IOException, ExecutionException, InterruptedException {
    String payload = "dummy payload string";
    SimpleHttpResponse response1 = SimpleHttpResponse.create(HttpStatus.SC_OK, "");
    response1.setHeaders(responseHeaders);
    Map<String, String> requestHeaders =
        ImmutableMap.of("reqKey1", "reqVal1", "reqKey2", "reqVal2");
    when(httpClient.execute(any(), any())).thenReturn(responseFuture);
    when(responseFuture.get()).thenReturn(response1);

    HttpClientResponse response =
        awsHttpClient.executePost(endpointUrl + relativePath, payload, requestHeaders);

    assertThat(response.statusCode()).isEqualTo(HttpStatus.SC_OK);
    assertThat(response.headers()).isEqualTo(headerArrayToMap(responseHeaders));
    assertThat(response.responseBody()).isEmpty();
    verify(httpClient, never()).start();
    verify(httpClient, times(1)).execute(any(), any());
    verify(responseFuture, times(1)).get();
  }

  @Test
  public void executePost_success_afterRetry()
      throws IOException, ExecutionException, InterruptedException {
    String payload = "dummy payload string";
    SimpleHttpResponse response1 = SimpleHttpResponse.create(HttpStatus.SC_OK, "");
    response1.setHeaders(responseHeaders);
    Map<String, String> requestHeaders =
        ImmutableMap.of("reqKey1", "reqVal1", "reqKey2", "reqVal2");
    ExecutionException exception =
        new ExecutionException(new ConnectionRequestTimeoutException("Connection timed out"));
    when(httpClient.execute(any(), any())).thenReturn(responseFuture);
    when(responseFuture.get()).thenThrow(exception, exception).thenReturn(response1);

    HttpClientResponse response =
        awsHttpClient.executePost(endpointUrl + relativePath, payload, requestHeaders);

    assertThat(response.statusCode()).isEqualTo(HttpStatus.SC_OK);
    assertThat(response.headers()).isEqualTo(headerArrayToMap(responseHeaders));
    assertThat(response.responseBody()).isEmpty();
    verify(httpClient, never()).start();
    verify(httpClient, times(3)).execute(any(), any());
    verify(responseFuture, times(3)).get();
  }

  @Test
  public void executePost_failure_retriesExhausted()
      throws ExecutionException, InterruptedException {
    String payload = "dummy payload string";
    Map<String, String> requestHeaders =
        ImmutableMap.of("reqKey1", "reqVal1", "reqKey2", "reqVal2");
    when(httpClient.execute(any(), any())).thenReturn(responseFuture);
    when(responseFuture.get())
        .thenThrow(new ExecutionException(new IOException("Connection timed out")));

    IOException actualException =
        assertThrows(
            IOException.class,
            () -> awsHttpClient.executePost(endpointUrl + relativePath, payload, requestHeaders));

    IOException expectedException = new IOException("Connection timed out");
    assertThat(expectedException.getMessage()).isEqualTo(actualException.getMessage());
    verify(httpClient, times(3)).execute(any(), any());
    verify(responseFuture, times(3)).get();
  }

  @Test
  public void executePost_throws_someNonRetriableException()
      throws ExecutionException, InterruptedException {
    String payload = "dummy payload string";
    Map<String, String> requestHeaders =
        ImmutableMap.of("reqKey1", "reqVal1", "reqKey2", "reqVal2");
    when(httpClient.execute(any(), any())).thenReturn(responseFuture);
    when(responseFuture.get()).thenThrow(new InterruptedException("some non-retriable exception"));

    RuntimeException actualException =
        assertThrows(
            RuntimeException.class,
            () -> awsHttpClient.executePost(endpointUrl + relativePath, payload, requestHeaders));

    RuntimeException expectedException =
        new RuntimeException("Unexpected exception or error thrown while performing http request.");
    assertThat(expectedException.getMessage()).isEqualTo(actualException.getMessage());
    assertThat(actualException.getCause().getMessage()).isEqualTo("some non-retriable exception");
    verify(httpClient, times(1)).execute(any(), any());
    verify(responseFuture, times(1)).get();
  }

  @Test
  public void startsClientIfNotActive() {
    when(httpClient.getStatus()).thenReturn(IOReactorStatus.INACTIVE);
    doNothing().when(httpClient).start();

    awsHttpClient = new AwsHttpClient(httpClient);

    verify(httpClient, times(1)).start();
  }

  private static Map<String, String> headerArrayToMap(Header... headers) {
    return Stream.of(headers).collect(Collectors.toMap(Header::getName, Header::getValue));
  }
}
