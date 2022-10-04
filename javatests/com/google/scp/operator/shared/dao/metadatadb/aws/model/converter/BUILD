package(default_visibility = ["//visibility:public"])

java_test(
    name = "AttributeValueMapToJobMetadataConverterTest",
    srcs = ["AttributeValueMapToJobMetadataConverterTest.java"],
    deps = [
        "//java/com/google/scp/operator/shared/dao/metadatadb/aws/model/converter",
        "//java/com/google/scp/operator/shared/dao/metadatadb/model",
        "//java/com/google/scp/operator/shared/model",
        "//java/com/google/scp/shared/proto",
        "//java/external:acai",
        "//java/external:aws_dynamodb",
        "//java/external:aws_dynamodb_enhanced",
        "//java/external:google_truth",
        "//java/external:guice",
        "//operator/protos/shared/backend:java_proto",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)
