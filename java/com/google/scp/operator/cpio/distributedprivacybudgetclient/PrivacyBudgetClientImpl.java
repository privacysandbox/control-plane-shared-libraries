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

import static com.google.common.collect.ImmutableList.toImmutableList;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.scp.coordinator.privacy.budgeting.model.PrivacyBudgetUnit;
import com.google.scp.shared.api.util.HttpClientResponse;
import com.google.scp.shared.api.util.HttpClientWithInterceptor;
import com.google.scp.shared.mapper.TimeObjectMapper;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import org.apache.hc.core5.http.HttpStatus;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * This class responsible for making API requests to a given Pbs coordinator based on the
 * transaction details
 */
public final class PrivacyBudgetClientImpl implements PrivacyBudgetClient {

  private static final Logger logger = LoggerFactory.getLogger(PrivacyBudgetClientImpl.class);

  private static final String beginTransactionPath = "/transactions:begin";
  private static final String prepareTransactionPath = "/transactions:prepare";
  private static final String commitTransactionPath = "/transactions:commit";
  private static final String notifyTransactionPath = "/transactions:notify";
  private static final String abortTransactionPath = "/transactions:abort";
  private static final String endTransactionPath = "/transactions:end";
  private static final String transactionStatusPath = "/transactions:status";
  private static final String transactionIdHeaderKey = "x-gscp-transaction-id";
  private static final String transactionLastExecTimestampHeaderKey =
      "x-gscp-transaction-last-execution-timestamp";
  private static final String transactionSecretHeaderKey = "x-gscp-transaction-secret";
  private static final String claimedIdentityHeaderKey = "x-gscp-claimed-identity";
  private static final ObjectMapper mapper = new TimeObjectMapper();

  private final HttpClientWithInterceptor httpClient;
  private final String baseUrl;

  public PrivacyBudgetClientImpl(HttpClientWithInterceptor httpClient, String baseUrl) {
    this.httpClient = httpClient;
    this.baseUrl = baseUrl;
  }

  /**
   * Performs request against the provided baseUrl as part of BEGIN phase of transaction. Updates
   * the transaction's lastExecutionTimestamp field in case of successful operation
   *
   * @param transaction -The transaction which the action pertains to
   * @return ExecutionResult The execution result of the operation.
   */
  @Override
  public ExecutionResult performActionBegin(Transaction transaction)
      throws PrivacyBudgetClientException {
    return performTransactionPhaseAction(beginTransactionPath, transaction);
  }

  /**
   * Performs request against the provided baseUrl as part of PREPARE phase of transaction. Updates
   * the transaction's lastExecutionTimestamp field in case of successful operation
   *
   * @param transaction -The transaction which the action pertains to
   * @return ExecutionResult The execution result of the operation.
   */
  @Override
  public ExecutionResult performActionPrepare(Transaction transaction)
      throws PrivacyBudgetClientException {
    return performTransactionPhaseAction(prepareTransactionPath, transaction);
  }

  /**
   * Performs request against the provided baseUrl as part of COMMIT phase of transaction. Updates
   * the transaction's lastExecutionTimestamp field in case of successful operation
   *
   * @param transaction -The transaction which the action pertains to
   * @return ExecutionResult The execution result of the operation.
   */
  @Override
  public ExecutionResult performActionCommit(Transaction transaction)
      throws PrivacyBudgetClientException {
    return performTransactionPhaseAction(commitTransactionPath, transaction);
  }

  /**
   * Performs request against the provided baseUrl as part of NOTIFY phase of transaction. Updates
   * the transaction's lastExecutionTimestamp field in case of successful operation
   *
   * @param transaction -The transaction which the action pertains to
   * @return ExecutionResult The execution result of the operation.
   */
  @Override
  public ExecutionResult performActionNotify(Transaction transaction)
      throws PrivacyBudgetClientException {
    return performTransactionPhaseAction(notifyTransactionPath, transaction);
  }

  /**
   * Performs request against the provided baseUrl as part of END phase of transaction. Updates the
   * transaction's lastExecutionTimestamp field in case of successful operation
   *
   * @param transaction -The transaction which the action pertains to
   * @return ExecutionResult The execution result of the operation.
   */
  @Override
  public ExecutionResult performActionEnd(Transaction transaction)
      throws PrivacyBudgetClientException {
    return performTransactionPhaseAction(endTransactionPath, transaction);
  }

  /**
   * Performs request against the provided baseUrl as part of ABORT phase of transaction. Updates
   * the transaction's lastExecutionTimestamp field in case of successful operation
   *
   * @param transaction -The transaction which the action pertains to
   * @return ExecutionResult The execution result of the operation.
   */
  @Override
  public ExecutionResult performActionAbort(Transaction transaction)
      throws PrivacyBudgetClientException {
    return performTransactionPhaseAction(abortTransactionPath, transaction);
  }

  @Override
  public String getPrivacyBudgetServerIdentifier() {
    return baseUrl;
  }

