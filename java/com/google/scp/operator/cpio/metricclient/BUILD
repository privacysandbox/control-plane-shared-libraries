package(default_visibility = ["//visibility:public"])

java_library(
    name = "aws",
    srcs = glob([
        "*.java",
        "aws/*.java",
        "local/*.java",
    ]),
    deps = [
        ":model",
        "//java/com/google/scp/operator/cpio/configclient",
        "//java/com/google/scp/operator/cpio/metricclient",
        "//java/com/google/scp/shared/clients/configclient",
        "//java/external:aws_auth",
        "//java/external:aws_cloudwatch",
        "//java/external:aws_core",
        "//java/external:aws_http_client_spi",
        "//java/external:aws_regions",
        "//java/external:guava",
        "//java/external:guice",
        "//java/external:javax_inject",
    ],
)

java_library(
    name = "metricclient",
    srcs = glob([
        "*.java",
        "local/*.java",
    ]),
    deps = [
        ":model",
        "//java/external:guice",
        "//java/external:javax_inject",
    ],
)

java_library(
    name = "model",
    srcs = glob(["model/*.java"]),
    deps = [
        "//java/external:autovalue",
        "//java/external:autovalue_annotations",
        "//java/external:guava",
        "//java/external:jackson_annotations",
    ],
)
