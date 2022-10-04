package(default_visibility = ["//visibility:public"])

java_library(
    name = "tink_utils",
    srcs = [
        "AwsTinkUtilsModule.java",
        "MockTinkUtils.java",
        "TinkUtils.java",
    ],
    deps = [
        "//java/com/google/crypto/tink/integration/awskmsv2",
        "//java/com/google/scp/coordinator/keymanagement/shared/util",
        "//java/com/google/scp/operator/cpio/cryptoclient:aws",
        "//java/external:acai",
        "//java/external:aws_apache_http",
        "//java/external:aws_auth",
        "//java/external:guice",
        "//java/external:localstack_containers",
        "//java/external:mockito",
        "//java/external:protobuf_java",
        "//java/external:tink",
        "//javatests/com/google/scp/shared/testutils/aws",
    ],
)
