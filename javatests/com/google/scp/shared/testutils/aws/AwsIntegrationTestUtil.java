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

package com.google.scp.shared.testutils.aws;

import static com.google.common.collect.MoreCollectors.onlyElement;
import static software.amazon.awssdk.regions.Region.US_WEST_2;

import java.nio.file.Path;
import java.util.Map;
import org.testcontainers.containers.localstack.LocalStackContainer;
import org.testcontainers.utility.DockerImageName;
import software.amazon.awssdk.enhanced.dynamodb.DynamoDbEnhancedClient;
import software.amazon.awssdk.regions.Region;
import software.amazon.awssdk.services.apigateway.ApiGatewayClient;
import software.amazon.awssdk.services.apigateway.model.IntegrationType;
import software.amazon.awssdk.services.dynamodb.DynamoDbClient;
import software.amazon.awssdk.services.dynamodb.model.AttributeDefinition;
import software.amazon.awssdk.services.dynamodb.model.CreateTableRequest;
import software.amazon.awssdk.services.dynamodb.model.KeySchemaElement;
import software.amazon.awssdk.services.dynamodb.model.KeyType;
import software.amazon.awssdk.services.dynamodb.model.ProvisionedThroughput;
import software.amazon.awssdk.services.dynamodb.model.ScalarAttributeType;
import software.amazon.awssdk.services.dynamodb.waiters.DynamoDbWaiter;
import software.amazon.awssdk.services.kms.KmsClient;
import software.amazon.awssdk.services.kms.model.DataKeySpec;
import software.amazon.awssdk.services.kms.model.KeySpec;
import software.amazon.awssdk.services.kms.model.KeyUsageType;
import software.amazon.awssdk.services.kms.model.SigningAlgorithmSpec;
import software.amazon.awssdk.services.lambda.LambdaClient;
import software.amazon.awssdk.services.lambda.model.CreateFunctionRequest;
import software.amazon.awssdk.services.lambda.model.CreateFunctionResponse;
import software.amazon.awssdk.services.lambda.model.Environment;
import software.amazon.awssdk.services.lambda.model.FunctionCode;
import software.amazon.awssdk.services.s3.S3Client;
import software.amazon.awssdk.services.s3.model.PutObjectRequest;

/** Helper class providing methods to test AWS features locally. */
public final class AwsIntegrationTestUtil {

  private static final String BUCKET_NAME = "bucketname";
  // localstack version is pinned so that tests are repeatable
  private static final DockerImageName IMAGE = DockerImageName.parse("localstack/localstack:1.0.4");
  private static final Region REGION = US_WEST_2;
  private static final String LAMBDA_RUNTIME = "java11";
  private static final String LAMBDA_IAM_ROLE = "arn:aws:iam::000000000000:role/IamRole";
  private static final int LAMBDA_TIMEOUT_SECONDS = 60;

  /** Signature algorithm of the key generated by {@link #createSignatureKey(KmsClient)}. */
  public static final String SIGNATURE_KEY_ALGORITHM =
      SigningAlgorithmSpec.ECDSA_SHA_256.toString();

  private AwsIntegrationTestUtil() {}

  /**
   * Return LocalStackContainer for ClassRule to locally test AWS features via a Docker container.
   */
  public static LocalStackContainer createContainer(LocalStackContainer.Service... services) {
    return new LocalStackContainer(IMAGE).withServices(services);
  }

  /** Return BucketName set on localstack container. */
  public static String getBucketName() {
    return BUCKET_NAME;
  }

  /** Return the region used */
  public static Region getRegion() {
    return REGION;
  }

  /** Return the region name used */
  public static String getRegionName() {
    return getRegion().toString();
  }

  /** Creates a symmetric AWS KMS key and returns its key ID. */
  public static String createKmsKey(KmsClient kmsClient) {
    String keyId = kmsClient.createKey().keyMetadata().keyId();
    kmsClient.generateDataKey(r -> r.keyId(keyId).keySpec(DataKeySpec.AES_256));
    return keyId;
  }

