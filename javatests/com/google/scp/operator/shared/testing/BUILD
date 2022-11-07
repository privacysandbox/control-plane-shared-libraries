package(default_visibility = ["//visibility:public"])

java_test(
    name = "StringToByteSourceConverterTest",
    srcs = ["StringToByteSourceConverterTest.java"],
    deps = [
        "//java/com/google/scp/operator/shared/testing",
        "//java/external:google_truth",
        "//java/external:guava",
    ],
)

java_test(
    name = "JobGeneratorTest",
    srcs = ["JobGeneratorTest.java"],
    deps = [
        "//java/com/google/scp/operator/frontend/service/model",
        "//java/com/google/scp/operator/shared/dao/metadatadb/aws/model/converter",
        "//java/com/google/scp/operator/shared/dao/metadatadb/testing:job_generator",
        "//java/com/google/scp/operator/shared/model",
        "//java/com/google/scp/shared/proto",
        "//java/external:acai",
        "//java/external:aws_dynamodb",
        "//java/external:aws_lambda_java_core",
        "//java/external:aws_lambda_java_events",
        "//java/external:aws_lambda_java_events_sdk_transformer",
        "//java/external:google_truth",
        "//java/external:guava",
        "//java/external:guice",
        "//java/external:protobuf_java",
        "//java/external:protobuf_java_util",
        "//javatests/com/google/scp/operator/frontend/service",
        "//operator/protos/frontend/api/v1:java_proto",
        "//operator/protos/shared/backend:java_proto",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)

java_test(
    name = "FakeJobMetadataChangeHandlerTest",
    srcs = ["FakeJobMetadataChangeHandlerTest.java"],
    deps = [
        "//java/com/google/scp/operator/frontend/testing:fake_jobmetadata_change_handler",
        "//java/com/google/scp/operator/shared/dao/metadatadb/testing:job_generator",
        "//java/external:aws_lambda_java_core",
        "//java/external:aws_lambda_java_events",
        "//java/external:google_truth",
        "//java/external:guava",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)

java_library(
    name = "hermetic_aws_clients_module",
    testonly = True,
    srcs = ["HermeticAwsClientsModule.java"],
    deps = [
        "//java/com/google/scp/operator/shared/injection/modules",
        "//java/external:autoservice",
        "//java/external:autoservice_annotations",
        "//java/external:aws_apache_http",
        "//java/external:aws_auth",
        "//java/external:aws_core",
        "//java/external:aws_core2",
        "//java/external:aws_dynamodb",
        "//java/external:aws_dynamodb_enhanced",
        "//java/external:aws_lambda_java_core",
        "//java/external:aws_lambda_java_events",
        "//java/external:aws_regions",
        "//java/external:aws_sqs",
        "//java/external:guava",
        "//java/external:guice",
        "//java/external:localstack_containers",
        "//javatests/com/google/scp/shared/testutils/aws",
    ],
)
