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

package com.google.scp.shared.aws.credsprovider;

import software.amazon.awssdk.auth.credentials.AwsSessionCredentials;
import software.amazon.awssdk.services.sts.StsClient;
import software.amazon.awssdk.services.sts.model.AssumeRoleRequest;
import software.amazon.awssdk.services.sts.model.Credentials;

/**
 * Provides temporary AWS credentials using AWS Security Token Service (STS)
 *
 * <p>The credentials provided by this class are fetched directly from STS as oppposed to being
 * reused from the credentials available from e.g. the EC2 instance metadata.
 *
 * <p>TODO: implement credential caching.
 */
public final class StsAwsSessionCredentialsProvider implements AwsSessionCredentialsProvider {

  /** Duration for which requested credentials should be valid. */
  private static final int DURATION_SECONDS = 1000;

  private final StsClient stsClient;
  private final String roleArn;
  private final String sessionName;

  /**
   * @param roleArn the role to assume and provide credentials for.
   * @param sessionName The name of the session associated with the credentials (visible in logs)
   */
  public StsAwsSessionCredentialsProvider(StsClient stsClient, String roleArn, String sessionName) {
    this.stsClient = stsClient;
    this.roleArn = roleArn;
    this.sessionName = sessionName;
  }

  @Override
  public AwsSessionCredentials resolveCredentials() {
    AssumeRoleRequest request =
        AssumeRoleRequest.builder()
            .durationSeconds(DURATION_SECONDS)
            .roleArn(roleArn)
            .roleSessionName(sessionName)
            .build();
    Credentials credentials = stsClient.assumeRole(request).credentials();

    return AwsSessionCredentials.create(
        credentials.accessKeyId(), credentials.secretAccessKey(), credentials.sessionToken());
  }
}