  private void updateTransactionState(Transaction transaction, HttpClientResponse response) {
    if (response.statusCode() == 200) {
      String lastExecTimestamp = response.headers().get(transactionLastExecTimestampHeaderKey);
      transaction.setLastExecutionTimestamp(baseUrl, lastExecTimestamp);
    }
  }

  private TransactionStatusResponse fetchTransactionStatus(Transaction transaction)
      throws IOException {
    final String transactionId = transaction.getId().toString();
    ImmutableMap<String, String> headers =
        new ImmutableMap.Builder<String, String>()
            .put(transactionIdHeaderKey, transactionId.toUpperCase())
            .put(transactionSecretHeaderKey, transaction.getRequest().transactionSecret())
            .put(claimedIdentityHeaderKey, transaction.getRequest().attributionReportTo())
            .build();
    logger.info("[{}] Making GET request to {}", transactionId, baseUrl + transactionStatusPath);
    var response = httpClient.executeGet(baseUrl + transactionStatusPath, headers);
    logger.info("[{}] GET request response: " + response, transactionId);
    return generateTransactionStatus(response);
  }

  private Map<String, String> getTransactionPhaseRequestHeaders(Transaction transaction) {
    ImmutableMap.Builder<String, String> mapBuilder = ImmutableMap.builder();
    mapBuilder.put(transactionIdHeaderKey, transaction.getId().toString().toUpperCase());
    mapBuilder.put(transactionSecretHeaderKey, transaction.getRequest().transactionSecret());
    mapBuilder.put(claimedIdentityHeaderKey, transaction.getRequest().attributionReportTo());
    if (!transaction.getCurrentPhase().equals(TransactionPhase.BEGIN)) {
      String lastExecTimestamp = transaction.getLastExecutionTimestamp(baseUrl);
      mapBuilder.put(transactionLastExecTimestampHeaderKey, lastExecTimestamp);
    }
    return mapBuilder.build();
  }

  private String generatePayload(Transaction transaction) throws JsonProcessingException {
    Map<String, Object> transactionData = new HashMap<>();
    transactionData.put("v", "1.0");
    TransactionRequest transactionRequest = transaction.getRequest();
    List<Map<String, Object>> budgets = new ArrayList<>();
    for (PrivacyBudgetUnit budgetUnit : transactionRequest.privacyBudgetUnits()) {
      Map<String, Object> budgetMap = new HashMap<>();
      budgetMap.put("key", budgetUnit.privacyBudgetKey());
      budgetMap.put("token", transactionRequest.privacyBudgetLimit());
      budgetMap.put("reporting_time", budgetUnit.reportingWindow().toString());
      budgets.add(budgetMap);
    }
    transactionData.put("t", budgets);
    return mapper.writeValueAsString(transactionData);
  }

