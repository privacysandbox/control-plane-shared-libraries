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

package com.google.scp.operator.cpio.distributedprivacybudgetclient;

import static com.google.common.truth.Truth.assertThat;
import static org.junit.Assert.assertThrows;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.scp.coordinator.privacy.budgeting.model.PrivacyBudgetUnit;
import com.google.scp.operator.cpio.distributedprivacybudgetclient.PrivacyBudgetClient.PrivacyBudgetClientException;
import com.google.scp.shared.api.util.HttpClientResponse;
import com.google.scp.shared.api.util.HttpClientWithInterceptor;
import java.io.IOException;
import java.sql.Timestamp;
import java.time.Instant;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;

public final class PrivacyBudgetClientTest {
  @Rule public final MockitoRule mockito = MockitoJUnit.rule();

  @Mock private HttpClientWithInterceptor awsHttpClient;

  private final UUID transactionId = UUID.randomUUID();
  private final String endpoint = "http://www.google.com/v1";

  private final String beginPhaseUri = endpoint + "/transactions:begin";
  private final String preparePhaseUri = endpoint + "/transactions:prepare";
  private final String notifyPhaseUri = endpoint + "/transactions:notify";
  private final String commitPhaseUri = endpoint + "/transactions:commit";
  private final String endPhaseUri = endpoint + "/transactions:end";
  private final String abortPhaseUri = endpoint + "/transactions:abort";
  private final String transactionStatusUri = endpoint + "/transactions:status";

  private TransactionRequest transactionRequest;
  private HttpClientResponse successResponse;
  private HttpClientResponse retryableFailureResponse;
  private HttpClientResponse nonRetryableFailureResponse;
  private HttpClientResponse preconditionNotMetFailureResponse;
  private HttpClientResponse badRequestFailureResponse;
  private HttpClientResponse budgetExhaustedResponse;
  private Transaction transaction;

  private PrivacyBudgetClientImpl privacyBudgetClient;

  @Before
  public void setup() {
    this.privacyBudgetClient = new PrivacyBudgetClientImpl(awsHttpClient, endpoint);
    transactionRequest = generateTransactionRequest();
    Map<String, String> responseHeaders =
        ImmutableMap.of(
            "x-gscp-transaction-last-execution-timestamp",
            String.valueOf(Instant.now().toEpochMilli()));
    successResponse = HttpClientResponse.create(200, "", responseHeaders);
    retryableFailureResponse = HttpClientResponse.create(500, "", Collections.emptyMap());
    nonRetryableFailureResponse = HttpClientResponse.create(400, "", Collections.emptyMap());
    preconditionNotMetFailureResponse = HttpClientResponse.create(412, "", Collections.emptyMap());
    badRequestFailureResponse = HttpClientResponse.create(400, "", Collections.emptyMap());
    budgetExhaustedResponse =
        HttpClientResponse.create(409, "{\"f\":[0],\"v\":\"1.0\"}", Collections.emptyMap());
  }

