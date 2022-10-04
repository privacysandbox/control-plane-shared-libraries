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

package com.google.scp.operator.cpio.lifecycleclient.aws;

import com.google.scp.operator.cpio.lifecycleclient.LifecycleClient;
import com.google.scp.shared.clients.configclient.ParameterClient;
import com.google.scp.shared.clients.configclient.ParameterClient.ParameterClientException;
import com.google.scp.shared.clients.configclient.model.WorkerParameter;
import java.util.Optional;
import java.util.logging.Logger;
import javax.inject.Inject;
import software.amazon.awssdk.core.exception.SdkException;
import software.amazon.awssdk.regions.internal.util.EC2MetadataUtils;
import software.amazon.awssdk.services.autoscaling.AutoScalingClient;
import software.amazon.awssdk.services.autoscaling.model.CompleteLifecycleActionRequest;
import software.amazon.awssdk.services.autoscaling.model.DescribeAutoScalingInstancesRequest;
import software.amazon.awssdk.services.autoscaling.model.DescribeAutoScalingInstancesResponse;
import software.amazon.awssdk.services.autoscaling.model.LifecycleState;

/** Lifecycle Client for AWS Cloud instances. */
public final class AwsLifecycleClient implements LifecycleClient {

  private static final Logger logger = Logger.getLogger(AwsLifecycleClient.class.getName());
  private final AutoScalingClient autoScalingClient;
  private final ParameterClient parameterClient;

  /** Creates a new instance of the {@code AwsLifecycleClient} class. */
  @Inject
  AwsLifecycleClient(AutoScalingClient autoScalingClient, ParameterClient parameterClient) {
    this.autoScalingClient = autoScalingClient;
    this.parameterClient = parameterClient;
  }

  @Override
  public Optional<String> getLifecycleState() throws LifecycleClientException {
    String instanceId = EC2MetadataUtils.getInstanceId();
    try {
      DescribeAutoScalingInstancesRequest request =
          DescribeAutoScalingInstancesRequest.builder().instanceIds(instanceId).build();
      DescribeAutoScalingInstancesResponse response =
          autoScalingClient.describeAutoScalingInstances(request);
      return Optional.ofNullable(response.autoScalingInstances().get(0).lifecycleState());
    } catch (SdkException exception) {
      throw new LifecycleClientException(exception);
    }
  }

  @Override
  public boolean handleScaleInLifecycleAction() throws LifecycleClientException {
    Optional<String> scaleInLifecycleHook = Optional.empty();
    try {
      scaleInLifecycleHook = parameterClient.getParameter(WorkerParameter.SCALE_IN_HOOK.name());
    } catch (ParameterClientException e) {
      logger.info("WorkerParameter.SCALE_IN_HOOK not found" + e);
    }

    if (scaleInLifecycleHook.isEmpty() || scaleInLifecycleHook.get().isEmpty()) {
      return false;
    }

    try {
      String instanceId = EC2MetadataUtils.getInstanceId();

      DescribeAutoScalingInstancesRequest request =
          DescribeAutoScalingInstancesRequest.builder().instanceIds(instanceId).build();
      DescribeAutoScalingInstancesResponse response =
          autoScalingClient.describeAutoScalingInstances(request);
      if (response.autoScalingInstances().isEmpty()) {
        return false;
      }
      String lifecycleState = response.autoScalingInstances().get(0).lifecycleState();
      String autoScalingGroup = response.autoScalingInstances().get(0).autoScalingGroupName();

      // TODO : If lifecycle is in terminating,  wait and check if it goes into waiting state
      if (lifecycleState.equals(LifecycleState.TERMINATING_WAIT.toString())) {
        CompleteLifecycleActionRequest lifecycleRequest =
            CompleteLifecycleActionRequest.builder()
                .autoScalingGroupName(autoScalingGroup)
                .lifecycleActionResult("CONTINUE")
                .instanceId(instanceId)
                .lifecycleHookName(scaleInLifecycleHook.get())
                .build();
        autoScalingClient.completeLifecycleAction(lifecycleRequest);
        return true;
      }

      // Scale-in Lifecycle hook was already completed, no-op and return true for scale-in action.
      if (lifecycleState.equals(LifecycleState.TERMINATING_PROCEED.toString())) {
        return true;
      }
    } catch (SdkException exception) {
      throw new LifecycleClientException(exception);
    }
    return false;
  }
}
