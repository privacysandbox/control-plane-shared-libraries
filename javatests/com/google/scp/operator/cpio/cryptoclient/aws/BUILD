package(default_visibility = ["//visibility:public"])

java_test(
    name = "AwsKmsDecryptionKeyServiceTest",
    srcs = ["AwsKmsDecryptionKeyServiceTest.java"],
    runtime_deps = [
        # LocalStackContainers uses v1 API in the background.
        # However, we don't interface with the v1 API.
        "//java/external:aws_kms_java_sdk1",
    ],
    deps = [
        "//java/com/google/scp/operator/cpio/cryptoclient:aws",
        "//java/external:acai",
        "//java/external:aws_auth",
        "//java/external:aws_kms",
        "//java/external:google_truth",
        "//java/external:guice",
        "//java/external:localstack_containers",
        "//java/external:testcontainers",
        "//java/external:tink",
        "//javatests/com/google/scp/shared/testutils/aws",
        "//javatests/com/google/scp/shared/testutils/crypto:tink_utils",
    ],
)
