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

package com.google.scp.operator.autoscaling.tasks.aws;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;
import software.amazon.awssdk.services.autoscaling.AutoScalingClient;
import software.amazon.awssdk.services.autoscaling.model.AutoScalingException;
import software.amazon.awssdk.services.autoscaling.model.AutoScalingInstanceDetails;
import software.amazon.awssdk.services.autoscaling.model.CompleteLifecycleActionRequest;
import software.amazon.awssdk.services.autoscaling.model.DescribeAutoScalingInstancesRequest;
import software.amazon.awssdk.services.autoscaling.model.DescribeAutoScalingInstancesResponse;
import software.amazon.awssdk.services.autoscaling.model.LifecycleState;

@RunWith(JUnit4.class)
public final class ManageTerminatedInstanceTaskTest {
  @Rule public final MockitoRule mockito = MockitoJUnit.rule();
  @Mock AutoScalingClient autoScalingClientMock;

  private final String asgName = "fakeAsg";
  private final String instanceId = "fakeInstanceId";
  private final String lifecycleHookName = "fakeLifecycleHookName";
  private final String lifecycleActionToken = "lifecycleActionToken";

  @Test
  public void manageTerminatedInstance_unhealthyInstance() {
    setupAutoScalingClient(false, LifecycleState.TERMINATING_WAIT.toString());

    var manageTerminatedInstanceTask = new ManageTerminatedInstanceTask(autoScalingClientMock);
    Boolean lifecycleActionCompleted =
        manageTerminatedInstanceTask.manageTerminatedInstance(
            asgName, instanceId, lifecycleHookName, lifecycleActionToken);

    Assert.assertTrue(lifecycleActionCompleted);
  }

  @Test
  public void manageTerminatedInstance_healthyInstance() {
    setupAutoScalingClient(true, LifecycleState.TERMINATING_WAIT.toString());

    var manageTerminatedInstanceTask = new ManageTerminatedInstanceTask(autoScalingClientMock);
    Boolean lifecycleActionCompleted =
        manageTerminatedInstanceTask.manageTerminatedInstance(
            asgName, instanceId, lifecycleHookName, lifecycleActionToken);

    Assert.assertFalse(lifecycleActionCompleted);
  }

  @Test
  public void manageTerminatedInstance_instanceAlreadyTerminated() {
    setupAutoScalingClient(false, LifecycleState.TERMINATING_PROCEED.toString());

    var manageTerminatedInstanceTask = new ManageTerminatedInstanceTask(autoScalingClientMock);
    Boolean lifecycleActionCompleted =
        manageTerminatedInstanceTask.manageTerminatedInstance(
            asgName, instanceId, lifecycleHookName, lifecycleActionToken);

    Assert.assertFalse(lifecycleActionCompleted);
  }

  @Test
  public void manageTerminatedInstance_failedToCompleteLifecycleAction() {
    setupAutoScalingClient(false, LifecycleState.TERMINATING_WAIT.toString());
    AutoScalingException completeLifecycleActionError =
        (AutoScalingException)
            AutoScalingException.builder().message("Failed to complete lifecycle action").build();
    when(autoScalingClientMock.completeLifecycleAction(any(CompleteLifecycleActionRequest.class)))
        .thenThrow(completeLifecycleActionError);

    var manageTerminatedInstanceTask = new ManageTerminatedInstanceTask(autoScalingClientMock);
    Assert.assertThrows(
        AutoScalingException.class,
        () ->
            manageTerminatedInstanceTask.manageTerminatedInstance(
                asgName, instanceId, lifecycleHookName, lifecycleActionToken));
  }

  private void setupAutoScalingClient(Boolean isHealthy, String lifecycleState) {
    String healthStatus = isHealthy ? "HEALTHY" : "UNHEALTHY";
    AutoScalingInstanceDetails instanceDetails =
        AutoScalingInstanceDetails.builder()
            .healthStatus(healthStatus)
            .lifecycleState(lifecycleState)
            .build();
    DescribeAutoScalingInstancesResponse describeAsgInstancesResponse =
        DescribeAutoScalingInstancesResponse.builder()
            .autoScalingInstances(instanceDetails)
            .build();
    when(autoScalingClientMock.describeAutoScalingInstances(
            any(DescribeAutoScalingInstancesRequest.class)))
        .thenReturn(describeAsgInstancesResponse);
  }
}
