package(default_visibility = ["//visibility:public"])

java_library(
    name = "aws",
    srcs = glob(["*.java"]),
    deps = [
        "//java/com/google/scp/operator/shared/dao/metadatadb/aws/model",
        "//java/com/google/scp/operator/shared/dao/metadatadb/common",
        "//java/com/google/scp/shared/proto",
        "//java/external:aws_core",
        "//java/external:aws_dynamodb",
        "//java/external:aws_dynamodb_enhanced",
        "//java/external:guava",
        "//java/external:guice",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)
