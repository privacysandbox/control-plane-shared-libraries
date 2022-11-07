package(default_visibility = ["//visibility:public"])

java_library(
    name = "model",
    srcs = glob(["*.java"]),
    deps = [
        "//java/com/google/scp/operator/shared/dao/metadatadb/aws/model/attributeconverter",
        "//java/com/google/scp/operator/shared/model",
        "//java/com/google/scp/shared/proto",
        "//java/external:aws_dynamodb_enhanced",
        "//operator/protos/shared/backend:java_proto",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)