  /**
   * Returns a new asymmetric signature key and returns its key ID. The corresponding algorithm
   * (necessary for sign/verify operations) is {@link #SIGNATURE_KEY_ALGORITHM}.
   */
  public static String createSignatureKey(KmsClient kmsClient) {
    var response =
        kmsClient.createKey(
            (req) -> req.keyUsage(KeyUsageType.SIGN_VERIFY).keySpec(KeySpec.ECC_NIST_P256));

    var hasValidAlgorithm =
        response.keyMetadata().signingAlgorithmsAsStrings().contains(SIGNATURE_KEY_ALGORITHM);

    if (!hasValidAlgorithm) {
      throw new IllegalStateException(
          "SIGNATURE_KEY_ALGORITHM unsupported by generated signing key");
    }
    // Note: There appears to be a bug in LocalStack where providing the ARN causes sign/verify
    // operations to fail, specify the keyId instead.
    return response.keyMetadata().keyId();
  }

  /** Creates a DynamoDB table with the necessary properties to use as a KeyDB. */
  public static void createKeyDbTable(DynamoDbClient ddbClient, String tableName) {
    var request =
        CreateTableRequest.builder()
            .tableName(tableName)
            .attributeDefinitions(
                AttributeDefinition.builder()
                    .attributeName("KeyId")
                    .attributeType(ScalarAttributeType.S)
                    .build())
            .keySchema(
                KeySchemaElement.builder().attributeName("KeyId").keyType(KeyType.HASH).build())
            .provisionedThroughput(
                ProvisionedThroughput.builder()
                    .readCapacityUnits(10L)
                    .writeCapacityUnits(10L)
                    .build())
            .build();
    ddbClient.createTable(request);

    // Wait for the table to be ready, then return the client
    DynamoDbWaiter waiter = ddbClient.waiter();
    waiter.waitUntilTableExists(requestBuilder -> requestBuilder.tableName(tableName));
  }

  /** Creates a DynamoDB table with the necessary properties to use as a metadata DB. */
  public static void createMetadataDbTable(DynamoDbClient ddbClient, String tableName) {
    var request =
        CreateTableRequest.builder()
            .tableName(tableName)
            .attributeDefinitions(
                AttributeDefinition.builder()
                    .attributeName("JobKey")
                    .attributeType(ScalarAttributeType.S)
                    .build())
            .keySchema(
                KeySchemaElement.builder().attributeName("JobKey").keyType(KeyType.HASH).build())
            .provisionedThroughput(
                ProvisionedThroughput.builder()
                    .readCapacityUnits(10L)
                    .writeCapacityUnits(10L)
                    .build())
            .build();
    ddbClient.createTable(request);

    // Wait for the table to be ready, then return the client
    DynamoDbWaiter waiter = ddbClient.waiter();
    waiter.waitUntilTableExists(requestBuilder -> requestBuilder.tableName(tableName));
  }

  /**
   * Return AWS S3Client with a bucket BUCKET_NAME. This client interacts with a localstack docker
   * container allowing tests to be hermetic.
   *
   * @deprecated use {@link LocalStackAwsClientUtil.createS3Client} and manually create the
   *     necessary bucket.
   */
  @Deprecated
  public static S3Client createS3Client(LocalStackContainer localstack) {
    S3Client s3Client = LocalStackAwsClientUtil.createS3Client(localstack);
    s3Client.createBucket(b -> b.bucket(BUCKET_NAME));
    return s3Client;
  }

  /**
   * Provide a DynamoDbClient that uses the localstack container.
   *
   * @deprecated use {@link LocalStackAwsClientUtil.createDynamoDbClient}.
   */
  @Deprecated
  public static DynamoDbClient createDynamoDbClient(LocalStackContainer localstack) {
    return LocalStackAwsClientUtil.createDynamoDbClient(localstack);
  }

  /**
   * Provide a DynamoDbEnhancedClient that uses the localstack container
   *
   * @deprecated use {@link LocalStackAwsClientUtil.createDynamoDbEnhancedClient}.
   */
  @Deprecated
  public static DynamoDbEnhancedClient createDynamoDbEnhancedClient(
      LocalStackContainer localStack) {
    return LocalStackAwsClientUtil.createDynamoDbEnhancedClient(localStack);
  }

  /** Provide a DynamoDbEnhancedClient that an existing DynamoDbClient */
  public static DynamoDbEnhancedClient createDynamoDbEnhancedClient(DynamoDbClient dynamoDbClient) {
    return DynamoDbEnhancedClient.builder().dynamoDbClient(dynamoDbClient).build();
  }

