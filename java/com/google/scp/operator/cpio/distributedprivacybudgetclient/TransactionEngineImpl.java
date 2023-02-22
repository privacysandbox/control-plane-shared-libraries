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

import com.google.common.collect.ImmutableList;
import com.google.inject.Inject;
import com.google.scp.coordinator.privacy.budgeting.model.PrivacyBudgetUnit;
import com.google.scp.operator.cpio.distributedprivacybudgetclient.PrivacyBudgetClient.PrivacyBudgetClientException;

/** Transaction engine is responsible for running the transactions. */
public class TransactionEngineImpl implements TransactionEngine {

  private final TransactionPhaseManager transactionPhaseManager;
  private final ImmutableList<PrivacyBudgetClient> privacyBudgetClients;

  /** Creates a new instance of the {@code TransactionEngineImpl} class. */
  @Inject
  public TransactionEngineImpl(
      TransactionPhaseManager transactionPhaseManager,
      ImmutableList<PrivacyBudgetClient> privacyBudgetClients) {
    this.transactionPhaseManager = transactionPhaseManager;
    this.privacyBudgetClients = privacyBudgetClients;
  }

  private Transaction initializeTransaction(TransactionRequest request) {
    Transaction transaction = new Transaction();
    transaction.setId(request.transactionId());
    transaction.setRequest(request);
    return transaction;
  }

  private void proceedToNextPhase(TransactionPhase currentPhase, Transaction transaction)
      throws TransactionEngineException {
    TransactionPhase nextPhase =
        transactionPhaseManager.proceedToNextPhase(
            currentPhase, transaction.getCurrentPhaseExecutionResult());

    if (nextPhase == TransactionPhase.UNKNOWN) {
      if (currentPhase == TransactionPhase.BEGIN) {
        throw new TransactionEngineException(
            StatusCode.TRANSACTION_ENGINE_CANNOT_START_TRANSACTION.name());
      }
      throw new TransactionEngineException(
          StatusCode.TRANSACTION_MANAGER_TRANSACTION_UNKNOWN.name());
    }

    if (nextPhase == TransactionPhase.ABORT) {
      transaction.setTransactionFailed(true);
    }

    // Handle maximum retries for Transaction phase
    if (nextPhase == currentPhase) {
      transaction.setRetries(transaction.getRetries() - 1);
      if (transaction.getRetries() <= 0) {
        throw new TransactionEngineException(
            StatusCode.TRANSACTION_MANAGER_INVALID_MAX_RETRIES_VALUE.name());
      }
    }

    if (transaction.isCurrentPhaseFailed()) {
      // Only change if the current status was false.
      if (!transaction.isTransactionPhaseFailed()) {
        transaction.setTransactionPhaseFailed(true);
        transaction.setTransactionExecutionResult(transaction.getCurrentPhaseExecutionResult());
      }
      // Reset the state before continuing to the next phase.
      transaction.setCurrentPhaseFailed(false);
      transaction.setCurrentPhaseExecutionResult(
          ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    }

    if (transaction.getCurrentPhase() != currentPhase) {
      return;
    }
    transaction.setCurrentPhase(nextPhase);
    executeCurrentPhase(transaction);
  }

  private void executeCurrentPhase(Transaction transaction) throws TransactionEngineException {
    switch (transaction.getCurrentPhase()) {
      case BEGIN:
        executeDistributedPhase(TransactionPhase.BEGIN, transaction);
        return;
      case PREPARE:
        executeDistributedPhase(TransactionPhase.PREPARE, transaction);
        return;
      case COMMIT:
        executeDistributedPhase(TransactionPhase.COMMIT, transaction);
        return;
      case NOTIFY:
        executeDistributedPhase(TransactionPhase.NOTIFY, transaction);
        return;
      case ABORT:
        executeDistributedPhase(TransactionPhase.ABORT, transaction);
        return;
      case END:
        executeDistributedPhase(TransactionPhase.END, transaction);
        return;
      default:
        throw new TransactionEngineException(
            StatusCode.TRANSACTION_MANAGER_INVALID_TRANSACTION_PHASE.name());
    }
  }

  private void executeDistributedPhase(TransactionPhase currentPhase, Transaction transaction)
      throws TransactionEngineException {
    for (PrivacyBudgetClient privacyBudgetClient : this.privacyBudgetClients) {
      String privacyBudgetServerIdentifier = privacyBudgetClient.getPrivacyBudgetServerIdentifier();
      TransactionPhase lastSuccessfulTransactionPhase =
          transaction.getLastCompletedTransactionPhaseOnPrivacyBudgetServer(
              privacyBudgetServerIdentifier);
      if ((lastSuccessfulTransactionPhase == TransactionPhase.NOTSTARTED
              && currentPhase != TransactionPhase.BEGIN)
          || lastSuccessfulTransactionPhase == TransactionPhase.END) {
        continue;
      }
      ExecutionResult executionResult =
          dispatchDistributedCommand(privacyBudgetClient, transaction);
      if (executionResult.executionStatus() != ExecutionStatus.SUCCESS) {
        // Only change if the current status was false.
        if (!transaction.isCurrentPhaseFailed()) {
          transaction.setCurrentPhaseFailed(true);
          transaction.setCurrentPhaseExecutionResult(executionResult);
        }
      } else {
        transaction.setLastCompletedTransactionPhaseOnPrivacyBudgetServer(
            privacyBudgetServerIdentifier, currentPhase);
      }
    }

    // Return if `END` phase has executed successfully and the transaction attempt hasn't failed
    // already
    if (currentPhase == TransactionPhase.END) {
      if (transaction.isTransactionFailed()) {
        throw new TransactionEngineException(
            StatusCode.TRANSACTION_ENGINE_TRANSACTION_FAILED.name());
      }
      return;
    }
    proceedToNextPhase(currentPhase, transaction);
  }

  private ExecutionResult dispatchDistributedCommand(
      PrivacyBudgetClient privacyBudgetClient, Transaction transaction)
      throws TransactionEngineException {
    try {
      switch (transaction.getCurrentPhase()) {
        case BEGIN:
          return privacyBudgetClient.performActionBegin(transaction);
        case PREPARE:
          return privacyBudgetClient.performActionPrepare(transaction);

        case COMMIT:
          return privacyBudgetClient.performActionCommit(transaction);

        case NOTIFY:
          return privacyBudgetClient.performActionNotify(transaction);

        case ABORT:
          return privacyBudgetClient.performActionAbort(transaction);

        case END:
          return privacyBudgetClient.performActionEnd(transaction);

        default:
          throw new TransactionEngineException(
              StatusCode.TRANSACTION_MANAGER_INVALID_TRANSACTION_PHASE.name());
      }
    } catch (PrivacyBudgetClientException e) {
      throw new TransactionEngineException(e.getMessage());
    }
  }

  /**
   * @param request Transaction request metadata.
   */
  @Override
  public ImmutableList<PrivacyBudgetUnit> execute(TransactionRequest request)
      throws TransactionEngineException {
    Transaction transaction = initializeTransaction(request);
    proceedToNextPhase(TransactionPhase.NOTSTARTED, transaction);
    return transaction.getExhaustedPrivacyBudgetUnits();
  }

  /**
   * @param transactionPhaseRequest Transaction phase request metadata.
   */
  @Override
  public void executePhase(TransactionPhaseRequest transactionPhaseRequest)
      throws TransactionEngineException {}
}
