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

import com.google.common.annotations.VisibleForTesting;
import com.google.inject.Inject;
import io.github.resilience4j.core.IntervalFunction;
import io.github.resilience4j.retry.Retry;
import io.github.resilience4j.retry.RetryConfig;
import io.github.resilience4j.retry.RetryRegistry;
import java.io.IOException;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.Map;
import java.util.concurrent.ExecutionException;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.apache.hc.client5.http.async.methods.SimpleHttpRequest;
import org.apache.hc.client5.http.async.methods.SimpleHttpResponse;
import org.apache.hc.client5.http.config.RequestConfig;
import org.apache.hc.client5.http.impl.async.CloseableHttpAsyncClient;
import org.apache.hc.client5.http.impl.async.HttpAsyncClients;
import org.apache.hc.core5.http.ContentType;
import org.apache.hc.core5.http.Header;
import org.apache.hc.core5.http.HttpRequestInterceptor;
import org.apache.hc.core5.http.Method;
import org.apache.hc.core5.reactor.IOReactorStatus;
import org.apache.hc.core5.util.Timeout;

/**
 * This is a wrapper on top the bare-bones Apache HttpClient 5.0. Every request executed by the
 * HttpClient in this wrapper is subject to be intercepted by an HttpRequestInterceptor.
 */
public class HttpClientWithInterceptor {
  private static final RetryRegistry RETRY_REGISTRY =
      RetryRegistry.of(
          RetryConfig.custom()
              .maxAttempts(3)
              /*
               * Retries 3 times (in addition to initial attempt) with initial interval of 500ms between calls.
               * Backoff interval increases exponentially between retries viz. 500ms, 1000ms, 2000ms respectively
               */
              .intervalFunction(IntervalFunction.ofExponentialBackoff(Duration.ofMillis(500), 2))
              .retryExceptions(IOException.class)
              .build());
  private static final long REQUEST_TIMEOUT_DURATION = Duration.ofSeconds(60).toMillis();
  private static final RequestConfig REQUEST_CONFIG =
      RequestConfig.custom()
          // Timeout for requesting a connection from internal connection manager
          .setConnectionRequestTimeout(Timeout.ofMilliseconds(REQUEST_TIMEOUT_DURATION))
          // Timeout for establishing a request to host
          .setConnectTimeout(Timeout.ofMilliseconds(REQUEST_TIMEOUT_DURATION))
          // Timeout for waiting to get a response from the server
          .setResponseTimeout(Timeout.ofMilliseconds(REQUEST_TIMEOUT_DURATION))
          .build();

  private final CloseableHttpAsyncClient httpClient;
  private final Retry retryConfig;

  @Inject
  public HttpClientWithInterceptor(HttpRequestInterceptor authTokenInterceptor) {
    this.httpClient =
        HttpAsyncClients.customHttp2().addRequestInterceptorFirst(authTokenInterceptor).build();
    if (!IOReactorStatus.ACTIVE.equals(httpClient.getStatus())) {
      httpClient.start();
    }
    this.retryConfig = RETRY_REGISTRY.retry("awsHttpClientRetryConfig");
  }

  /**
   * This constructor is reserved specifically for unit tests pertaining to this class. This
   * constructor will not setup the request interceptor in the httpclient, thus rendering the object
   * useless for actual client use-cases outside of unit testing
   *
   * @param httpClient
   */
  @VisibleForTesting
  HttpClientWithInterceptor(CloseableHttpAsyncClient httpClient) {
    this.httpClient = httpClient;
    if (!IOReactorStatus.ACTIVE.equals(httpClient.getStatus())) {
      httpClient.start();
    }
    this.retryConfig = RETRY_REGISTRY.retry("awsHttpClientRetryConfig");
  }

  /**
   * @param endpointUrl - complete request url (authority + relative path) Example:
   *     https://client.example.com/v1/getExamples
   * @param headers - Any headers that need to be added to the request
   * @throws IOException in case of any issues executing the HTTP request. Example includes but is
   *     not limited to network connection issues
   */
  public HttpClientResponse executeGet(String endpointUrl, Map<String, String> headers)
      throws IOException {
    URI endpointUri = URI.create(endpointUrl);

    SimpleHttpRequest httpRequest = SimpleHttpRequest.create(Method.GET, endpointUri);
    httpRequest.setConfig(REQUEST_CONFIG);
    headers.forEach(httpRequest::setHeader);
    SimpleHttpResponse response = executeRequest(httpRequest);
    return HttpClientResponse.create(
        response.getCode(), response.getBodyText(), parseResponseHeaders(response.getHeaders()));
  }

  /**
   * @param endpointUrl - complete request url (authority + relative path). Example:
   *     https://client.example.com/v1/getExamples
   * @param payload - Request Body. The body needs to be of type application/json
   * @param headers - Any headers that need to be added to the request
   * @throws IOException in case of any issues executing the HTTP request. Example includes but is
   *     not limited to network connection issues
   */
  public HttpClientResponse executePost(
      String endpointUrl, String payload, Map<String, String> headers) throws IOException {
    URI endpointUri = URI.create(endpointUrl);

    SimpleHttpRequest httpRequest = SimpleHttpRequest.create(Method.POST, endpointUri);
    httpRequest.setConfig(REQUEST_CONFIG);
    headers.forEach(httpRequest::setHeader);
    httpRequest.setHeader("content-length", payload.getBytes(StandardCharsets.UTF_8).length);
    httpRequest.setBody(payload, ContentType.APPLICATION_JSON);
    SimpleHttpResponse response = executeRequest(httpRequest);
    return HttpClientResponse.create(
        response.getCode(), response.getBodyText(), parseResponseHeaders(response.getHeaders()));
  }

  private SimpleHttpResponse executeRequest(SimpleHttpRequest httpRequest) throws IOException {
    try {
      return Retry.decorateCheckedSupplier(
              retryConfig,
              () -> {
                try {
                  return httpClient.execute(httpRequest, null).get();
                } catch (InterruptedException | ExecutionException e) {
                  /*
                   * Any IOException occurring as a result of the operation is wrapped inside an ExecutionException
                   * This ExecutionException gets thrown by the above Future's `get()` method.
                   * We want to retry the request only if the underlying cause of the ExecutionException was an IOException.
                   * In such cases we throw the underlying IOException from this supplier and the
                   * retryconfig is configured to retry on IOExceptions coming out of the supplier.
                   */
                  if (e.getCause() != null && e.getCause() instanceof IOException) {
                    throw e.getCause();
                  }
                  throw e;
                }
              })
          .apply();
    } catch (Throwable e) {
      /*
       * This throwable could either be an IOException which was bubbled up because retries were exhausted or it could be something we did not expect.
       * If it was an IOException we want to propagate it as such. PrivacyBudgetClientImpl and TransactionEngineImpl
       * make transaction level retry decisions based on this IOException. If it was anything other than IOException we wrap it as a RuntimeException and propagate it
       */
      if (e instanceof IOException) {
        throw ((IOException) e);
      }
      // let the unknown throwable bubble up as a RuntimeException as it is not expected to be
      // handled
      throw new RuntimeException(
          "Unexpected exception or error thrown while performing http request.", e);
    }
  }

  private static Map<String, String> parseResponseHeaders(Header... headers) {
    return Stream.of(headers).collect(Collectors.toMap(Header::getName, Header::getValue));
  }
}