  /**
   * Creates a lambda client configured to use the localstack container.
   *
   * @deprecated use {@link LocalStackAwsClientUtil.createLambdaClient}.
   */
  @Deprecated
  public static LambdaClient createLambdaClient(LocalStackContainer localstack) {
    return LocalStackAwsClientUtil.createLambdaClient(localstack);
  }

  /**
   * Returns ip address which is used as LOCALSTACK_HOSTNAME environment variable in docker
   * container
   */
  public static String getLocalStackHostname(LocalStackContainer localstack) {
    return localstack.getContainerInfo().getNetworkSettings().getNetworks().values().stream()
        .collect(onlyElement())
        .getIpAddress();
  }

  /** Adds a JAR for a lambda to s3 to later be used */
  public static void uploadLambdaCode(S3Client s3Client, String s3Key, Path pathToJar) {
    s3Client.putObject(
        PutObjectRequest.builder().bucket(BUCKET_NAME).key(s3Key).build(), pathToJar);
  }

  /**
   * Adds a lambda handler class to the localstack container
   *
   * @param lambdaClient a lambda client that is connected to the localstack container
   * @param functionName the name to identify the lambda function
   * @param handlerClass the fully qualified classname of the handler class
   * @param handlerJarS3Key the s3 key for the lambda jar
   * @return the ARN of the lambda function created
   */
  public static String addLambdaHandler(
      LambdaClient lambdaClient,
      String functionName,
      String handlerClass,
      String handlerJarS3Key,
      Map<String, String> environmentVariables) {
    CreateFunctionRequest createFunctionRequest =
        CreateFunctionRequest.builder()
            .functionName(functionName)
            .handler(handlerClass)
            .code(FunctionCode.builder().s3Bucket(BUCKET_NAME).s3Key(handlerJarS3Key).build())
            .role(LAMBDA_IAM_ROLE)
            .runtime(LAMBDA_RUNTIME)
            .timeout(LAMBDA_TIMEOUT_SECONDS)
            .environment(Environment.builder().variables(environmentVariables).build())
            .build();
    CreateFunctionResponse createFunctionResponse =
        lambdaClient.createFunction(createFunctionRequest);
    return createFunctionResponse.functionArn();
  }

  /**
   * Performs necessary setup to deploy an ApiGatewayV1 endpoint for a specified lambda. The created
   * endpoint proxies all requests (all paths and all HTTP methods) to the lambda of the provided
   * ARN.
   *
   * <p>For the sake of simplicity no URL variables are populated and assumes that the lambda
   * performs all URL parsing and does not share a rest API with any other lambdas.
   *
   * @return the base endpoint URL of the created API gateway resource (e.g.
   *     "http://foo:80/a/b/_stagename_")
   */
  public static String createApiGatewayHandler(
      LocalStackContainer localStack, ApiGatewayClient client, String lambdaArn) {
    var stageName = "integrationTest";
    // URI follows format detailed in https://docs.aws.amazon.com/lambda/latest/dg/API_Invoke.html
    var integrationUri =
        String.format(
            "arn:aws:apigateway:us-east-1:lambda:path/2015-03-31/functions/%s/invocations",
            lambdaArn);
    var restApiId = client.createRestApi(e -> e.name(lambdaArn)).id();

    // Need to get the resouce id of the root '/' path to pass as parent of the created resource.
    var rootResourceId = client.getResources(r -> r.restApiId(restApiId)).items().get(0).id();
    var resourceId =
        client
            .createResource(
                r -> r.restApiId(restApiId).pathPart("{proxy+}").parentId(rootResourceId))
            .id();

    // Allow all requests to the specified resource
    client.putMethod(
        m ->
            m.restApiId(restApiId)
                .resourceId(resourceId)
                .httpMethod("ANY")
                .authorizationType("NONE"));

    // Attach lambda
    client.putIntegration(
        i ->
            i.restApiId(restApiId)
                .resourceId(resourceId)
                .type(IntegrationType.AWS_PROXY)
                .integrationHttpMethod("POST")
                .uri(integrationUri)
                .httpMethod("ANY"));

    client.createDeployment(e -> e.restApiId(restApiId).stageName(stageName));

    // _user_request_ is hardcoded into LocalStack's urls:
    // https://github.com/localstack/localstack/blob/master/doc/interaction/README.md#invoking-api-gateway
    return String.format(
        "%s/restapis/%s/%s/_user_request_",
        localStack.getEndpointOverride(LocalStackContainer.Service.API_GATEWAY),
        restApiId,
        stageName);
  }
}
