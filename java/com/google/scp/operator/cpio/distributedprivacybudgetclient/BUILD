package(default_visibility = ["//visibility:public"])

java_library(
    name = "distributed_privacy_budget_client",
    srcs = glob([
        "*.java",
        "local/*.java",
        "testing/*.java",
    ]),
    deps = [
        "//java/com/google/scp/coordinator/privacy/budgeting/model",
        "//java/com/google/scp/operator/cpio/configclient",
        "//java/com/google/scp/operator/cpio/configclient:aws",
        "//java/com/google/scp/shared/aws/credsprovider",
        "//java/com/google/scp/shared/aws/util",
        "//java/com/google/scp/shared/clients/configclient:aws",
        "//java/com/google/scp/shared/mapper",
        "//java/external:autovalue",
        "//java/external:autovalue_annotations",
        "//java/external:aws_auth",
        "//java/external:aws_regions",
        "//java/external:guava",
        "//java/external:guice",
        "//java/external:jackson_annotations",
        "//java/external:jackson_core",
        "//java/external:jackson_databind",
        "//java/external:slf4j",
    ],
)
