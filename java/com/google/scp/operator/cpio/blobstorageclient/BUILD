package(default_visibility = ["//visibility:public"])

java_library(
    name = "blobstorageclient",
    srcs = glob([
        "*.java",
        "testing/*.java",
    ]),
    deps = [
        ":model",
        "//java/external:autovalue",
        "//java/external:autovalue_annotations",
        "//java/external:guava",
        "//java/external:guice",
        "//java/external:javax_inject",
    ],
)

java_library(
    name = "aws",
    srcs = glob([
        "*.java",
        "aws/*.java",
        "testing/*.java",
    ]),
    deps = [
        ":model",
        "//java/com/google/scp/shared/clients/configclient",
        "//java/external:aws_apache_http",
        "//java/external:aws_auth",
        "//java/external:aws_core",
        "//java/external:aws_regions",
        "//java/external:aws_s3",
        "//java/external:google_gax",
        "//java/external:guava",
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
    ],
)
