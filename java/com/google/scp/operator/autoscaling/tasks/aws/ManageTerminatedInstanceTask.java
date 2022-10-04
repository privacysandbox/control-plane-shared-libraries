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

import com.google.inject.Inject;
import javax.annotation.Nullable;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import software.amazon.awssdk.services.autoscaling.AutoScalingClient;
import software.amazon.awssdk.services.autoscaling.model.AutoScalingInstanceDetails;
import software.amazon.awssdk.services.autoscaling.model.CompleteLifecycleActionRequest;
import software.amazon.awssdk.services.autoscaling.model.DescribeAutoScalingInstancesRequest;
import software.amazon.awssdk.services.autoscaling.model.DescribeAutoScalingInstancesResponse;
import software.amazon.awssdk.services.autoscaling.model.LifecycleState;

/** Determines whether to complete the EC2 Instance termination lifecycle action. */
public class ManageTerminatedInstanceTask {

  private static final Logger logger = LoggerFactory.getLogger(ManageTerminatedInstanceTask.class);
  private final AutoScalingClient autoScalingClient;

  @Inject
  public ManageTerminatedInstanceTask(AutoScalingClient autoScalingClient) {
    this.autoScalingClient = autoScalingClient;
  }

  /**
   * Checks the terminating EC2 Instance's lifecycle and health status. If the lifecycle state is
   * Terminating:Wait and health status is UNHEALTHY, complete the lifecycle action and continue
   * with instance termination and return true. Otherwise, return false.
   */
  public Boolean manageTerminatedInstance(
      String asgName, String instanceId, String lifecycleHookName, String lifecycleActionToken) {
    AutoScalingInstanceDetails instanceDetails = getInstanceDetails(instanceId);
    logger.info(
        String.format(
            "EC2 Instance %s has lifecycle state %s and health status %s.",
            instanceId, instanceDetails.lifecycleState(), instanceDetails.healthStatus()));

    if (instanceDetails.healthStatus().equals("UNHEALTHY")
        && instanceDetails.lifecycleState().equals(LifecycleState.TERMINATING_WAIT.toString())) {
      logger.info("Completing lifecycle action.");
      completeLifecycleAction(asgName, instanceId, lifecycleHookName, lifecycleActionToken);
      return true;
    }
    logger.info("Lifecycle action completion skipped.");
    return false;
  }

  @Nullable
  private AutoScalingInstanceDetails getInstanceDetails(String instanceId) {
    DescribeAutoScalingInstancesRequest describeInstanceRequest =
        DescribeAutoScalingInstancesRequest.builder().instanceIds(instanceId).build();
    DescribeAutoScalingInstancesResponse describeInstanceResponse =
        autoScalingClient.describeAutoScalingInstances(describeInstanceRequest);

    if (describeInstanceResponse.autoScalingInstances().isEmpty()) {
      logger.info(
          String.format(
              "EC2 Instance %s has already been terminated or does not exist.", instanceId));
      return null;
    }
    return describeInstanceResponse.autoScalingInstances().get(0);
  }

  private void completeLifecycleAction(
      String asgName, String instanceId, String lifecycleHookName, String lifecycleActionToken) {
    CompleteLifecycleActionRequest completeLifecycleActionRequest =
        CompleteLifecycleActionRequest.builder()
            .instanceId(instanceId)
            .autoScalingGroupName(asgName)
            .lifecycleHookName(lifecycleHookName)
            .lifecycleActionToken(lifecycleActionToken)
            .lifecycleActionResult("CONTINUE")
            .build();
    autoScalingClient.completeLifecycleAction(completeLifecycleActionRequest);
    logger.info(
        String.format("EC2 Instance %s termination lifecycle action completed.", instanceId));
  }
}
