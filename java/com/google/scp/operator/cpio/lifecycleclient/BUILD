package(default_visibility = ["//visibility:public"])

java_library(
    name = "aws",
    srcs = glob([
        "*.java",
        "aws/*.java",
    ]),
    deps = [
        "//java/com/google/scp/operator/cpio/configclient",
        "//java/com/google/scp/operator/cpio/configclient:aws",
        "//java/com/google/scp/shared/clients/configclient",
        "//java/com/google/scp/shared/clients/configclient:model",
        "//java/external:aws_apache_http",
        "//java/external:aws_auth",
        "//java/external:aws_auto_scaling",
        "//java/external:aws_core",
        "//java/external:aws_ec2",
        "//java/external:aws_http_client_spi",
        "//java/external:aws_regions",
        "//java/external:aws_ssm",
        "//java/external:aws_sts",
        "//java/external:aws_utils",
        "//java/external:findbugs",
        "//java/external:guava",
        "//java/external:guice",
        "//java/external:javax_inject",
        "//java/external:slf4j",
    ],
)

java_library(
    name = "local",
    srcs = glob([
        "*.java",
        "local/*.java",
    ]),
    deps = [
        "//java/external:guice",
    ],
)

java_library(
    name = "lifecycleclient",
    srcs = glob([
        "*.java",
    ]),
    deps = [
        "//java/external:guice",
    ],
)
