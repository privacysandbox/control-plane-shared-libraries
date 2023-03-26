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
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import com.google.common.collect.ImmutableList;
import com.google.scp.operator.cpio.distributedprivacybudgetclient.PrivacyBudgetClient.PrivacyBudgetClientException;
import com.google.scp.operator.cpio.distributedprivacybudgetclient.TransactionEngine.TransactionEngineException;
import java.sql.Timestamp;
import java.time.Instant;
import java.util.UUID;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;

@RunWith(JUnit4.class)
public final class TransactionEngineTest {
  @Rule public final MockitoRule mockito = MockitoJUnit.rule();
  @Mock private TransactionPhaseManagerImpl transactionPhaseManager;
  @Mock private PrivacyBudgetClient privacyBudgetClient1;
  @Mock private PrivacyBudgetClient privacyBudgetClient2;

  private TransactionEngineImpl transactionEngine;

  @Before
  public void setUp() {
    transactionEngine =
        new TransactionEngineImpl(
            transactionPhaseManager, ImmutableList.of(privacyBudgetClient1, privacyBudgetClient2));
    when(privacyBudgetClient1.getPrivacyBudgetServerIdentifier()).thenReturn("coordinator 1");
    when(privacyBudgetClient2.getPrivacyBudgetServerIdentifier()).thenReturn("coordinator 2");
  }

  @Test
  public void execute_unknownTransactionPhase() throws PrivacyBudgetClientException {
    when(transactionPhaseManager.proceedToNextPhase(
            any(TransactionPhase.class), any(ExecutionResult.class)))
        .thenReturn(TransactionPhase.UNKNOWN);

    assertThrows(
        TransactionEngineException.class, () -> transactionEngine.execute(getTestRequest()));

    verify(transactionPhaseManager, times(1)).proceedToNextPhase(any(), any());
    verify(privacyBudgetClient1, times(0)).performActionBegin(any(Transaction.class));
  }