  @Test
  public void performAction_begin_success() throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.BEGIN, transactionRequest);
    when(awsHttpClient.executePost(
            eq(beginPhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(successResponse);

    ExecutionResult executionResult = privacyBudgetClient.performActionBegin(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.SUCCESS);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.OK);
  }

  @Test
  public void performAction_begin_retriableFailure()
      throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.BEGIN, transactionRequest);
    when(awsHttpClient.executePost(
            eq(beginPhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(retryableFailureResponse);

    ExecutionResult executionResult = privacyBudgetClient.performActionBegin(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.RETRY);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.UNKNOWN);
  }

  @Test
  public void performAction_begin_nonRetriableFailure()
      throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.BEGIN, transactionRequest);
    when(awsHttpClient.executePost(
            eq(beginPhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(nonRetryableFailureResponse);
    long lastExecTimeStamp = Instant.now().toEpochMilli();
    Map<String, String> responseHeaders =
        ImmutableMap.of(
            "x-gscp-transaction-last-execution-timestamp", String.valueOf(lastExecTimeStamp));
    HttpClientResponse statusResponse =
        HttpClientResponse.create(
            200,
            "{\"has_failures\":false,\"is_expired\":false,\"last_execution_timestamp\":1682345560947309"
                + ",\"transaction_execution_phase\":\"BEGIN\"}",
            responseHeaders);
    Map<String, String> statusRequestExpectedHeadersMap = expectedHeadersMap(endpoint, transaction);
    statusRequestExpectedHeadersMap.remove("x-gscp-transaction-last-execution-timestamp");
    when(awsHttpClient.executeGet(eq(transactionStatusUri), eq(statusRequestExpectedHeadersMap)))
        .thenReturn(statusResponse);

    ExecutionResult executionResult = privacyBudgetClient.performActionBegin(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.FAILURE);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.UNKNOWN);
  }

  @Test
  public void performAction_begin_IOException() throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.BEGIN, transactionRequest);
    when(awsHttpClient.executePost(
            eq(beginPhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenThrow(new IOException());

    ExecutionResult executionResult = privacyBudgetClient.performActionBegin(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.RETRY);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.UNKNOWN);
  }

  @Test
  public void performAction_prepare_success() throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.PREPARE, transactionRequest);
    when(awsHttpClient.executePost(
            eq(preparePhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(successResponse);

    ExecutionResult executionResult = privacyBudgetClient.performActionPrepare(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.SUCCESS);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.OK);
  }

  @Test
  public void performAction_commit_success() throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.COMMIT, transactionRequest);
    when(awsHttpClient.executePost(
            eq(commitPhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(successResponse);

    ExecutionResult executionResult = privacyBudgetClient.performActionCommit(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.SUCCESS);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.OK);
  }

  @Test
  public void performAction_notify_success() throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.NOTIFY, transactionRequest);
    when(awsHttpClient.executePost(
            eq(notifyPhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(successResponse);

    ExecutionResult executionResult = privacyBudgetClient.performActionNotify(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.SUCCESS);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.OK);
  }

  @Test
  public void performAction_abort_success() throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.ABORT, transactionRequest);
    when(awsHttpClient.executePost(
            eq(abortPhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(successResponse);

    ExecutionResult executionResult = privacyBudgetClient.performActionAbort(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.SUCCESS);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.OK);
  }

  @Test
  public void performAction_end_success() throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.END, transactionRequest);
    when(awsHttpClient.executePost(
            eq(endPhaseUri), eq(expectedPayload()), eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(successResponse);

    ExecutionResult executionResult = privacyBudgetClient.performActionEnd(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.SUCCESS);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.OK);
  }

  @Test
  public void performAction_privacyBudgetExhausted()
      throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.PREPARE, transactionRequest);
    long lastExecTimeStamp = Instant.now().toEpochMilli();
    when(awsHttpClient.executePost(
            eq(preparePhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(budgetExhaustedResponse);

    ExecutionResult executionResult = privacyBudgetClient.performActionPrepare(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.FAILURE);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.UNKNOWN);
    assertThat(transaction.getExhaustedPrivacyBudgetUnits())
        .containsExactly(transactionRequest.privacyBudgetUnits().get(0));
  }

  @Test
  public void performAction_begin_preConditionNotMet_statusCheck_phaseFailed()
      throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.BEGIN, transactionRequest);
    long lastExecTimeStamp = Instant.now().toEpochMilli();
    Map<String, String> responseHeaders =
        ImmutableMap.of(
            "x-gscp-transaction-last-execution-timestamp", String.valueOf(lastExecTimeStamp));
    HttpClientResponse statusResponse =
        HttpClientResponse.create(
            200,
            "{\"has_failures\":true,\"is_expired\":false,\"last_execution_timestamp\":1682345560947309"
                + ",\"transaction_execution_phase\":\"PREPARE\"}",
            responseHeaders);
    when(awsHttpClient.executePost(
            eq(beginPhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(preconditionNotMetFailureResponse);
    when(awsHttpClient.executeGet(
            eq(transactionStatusUri), eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(statusResponse);

    ExecutionResult executionResult = privacyBudgetClient.performActionBegin(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.RETRY);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.UNKNOWN);
  }

  @Test
  public void performAction_begin_preConditionNotMet_statusCheckFailed()
      throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.BEGIN, transactionRequest);
    when(awsHttpClient.executePost(
            eq(beginPhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(preconditionNotMetFailureResponse);
    when(awsHttpClient.executeGet(
            eq(transactionStatusUri), eq(expectedHeadersMap(endpoint, transaction))))
        .thenThrow(new IOException("Timeout waiting for reply"));

    ExecutionResult executionResult = privacyBudgetClient.performActionBegin(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.FAILURE);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.UNKNOWN);
  }

  @Test
  public void performAction_begin_preConditionNotMet_statusCheck_non200Response()
      throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.BEGIN, transactionRequest);
    long lastExecTimeStamp = Instant.now().toEpochMilli();
    Map<String, String> responseHeaders =
        ImmutableMap.of(
            "x-gscp-transaction-last-execution-timestamp", String.valueOf(lastExecTimeStamp));
    HttpClientResponse statusResponse = HttpClientResponse.create(400, "", responseHeaders);
    when(awsHttpClient.executePost(
            eq(beginPhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(preconditionNotMetFailureResponse);
    when(awsHttpClient.executeGet(
            eq(transactionStatusUri), eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(statusResponse);

    ExecutionResult executionResult = privacyBudgetClient.performActionBegin(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.FAILURE);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.UNKNOWN);
  }

  @Test
  public void performAction_begin_preConditionNotMet_statusCheck_transactionExpired()
      throws IOException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.BEGIN, transactionRequest);
    long lastExecTimeStamp = Instant.now().toEpochMilli();
    Map<String, String> responseHeaders =
        ImmutableMap.of(
            "x-gscp-transaction-last-execution-timestamp", String.valueOf(lastExecTimeStamp));
    HttpClientResponse statusResponse =
        HttpClientResponse.create(
            200,
            "{\"has_failures\":false,\"is_expired\":true,\"last_execution_timestamp\":1682345560947309"
                + ",\"transaction_execution_phase\":\"PREPARE\"}",
            responseHeaders);
    when(awsHttpClient.executePost(
            eq(beginPhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(preconditionNotMetFailureResponse);
    when(awsHttpClient.executeGet(
            eq(transactionStatusUri), eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(statusResponse);
    PrivacyBudgetClientException expectedException =
        new PrivacyBudgetClientException(
            StatusCode.TRANSACTION_ENGINE_TRANSACTION_IS_EXPIRED.name());

    PrivacyBudgetClientException actualException =
        assertThrows(
            PrivacyBudgetClientException.class,
            () -> privacyBudgetClient.performActionBegin(transaction));

    assertThat(expectedException.getMessage()).isEqualTo(actualException.getMessage());
  }

  @Test
  public void performAction_commit_Response400_statusCheck_phaseSucceeded()
      throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.COMMIT, transactionRequest);
    long lastExecTimeStamp = Instant.now().toEpochMilli();
    Map<String, String> responseHeaders =
        ImmutableMap.of(
            "x-gscp-transaction-last-execution-timestamp", String.valueOf(lastExecTimeStamp));
    HttpClientResponse statusResponse =
        HttpClientResponse.create(
            200,
            "{\"has_failures\":false,\"is_expired\":false,\"last_execution_timestamp\":1682345560947309"
                + ",\"transaction_execution_phase\":\"NOTIFY\"}",
            responseHeaders);
    when(awsHttpClient.executePost(
            eq(commitPhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(badRequestFailureResponse);
    Map<String, String> statusRequestExpectedHeadersMap = expectedHeadersMap(endpoint, transaction);
    statusRequestExpectedHeadersMap.remove("x-gscp-transaction-last-execution-timestamp");
    when(awsHttpClient.executeGet(eq(transactionStatusUri), eq(statusRequestExpectedHeadersMap)))
        .thenReturn(statusResponse);

    ExecutionResult executionResult = privacyBudgetClient.performActionCommit(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.SUCCESS);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.OK);
  }

  @Test
  public void performAction_commit_Response400_statusCheck_phaseFailed()
      throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.COMMIT, transactionRequest);
    long lastExecTimeStamp = Instant.now().toEpochMilli();
    Map<String, String> responseHeaders =
        ImmutableMap.of(
            "x-gscp-transaction-last-execution-timestamp", String.valueOf(lastExecTimeStamp));
    HttpClientResponse statusResponse =
        HttpClientResponse.create(
            200,
            "{\"has_failures\":true,\"is_expired\":false,\"last_execution_timestamp\":1682345560947309"
                + ",\"transaction_execution_phase\":\"COMMIT\"}",
            responseHeaders);
    when(awsHttpClient.executePost(
            eq(commitPhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(badRequestFailureResponse);
    Map<String, String> statusRequestExpectedHeadersMap = expectedHeadersMap(endpoint, transaction);
    statusRequestExpectedHeadersMap.remove("x-gscp-transaction-last-execution-timestamp");
    when(awsHttpClient.executeGet(eq(transactionStatusUri), eq(statusRequestExpectedHeadersMap)))
        .thenReturn(statusResponse);

    ExecutionResult executionResult = privacyBudgetClient.performActionCommit(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.RETRY);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.UNKNOWN);
  }

  @Test
  public void performAction_Prepare_Response400_statusCheck_ServerPhaseAheadBy2()
      throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.PREPARE, transactionRequest);

    long lastExecTimeStamp = Instant.now().toEpochMilli();
    Map<String, String> responseHeaders =
        ImmutableMap.of(
            "x-gscp-transaction-last-execution-timestamp", String.valueOf(lastExecTimeStamp));
    HttpClientResponse statusResponse =
        HttpClientResponse.create(
            200,
            "{\"has_failures\":false,\"is_expired\":false,\"last_execution_timestamp\":1682345560947309"
                + ",\"transaction_execution_phase\":\"NOTIFY\"}",
            responseHeaders);
    when(awsHttpClient.executePost(
            eq(preparePhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(badRequestFailureResponse);
    Map<String, String> statusRequestExpectedHeadersMap = expectedHeadersMap(endpoint, transaction);
    statusRequestExpectedHeadersMap.remove("x-gscp-transaction-last-execution-timestamp");
    when(awsHttpClient.executeGet(eq(transactionStatusUri), eq(statusRequestExpectedHeadersMap)))
        .thenReturn(statusResponse);
    PrivacyBudgetClientException expectedException =
        new PrivacyBudgetClientException(
            "The PrivacyBudget client and server phases are out of sync. server phase numeric"
                + " value: 4. client phase numeric value; 2. Transaction cannot be completed");

    PrivacyBudgetClientException actualException =
        assertThrows(
            PrivacyBudgetClientException.class,
            () -> privacyBudgetClient.performActionPrepare(transaction));

    assertThat(actualException.getMessage()).isEqualTo(expectedException.getMessage());
  }

  @Test
  public void performAction_commit_Response400_statusCheck_statusCheckFailed()
      throws IOException, PrivacyBudgetClientException {
    transaction =
        generateTransaction(endpoint, transactionId, TransactionPhase.COMMIT, transactionRequest);
    long lastExecTimeStamp = Instant.now().toEpochMilli();
    Map<String, String> responseHeaders =
        ImmutableMap.of(
            "x-gscp-transaction-last-execution-timestamp", String.valueOf(lastExecTimeStamp));
    HttpClientResponse statusResponse =
        HttpClientResponse.create(
            200,
            "{\"has_failures\":false,\"is_expired\":false,\"last_execution_timestamp\":1682345560947309"
                + ",\"transaction_execution_phase\":\"NOTIFY\"}",
            responseHeaders);
    when(awsHttpClient.executePost(
            eq(commitPhaseUri),
            eq(expectedPayload()),
            eq(expectedHeadersMap(endpoint, transaction))))
        .thenReturn(badRequestFailureResponse);
    Map<String, String> statusRequestExpectedHeadersMap = expectedHeadersMap(endpoint, transaction);
    statusRequestExpectedHeadersMap.remove("x-gscp-transaction-last-execution-timestamp");
    when(awsHttpClient.executeGet(eq(transactionStatusUri), eq(statusRequestExpectedHeadersMap)))
        .thenThrow(new IOException("Timeout waiting for reply"));

    ExecutionResult executionResult = privacyBudgetClient.performActionCommit(transaction);

    assertThat(executionResult.executionStatus()).isEqualTo(ExecutionStatus.FAILURE);
    assertThat(executionResult.statusCode()).isEqualTo(StatusCode.UNKNOWN);
  }

  private Transaction generateTransaction(
      String endpoint,
      UUID transactionId,
      TransactionPhase currentPhase,
      TransactionRequest transactionRequest) {
    Transaction transaction = new Transaction();
    transaction.setId(transactionId);
    transaction.setCurrentPhase(currentPhase);
    transaction.setRequest(transactionRequest);
    if (currentPhase != TransactionPhase.BEGIN) {
      transaction.setLastExecutionTimestamp(endpoint, String.valueOf(Instant.now().getNano()));
    }
    return transaction;
  }

  private TransactionRequest generateTransactionRequest() {
    final Instant timeInstant1 = Instant.ofEpochMilli(1658960799);
    final Instant timeInstant2 = Instant.ofEpochMilli(1658960845);
    final PrivacyBudgetUnit unit1 =
        PrivacyBudgetUnit.builder()
            .privacyBudgetKey("budgetkey1")
            .reportingWindow(timeInstant1)
            .build();
    final PrivacyBudgetUnit unit2 =
        PrivacyBudgetUnit.builder()
            .privacyBudgetKey("budgetkey2")
            .reportingWindow(timeInstant2)
            .build();
    return TransactionRequest.builder()
        .setTransactionId(transactionId)
        .setPrivacyBudgetUnits(ImmutableList.of(unit1, unit2))
        .setTransactionSecret("transaction-secret")
        .setTimeout(Timestamp.from(Instant.now()))
        .setAttributionReportTo("dummy-reporting-origin")
        .build();
  }

  private String expectedPayload() {
    return "{\"t\":[{\"reporting_time\":\"1970-01-20T04:49:20.799Z\",\"key\":\"budgetkey1\",\"token\":1},"
               + "{\"reporting_time\":\"1970-01-20T04:49:20.845Z\",\"key\":\"budgetkey2\",\"token\":1}],\"v\":\"1.0\"}";
  }

  private Map<String, String> expectedHeadersMap(String endpoint, Transaction transaction) {
    Map<String, String> headers = new HashMap<>();
    headers.put("x-gscp-transaction-id", transaction.getId().toString().toUpperCase());
    headers.put("x-gscp-claimed-identity", "dummy-reporting-origin");
    headers.put("x-gscp-transaction-secret", "transaction-secret");
    if (transaction.getCurrentPhase() != TransactionPhase.BEGIN) {
      headers.put(
          "x-gscp-transaction-last-execution-timestamp",
          transaction.getLastExecutionTimestamp(endpoint));
    }
    return headers;
  }
}