  private ExecutionResult generateExecutionResult(
      Transaction transaction, HttpClientResponse response) throws PrivacyBudgetClientException {
    int statusCode = response.statusCode();
    if (statusCode == HttpStatus.SC_OK) {
      return ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK);
    }
    if (ImmutableList.of(HttpStatus.SC_CLIENT_ERROR, HttpStatus.SC_PRECONDITION_FAILED)
        .contains(statusCode)) {
      // StatusCode 412 - Returned when last execution timestamp sent by the client does not match
      // what server has
      // StatusCode 400 - One of the reasons it can be returned is if client is trying to execute a
      // phase that is different from what server is expecting
      return getExecutionResultBasedOnTransactionStatus(transaction, statusCode);
    }
    if (statusCode == HttpStatus.SC_CONFLICT) {
      // StatusCode 409 is returned when certain keys have their budgets exhausted.
      try {
        processBudgetExhaustedResponse(response, transaction);
      } catch (JsonProcessingException e) {
        throw new PrivacyBudgetClientException(e.getMessage());
      }
      return ExecutionResult.create(ExecutionStatus.FAILURE, StatusCode.UNKNOWN);
    }
    if (statusCode < HttpStatus.SC_SERVER_ERROR || statusCode > 599) {
      return ExecutionResult.create(ExecutionStatus.FAILURE, StatusCode.UNKNOWN);
    }
    return ExecutionResult.create(ExecutionStatus.RETRY, StatusCode.UNKNOWN);
  }

  private void processBudgetExhaustedResponse(HttpClientResponse response, Transaction transaction)
      throws JsonProcessingException {
    BudgetExhaustedResponse budgetExhaustedResponse =
        mapper.readValue(response.responseBody(), BudgetExhaustedResponse.class);
    ImmutableList<PrivacyBudgetUnit> privacyBudgetUnits =
        transaction.getRequest().privacyBudgetUnits();
    ImmutableList<PrivacyBudgetUnit> exhaustedPrivacyBudgetUnits =
        budgetExhaustedResponse.budgetExhaustedIndices().stream()
            .map(index -> privacyBudgetUnits.get(index))
            .collect(toImmutableList());
    transaction.setExhaustedPrivacyBudgetUnits(exhaustedPrivacyBudgetUnits);
  }

  /**
   * Returns a status of {@link ExecutionStatus.FAILURE} if the transaction's status is unable to be
   * fetched.
   *
   * <p>Returns a status of {@link ExecutionStatus.RETRY} if either:
   *
   * <ul>
   *   <li>A status of transaction phase failure is received
   *   <li>the transaction phase returned matches the current transaction phase. Updates
   *       lastExecutionTimestamp based on received status.
   * </ul>
   *
   * <p>Returns a status of {@link Execution.SUCCESS} if both:
   *
   * <ul>
   *   <li>The status received is not "failure"
   *   <li>The transaction phase returned does not match the current transaction phase. Updates
   *       lastExecutionTimestamp based on received status
   * </ul>
   *
   * @param transaction - Transaction whose status needs to be checked against the given coordinator
   * @param actionHttpStatusCode - The Http status code returned by the transaction phase request
   * @throws PrivacyBudgetClientException - in case the status received is transaction expired or
   *     the transaction phase between client and server are out sync by more than 1 phase
   */
  private ExecutionResult getExecutionResultBasedOnTransactionStatus(
      Transaction transaction, int actionHttpStatusCode) throws PrivacyBudgetClientException {
    TransactionStatusResponse transactionStatusResponse = null;
    try {
      transactionStatusResponse = fetchTransactionStatus(transaction);
    } catch (Exception e) {
      logger.error(
          "[{}] Failed to fetch transaction status. Error is: ",
          transaction.getId(),
          e.getMessage());
      return ExecutionResult.create(ExecutionStatus.FAILURE, StatusCode.UNKNOWN);
    }
    if (transactionStatusResponse.isExpired()) {
      throw new PrivacyBudgetClientException(
          StatusCode.TRANSACTION_ENGINE_TRANSACTION_IS_EXPIRED.name());
    }
    transaction.setLastExecutionTimestamp(
        baseUrl, transactionStatusResponse.lastExecutionTimestamp());
    if (transactionStatusResponse.hasFailed()) {
      return ExecutionResult.create(ExecutionStatus.RETRY, StatusCode.UNKNOWN);
    }
    if (actionHttpStatusCode == HttpStatus.SC_CLIENT_ERROR) {
      /**
       * It is ok to mark SUCCESS in response to a 400 only if the server has moved ahead of the
       * client in its understanding of the current transaction phase. In all other cases status
       * code of 400 means that a non-retriable error has occurred
       */
      int serverPhaseValue = transactionStatusResponse.currentPhase().getNumValue();
      int clientPhaseValue = transaction.getCurrentPhase().getNumValue();
      int phaseDiff = serverPhaseValue - clientPhaseValue;
      switch (phaseDiff) {
        case 1: // Server is ahead of client by 1 phase
          return ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK);
        case 0: // The cause of the 400 is not due to phase differences.
          return ExecutionResult.create(ExecutionStatus.FAILURE, StatusCode.UNKNOWN);
        default: // serverPhaseValue > clientPhaseValue + 1 || serverPhaseValue < clientPhaseValue.
          throw new PrivacyBudgetClientException(
              String.format(
                  "The PrivacyBudget client and server phases are out of sync. server phase numeric"
                      + " value: %d. client phase numeric value; %d. Transaction cannot be"
                      + " completed",
                  serverPhaseValue, clientPhaseValue));
      }
    }
    return ExecutionResult.create(ExecutionStatus.FAILURE, StatusCode.UNKNOWN);
  }

  private TransactionStatusResponse generateTransactionStatus(HttpClientResponse response)
      throws IOException {
    int statusCode = response.statusCode();
    if (statusCode == 200) {
      return mapper.readValue(response.responseBody(), TransactionStatusResponse.class);
    } else {
      throw new IOException(
          String.format("Error retrieving transaction status. Error details: %s", response));
    }
  }

  private ExecutionResult performTransactionPhaseAction(
      String relativePath, Transaction transaction) throws PrivacyBudgetClientException {
    try {
      final String transactionId = transaction.getId().toString();
      String payload = generatePayload(transaction);
      logger.info("[{}] Making POST request to {}", transactionId, baseUrl + relativePath);
      var response =
          httpClient.executePost(
              baseUrl + relativePath, payload, getTransactionPhaseRequestHeaders(transaction));
      logger.info("[{}] POST request response: " + response, transactionId);
      updateTransactionState(transaction, response);
      return generateExecutionResult(transaction, response);
    } catch (JsonProcessingException e) {
      logger.error("Serialization error", e);
      return ExecutionResult.create(ExecutionStatus.RETRY, StatusCode.MALFORMED_DATA);
    } catch (IOException e) {
      logger.error("Error performing service call", e);
      return ExecutionResult.create(ExecutionStatus.RETRY, StatusCode.UNKNOWN);
    }
  }
}
