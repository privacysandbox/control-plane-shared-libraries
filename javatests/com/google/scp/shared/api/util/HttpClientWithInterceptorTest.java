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

package com.google.scp.shared.api.util;

import static com.google.common.truth.Truth.assertThat;
import static org.junit.Assert.assertThrows;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import com.google.common.collect.ImmutableMap;
import io.github.resilience4j.core.IntervalFunction;
import io.github.resilience4j.retry.RetryConfig;
import io.github.resilience4j.retry.RetryRegistry;
import java.io.IOException;
import java.time.Duration;
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
public final class HttpClientWithInterceptorTest {

  @Rule public final MockitoRule mockito = MockitoJUnit.rule();

  @Mock CloseableHttpAsyncClient httpClient;
  @Mock Future<SimpleHttpResponse> responseFuture;
  private final String endpointUrl = "https://www.google.com/";
  private final String relativePath = "phasePath";

  private HttpClientWithInterceptor httpClientWithInterceptor;
  private Header[] responseHeaders;

  private final RetryRegistry retryRegistry =
      RetryRegistry.of(
          RetryConfig.custom()
              .maxAttempts(4)
              /*
               * Retries 3 times (in addition to initial attempt) with initial interval of 1s between calls.
               * Backoff interval increases exponentially between retries viz. 1s, 2s, 4s respectively
               */
              .intervalFunction(
                  IntervalFunction.ofExponentialBackoff(
                      Duration.ofMillis(500), 2, Duration.ofMillis(2000)))
              .retryExceptions(Exception.class)
              .build());

  @Before
  public void setup() throws IOException {
    when(httpClient.getStatus()).thenReturn(IOReactorStatus.ACTIVE);
    httpClientWithInterceptor = new HttpClientWithInterceptor(httpClient, retryRegistry);
    this.responseHeaders =
        new BasicHeader[] {
          new BasicHeader("resKey1", "resVal1"), new BasicHeader("resKey2", "resVal2")
        };
  }

  @Test
  public void executeGet_success() throws Exception {
    SimpleHttpResponse response1 = SimpleHttpResponse.create(HttpStatus.SC_OK, "");
    response1.setHeaders(responseHeaders);
    Map<String, String> requestHeaders =
        ImmutableMap.of("reqKey1", "reqVal1", "reqKey2", "reqVal2");
    when(httpClient.execute(any(), any())).thenReturn(responseFuture);
    when(responseFuture.get()).thenReturn(response1);

    HttpClientResponse response =
        httpClientWithInterceptor.executeGet(endpointUrl + relativePath, requestHeaders);

    assertThat(response.statusCode()).isEqualTo(HttpStatus.SC_OK);
    assertThat(response.headers()).isEqualTo(headerArrayToMap(responseHeaders));
    assertThat(response.responseBody()).isEmpty();
    verify(httpClient, never()).start();
    verify(httpClient, times(1)).execute(any(), any());
    verify(responseFuture, times(1)).get();
  }

  @Test
  public void executePost_success() throws Exception {
    String payload = "dummy payload string";
    SimpleHttpResponse response1 = SimpleHttpResponse.create(HttpStatus.SC_OK, "");
    response1.setHeaders(responseHeaders);
    Map<String, String> requestHeaders =
        ImmutableMap.of("reqKey1", "reqVal1", "reqKey2", "reqVal2");
    when(httpClient.execute(any(), any())).thenReturn(responseFuture);
    when(responseFuture.get()).thenReturn(response1);

    HttpClientResponse response =
        httpClientWithInterceptor.executePost(endpointUrl + relativePath, payload, requestHeaders);

    assertThat(response.statusCode()).isEqualTo(HttpStatus.SC_OK);
    assertThat(response.headers()).isEqualTo(headerArrayToMap(responseHeaders));
    assertThat(response.responseBody()).isEmpty();
    verify(httpClient, never()).start();
    verify(httpClient, times(1)).execute(any(), any());
    verify(responseFuture, times(1)).get();
  }

  @Test
  public void executePost_success_afterRetry() throws Exception {
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
        httpClientWithInterceptor.executePost(endpointUrl + relativePath, payload, requestHeaders);

    assertThat(response.statusCode()).isEqualTo(HttpStatus.SC_OK);
    assertThat(response.headers()).isEqualTo(headerArrayToMap(responseHeaders));
    assertThat(response.responseBody()).isEmpty();
    verify(httpClient, never()).start();
    verify(httpClient, times(3)).execute(any(), any());
    verify(responseFuture, times(3)).get();
  }

  @Test
  public void executePost_failure_retriesExhausted_checkedException()
      throws ExecutionException, InterruptedException {
    String payload = "dummy payload string";
    Map<String, String> requestHeaders =
        ImmutableMap.of("reqKey1", "reqVal1", "reqKey2", "reqVal2");
    when(httpClient.execute(any(), any())).thenReturn(responseFuture);
    when(responseFuture.get())
        .thenThrow(new ExecutionException(new RuntimeException("Connection timed out")));

    IOException actualException =
        assertThrows(
            IOException.class,
            () ->
                httpClientWithInterceptor.executePost(
                    endpointUrl + relativePath, payload, requestHeaders));

    IOException expectedException =
        new IOException(
            "java.util.concurrent.ExecutionException: java.lang.RuntimeException: Connection timed"
                + " out");
    assertThat(actualException.getMessage()).isEqualTo(expectedException.getMessage());
    verify(httpClient, times(4)).execute(any(), any());
    verify(responseFuture, times(4)).get();
  }

  @Test
  public void executePost_failure_retriesExhausted_runtimeException()
      throws ExecutionException, InterruptedException {
    String payload = "dummy payload string";
    Map<String, String> requestHeaders =
        ImmutableMap.of("reqKey1", "reqVal1", "reqKey2", "reqVal2");
    when(httpClient.execute(any(), any())).thenReturn(responseFuture);
    when(responseFuture.get()).thenThrow(new RuntimeException("Connection timed out"));

    IOException actualException =
        assertThrows(
            IOException.class,
            () ->
                httpClientWithInterceptor.executePost(
                    endpointUrl + relativePath, payload, requestHeaders));

    IOException expectedException =
        new IOException("java.lang.RuntimeException: Connection timed out");
    assertThat(actualException.getMessage()).isEqualTo(expectedException.getMessage());
    verify(httpClient, times(4)).execute(any(), any());
    verify(responseFuture, times(4)).get();
  }

  @Test
  public void startsClientIfNotActive() {
    when(httpClient.getStatus()).thenReturn(IOReactorStatus.INACTIVE);
    doNothing().when(httpClient).start();

    httpClientWithInterceptor = new HttpClientWithInterceptor(httpClient, retryRegistry);

    verify(httpClient, times(1)).start();
  }

  private static Map<String, String> headerArrayToMap(Header... headers) {
    return Stream.of(headers).collect(Collectors.toMap(Header::getName, Header::getValue));
  }
}