  @Test
  public void execute_beginTransactionPhaseExecuted() throws Exception {
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTSTARTED,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.BEGIN);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.BEGIN, ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.END);
    when(privacyBudgetClient1.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));

    transactionEngine.execute(getTestRequest());

    verify(transactionPhaseManager, times(2)).proceedToNextPhase(any(), any());
    verify(privacyBudgetClient1, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionEnd(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionEnd(any(Transaction.class));
  }

  @Test
  public void execute_prepareTransactionPhaseExecuted() throws Exception {
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTSTARTED,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.BEGIN);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.BEGIN, ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.PREPARE);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.PREPARE,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.END);
    when(privacyBudgetClient1.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));

    transactionEngine.execute(getTestRequest());

    verify(transactionPhaseManager, times(3)).proceedToNextPhase(any(), any());
    verify(privacyBudgetClient1, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionBegin(any(Transaction.class));
  }

  @Test
  public void execute_commitTransactionPhaseExecuted() throws Exception {
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTSTARTED,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.BEGIN);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.BEGIN, ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.PREPARE);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.PREPARE,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.COMMIT);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.COMMIT,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.END);
    when(privacyBudgetClient1.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionCommit(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionCommit(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));

    transactionEngine.execute(getTestRequest());

    verify(transactionPhaseManager, times(4)).proceedToNextPhase(any(), any());
    verify(privacyBudgetClient1, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionPrepare(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionPrepare(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionEnd(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionPrepare(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionPrepare(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionEnd(any(Transaction.class));
  }

  @Test
  public void execute_notifyTransactionPhaseExecuted() throws Exception {
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTSTARTED,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.BEGIN);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.BEGIN, ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.PREPARE);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.PREPARE,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.COMMIT);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.COMMIT,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.NOTIFY);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTIFY,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.END);
    when(privacyBudgetClient1.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionCommit(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionNotify(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionCommit(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionNotify(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));

    transactionEngine.execute(getTestRequest());

    verify(transactionPhaseManager, times(5)).proceedToNextPhase(any(), any());
    verify(privacyBudgetClient1, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionPrepare(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionCommit(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionNotify(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionEnd(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionPrepare(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionCommit(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionNotify(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionEnd(any(Transaction.class));
  }

  @Test
  public void execute_beginTransactionPhaseExecuted_Failure() throws Exception {
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTSTARTED,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.BEGIN);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.BEGIN,
            ExecutionResult.create(ExecutionStatus.FAILURE, StatusCode.UNKNOWN)))
        .thenReturn(TransactionPhase.UNKNOWN);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.ABORT, ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.END);
    when(privacyBudgetClient1.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionAbort(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.FAILURE, StatusCode.UNKNOWN));
    when(privacyBudgetClient2.performActionAbort(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));

    assertThrows(
        TransactionEngineException.class, () -> transactionEngine.execute(getTestRequest()));

    verify(transactionPhaseManager, times(2)).proceedToNextPhase(any(), any());
    verify(privacyBudgetClient1, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient1, never()).performActionAbort(any(Transaction.class));
    verify(privacyBudgetClient1, never()).performActionEnd(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient2, never()).performActionAbort(any(Transaction.class));
    verify(privacyBudgetClient2, never()).performActionEnd(any(Transaction.class));
  }

  @Test
  public void execute_beginTransactionPhaseExecuted_Retry() throws Exception {
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTSTARTED,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.BEGIN);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.BEGIN,
            ExecutionResult.create(ExecutionStatus.RETRY, StatusCode.UNKNOWN)))
        .thenReturn(TransactionPhase.BEGIN);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.BEGIN, ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.END);
    when(privacyBudgetClient1.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.RETRY, StatusCode.UNKNOWN))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));

    transactionEngine.execute(getTestRequest());

    verify(transactionPhaseManager, times(3)).proceedToNextPhase(any(), any());
    verify(privacyBudgetClient1, times(2)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionEnd(any(Transaction.class));
    verify(privacyBudgetClient2, times(2)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionEnd(any(Transaction.class));
  }

  @Test
  public void execute_prepareTransactionPhaseExecuted_throwsTransactionEngineException()
      throws PrivacyBudgetClientException {
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTSTARTED,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.BEGIN);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.BEGIN, ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.PREPARE);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.PREPARE,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.UNKNOWN);
    when(privacyBudgetClient1.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));

    assertThrows(
        TransactionEngineException.class, () -> transactionEngine.execute(getTestRequest()));

    verify(transactionPhaseManager, times(3)).proceedToNextPhase(any(), any());
    verify(privacyBudgetClient1, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionPrepare(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionPrepare(any(Transaction.class));
  }

  @Test
  public void execute_commitTransactionPhaseExecuted_throwsTransactionEngineException()
      throws PrivacyBudgetClientException {
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTSTARTED,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.BEGIN);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.BEGIN, ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.PREPARE);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.PREPARE,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.COMMIT);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.COMMIT,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.UNKNOWN);
    when(privacyBudgetClient1.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionCommit(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionCommit(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));

    assertThrows(
        TransactionEngineException.class, () -> transactionEngine.execute(getTestRequest()));

    verify(transactionPhaseManager, times(4)).proceedToNextPhase(any(), any());
    verify(privacyBudgetClient1, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionPrepare(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionCommit(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionPrepare(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionCommit(any(Transaction.class));
  }

  @Test
  public void execute_notifyTransactionPhaseExecuted_throwsTransactionEngineException()
      throws PrivacyBudgetClientException {
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTSTARTED,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.BEGIN);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.BEGIN, ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.PREPARE);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.PREPARE,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.COMMIT);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.COMMIT,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.NOTIFY);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTIFY,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.UNKNOWN);
    when(privacyBudgetClient1.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionCommit(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionNotify(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionCommit(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionNotify(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));

    assertThrows(
        TransactionEngineException.class, () -> transactionEngine.execute(getTestRequest()));

    verify(transactionPhaseManager, times(5)).proceedToNextPhase(any(), any());
    verify(privacyBudgetClient1, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionPrepare(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionCommit(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionNotify(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionPrepare(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionCommit(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionNotify(any(Transaction.class));
  }

  @Test
  public void execute_prepareTransactionPhaseFailed_throwsTransactionEngineException()
      throws PrivacyBudgetClientException {
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTSTARTED,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.BEGIN);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.BEGIN, ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.PREPARE);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.PREPARE,
            ExecutionResult.create(ExecutionStatus.FAILURE, StatusCode.UNKNOWN)))
        .thenReturn(TransactionPhase.ABORT);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.ABORT, ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.END);
    when(privacyBudgetClient1.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.FAILURE, StatusCode.UNKNOWN));
    when(privacyBudgetClient1.performActionAbort(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionAbort(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    TransactionEngineException expectedException =
        new TransactionEngineException(StatusCode.TRANSACTION_ENGINE_TRANSACTION_FAILED.name());

    TransactionEngineException actualException =
        assertThrows(
            TransactionEngineException.class, () -> transactionEngine.execute(getTestRequest()));

    assertThat(expectedException.getMessage()).isEqualTo(actualException.getMessage());
    verify(transactionPhaseManager, times(4)).proceedToNextPhase(any(), any());
    verify(privacyBudgetClient1, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionPrepare(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionPrepare(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionAbort(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionAbort(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionEnd(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionEnd(any(Transaction.class));
  }

  @Test
  public void execute_endTransactionPhaseExecuted() throws Exception {
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTSTARTED,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.BEGIN);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.BEGIN, ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.PREPARE);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.PREPARE,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.COMMIT);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.COMMIT,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.NOTIFY);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTIFY,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.END);

    when(privacyBudgetClient1.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionPrepare(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionCommit(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionCommit(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionNotify(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionNotify(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));

    transactionEngine.execute(getTestRequest());

    verify(transactionPhaseManager, times(5)).proceedToNextPhase(any(), any());
    verify(privacyBudgetClient1, times(1)).performActionEnd(any(Transaction.class));
    verify(privacyBudgetClient2, times(1)).performActionEnd(any(Transaction.class));
  }

  @Test
  public void execute_AbortNotExecutedWhenCurrentPhaseNotStarted() throws Exception {
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.NOTSTARTED,
            ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.BEGIN);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.BEGIN,
            ExecutionResult.create(ExecutionStatus.FAILURE, StatusCode.UNKNOWN)))
        .thenReturn(TransactionPhase.ABORT);
    when(transactionPhaseManager.proceedToNextPhase(
            TransactionPhase.ABORT, ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK)))
        .thenReturn(TransactionPhase.END);

    when(privacyBudgetClient1.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient2.performActionBegin(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.FAILURE, StatusCode.UNKNOWN));
    when(privacyBudgetClient1.performActionAbort(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));
    when(privacyBudgetClient1.performActionEnd(any(Transaction.class)))
        .thenReturn(ExecutionResult.create(ExecutionStatus.SUCCESS, StatusCode.OK));

    assertThrows(
        TransactionEngineException.class, () -> transactionEngine.execute(getTestRequest()));

    verify(transactionPhaseManager, times(3)).proceedToNextPhase(any(), any());
    verify(privacyBudgetClient1, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionAbort(any(Transaction.class));
    verify(privacyBudgetClient1, times(1)).performActionEnd(any(Transaction.class));

    verify(privacyBudgetClient2, times(1)).performActionBegin(any(Transaction.class));
    verify(privacyBudgetClient2, never()).performActionAbort(any(Transaction.class));
    verify(privacyBudgetClient2, never()).performActionEnd(any(Transaction.class));
  }

  private TransactionRequest getTestRequest() {
    return TransactionRequest.builder()
        .setTransactionId(new UUID(0L, 0L))
        .setAttributionReportTo("random_adtech")
        .setPrivacyBudgetUnits(ImmutableList.of())
        .setTimeout(Timestamp.from(Instant.now()))
        .setTransactionSecret("random secret")
        .build();
  }
}
