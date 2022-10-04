package(default_visibility = ["//visibility:public"])

java_library(
    name = "configclient",
    srcs = glob(["*.java"]),
    deps = [
        "//java/external:guice",
    ],
)

java_library(
    name = "aws",
    srcs = glob([
        "aws/*.java",
        "local/*.java",
    ]),
    deps = [
        "//java/com/google/scp/operator/cpio/configclient",
        "//java/com/google/scp/shared/aws/credsprovider",
        "//java/com/google/scp/shared/aws/util",
        "//java/com/google/scp/shared/clients/configclient:aws",
        "//java/com/google/scp/shared/clients/configclient:model",
        "//java/external:apache_httpclient",
        "//java/external:apache_httpclient5",
        "//java/external:apache_httpcore",
        "//java/external:apache_httpcore5",
        "//java/external:aws_apache_http",
        "//java/external:aws_auth",
        "//java/external:aws_core",
        "//java/external:aws_ec2",
        "//java/external:aws_http_client_spi",
        "//java/external:aws_regions",
        "//java/external:aws_ssm",
        "//java/external:aws_sts",
        "//java/external:aws_utils",
        "//java/external:guava",
        "//java/external:guice",
        "//java/external:javax_inject",
    ],
)
