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

package com.google.scp.operator.autoscaling.app.aws;

import com.google.inject.AbstractModule;
import com.google.inject.Provides;
import com.google.inject.Singleton;
import software.amazon.awssdk.auth.credentials.AwsCredentialsProvider;
import software.amazon.awssdk.auth.credentials.EnvironmentVariableCredentialsProvider;
import software.amazon.awssdk.http.SdkHttpClient;
import software.amazon.awssdk.http.urlconnection.UrlConnectionHttpClient;
import software.amazon.awssdk.services.autoscaling.AutoScalingClient;

/**
 * Defines dependencies to be used with TerminatedInstanceHandler for initial filtering of
 * terminating instances for the worker Auto Scaling Group.
 */
public final class TerminatedInstanceModule extends AbstractModule {

  @Provides
  @Singleton
  public AutoScalingClient provideAutoScalingClient(
      SdkHttpClient httpClient, AwsCredentialsProvider credentialsProvider) {
    return AutoScalingClient.builder()
        .credentialsProvider(credentialsProvider)
        .httpClient(httpClient)
        .build();
  }

  @Override
  protected void configure() {
    bind(SdkHttpClient.class).toInstance(UrlConnectionHttpClient.builder().build());
    bind(AwsCredentialsProvider.class).toInstance(EnvironmentVariableCredentialsProvider.create());
  }
}
