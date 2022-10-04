package(default_visibility = ["//visibility:public"])

java_test(
    name = "AwsLifecycleClientTest",
    srcs = glob(["*.java"]),
    deps = [
        "//java/com/google/scp/operator/cpio/configclient:aws",
        "//java/com/google/scp/operator/cpio/lifecycleclient:aws",
        "//java/com/google/scp/shared/clients/configclient:aws",
        "//java/external:aws_auto_scaling",
        "//java/external:google_truth",
        "//java/external:google_truth8",
        "//java/external:guice",
        "//java/external:guice-testlib",
        "//java/external:mockito",
    ],
)
