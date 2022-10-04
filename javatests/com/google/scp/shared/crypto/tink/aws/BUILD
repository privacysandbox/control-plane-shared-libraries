package(default_visibility = ["//visibility:public"])

java_test(
    name = "AwsTinkUtilTest",
    srcs = [
        "AwsTinkUtilTest.java",
    ],
    deps = [
        "//java/com/google/crypto/tink/integration/awskmsv2",
        "//java/com/google/scp/shared/aws/credsprovider",
        "//java/com/google/scp/shared/crypto/tink",
        "//java/com/google/scp/shared/crypto/tink/aws",
        "//java/com/google/scp/shared/crypto/tink/kmstoolenclave",
        "//java/external:aws_arns",
        "//java/external:aws_auth",
        "//java/external:aws_http_client_spi",
        "//java/external:google_truth",
        "//java/external:mockito",
        "//java/external:tink_imported",
    ],
)
