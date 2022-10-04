package(default_visibility = ["//visibility:public"])

java_library(
    name = "aws",
    srcs = glob(["*.java"]),
    deps = [
        "//java/com/google/crypto/tink/integration/awskmsv2",
        "//java/com/google/scp/shared/aws/credsprovider",
        "//java/com/google/scp/shared/crypto/tink",
        "//java/com/google/scp/shared/crypto/tink/kmstoolenclave",
        "//java/external:aws_arns",
        "//java/external:aws_auth",
        "//java/external:aws_http_client_spi",
        "//java/external:aws_kms",
        "//java/external:aws_regions",
        "//java/external:tink",
    ],
)
