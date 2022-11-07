package(default_visibility = ["//visibility:public"])

java_library(
    name = "modules",
    srcs = [
        "BaseAwsClientsModule.java",
        "BaseDataModule.java",
        "EnvironmentVariables.java",
    ],
    deps = [
        "//java/com/google/scp/operator/shared/dao/metadatadb/aws",
        "//java/com/google/scp/operator/shared/dao/metadatadb/aws/model/converter",
        "//java/com/google/scp/operator/shared/dao/metadatadb/common",
        "//java/com/google/scp/operator/shared/injection/factories",
        "//java/external:aws_core2",
        "//java/external:aws_dynamodb",
        "//java/external:aws_dynamodb_enhanced",
        "//java/external:aws_kms",
        "//java/external:aws_lambda_java_core",
        "//java/external:aws_lambda_java_events",
        "//java/external:aws_regions",
        "//java/external:aws_s3",
        "//java/external:aws_sqs",
        "//java/external:guava",
        "//java/external:guice",
    ],
)

java_library(
    name = "data_module",
    srcs = [
        "DataModule.java",
    ],
    deps = [
        "modules",
        "//java/com/google/scp/operator/frontend/tasks/validation",
        "//java/com/google/scp/operator/shared/dao/metadatadb/aws",
        "//java/com/google/scp/operator/shared/dao/metadatadb/aws/model/converter",
        "//java/com/google/scp/operator/shared/dao/metadatadb/common",
        "//java/com/google/scp/operator/shared/environment",
        "//java/com/google/scp/operator/shared/injection/factories",
        "//java/com/google/scp/operator/shared/injection/modules:aws_clients_module",
        "//java/com/google/scp/operator/shared/model",
        "//java/external:autoservice",
        "//java/external:autoservice_annotations",
        "//java/external:aws_dynamodb",
        "//java/external:aws_dynamodb_enhanced",
        "//java/external:aws_lambda_java_core",
        "//java/external:aws_lambda_java_events",
        "//java/external:aws_regions",
        "//java/external:guava",
        "//java/external:guice",
    ],
)

java_library(
    name = "aws_clients_module",
    srcs = [
        "AwsClientsModule.java",
    ],
    deps = [
        "aws_clients_configurable_module",
        "modules",
        "//java/com/google/scp/operator/shared/dao/metadatadb/aws",
        "//java/com/google/scp/operator/shared/environment",
        "//java/com/google/scp/operator/shared/injection/factories",
        "//java/external:autoservice",
        "//java/external:autoservice_annotations",
        "//java/external:aws_auth",
        "//java/external:aws_core2",
        "//java/external:aws_dynamodb",
        "//java/external:aws_dynamodb_enhanced",
        "//java/external:aws_lambda_java_core",
        "//java/external:aws_lambda_java_events",
        "//java/external:aws_regions",
        "//java/external:aws_s3",
        "//java/external:aws_sqs",
        "//java/external:guava",
        "//java/external:guice",
    ],
)

java_library(
    name = "aws_clients_configurable_module",
    srcs = [
        "AwsClientsConfigurableModule.java",
    ],
    deps = [
        "modules",
        "//java/com/google/scp/operator/shared/dao/metadatadb/aws",
        "//java/com/google/scp/operator/shared/environment",
        "//java/com/google/scp/operator/shared/injection/factories",
        "//java/external:autoservice",
        "//java/external:autoservice_annotations",
        "//java/external:aws_auth",
        "//java/external:aws_core2",
        "//java/external:aws_dynamodb",
        "//java/external:aws_dynamodb_enhanced",
        "//java/external:aws_lambda_java_core",
        "//java/external:aws_lambda_java_events",
        "//java/external:aws_regions",
        "//java/external:aws_s3",
        "//java/external:aws_sqs",
        "//java/external:aws_url_connection_client",
        "//java/external:guava",
        "//java/external:guice",
    ],
)
