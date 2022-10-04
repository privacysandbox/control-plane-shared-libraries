package(default_visibility = ["//visibility:public"])

java_library(
    name = "testing",
    srcs = glob(["*.java"]),
    data = [
        # LINT.IfChange(integration-frontend-deploy-jar)
        "//java/com/google/scp/operator/frontend:AwsApigatewayFrontend_deploy.jar",
        # LINT.ThenChange(LocalFrontendServiceModule.java:integration-frontend-deploy-jar)
    ],
    deps = [
        "//java/com/google/scp/operator/shared/dao/metadatadb/aws",
        "//java/com/google/scp/operator/shared/dao/metadatadb/common",
        "//java/com/google/scp/operator/shared/injection/modules",
        "//java/external:acai",
        "//java/external:aws_dynamodb",
        "//java/external:aws_dynamodb_enhanced",
        "//java/external:aws_lambda_v2",
        "//java/external:aws_s3",
        "//java/external:google_truth",
        "//java/external:guava",
        "//java/external:guice",
        "//java/external:localstack_containers",
        "//javatests/com/google/scp/shared/testutils/aws",
    ],
)
