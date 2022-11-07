package(default_visibility = ["//visibility:public"])

java_library(
    name = "converter",
    srcs = glob(["*.java"]),
    deps = [
        "//java/com/google/scp/operator/shared/dao/metadatadb/aws/model",
        "//java/external:aws_dynamodb",
        "//java/external:aws_dynamodb_enhanced",
        "//java/external:aws_lambda_java_core",
        "//java/external:aws_lambda_java_events",
        "//java/external:aws_lambda_java_events_sdk_transformer",
        "//java/external:guava",
        "//java/external:guice",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)
