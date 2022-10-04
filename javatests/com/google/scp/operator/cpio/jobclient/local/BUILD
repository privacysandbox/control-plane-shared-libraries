package(default_visibility = ["//visibility:public"])

java_test(
    name = "LocalFileJobClientTest",
    srcs = ["LocalFileJobClientTest.java"],
    deps = [
        "//java/com/google/scp/operator/cpio/jobclient:aws",
        "//java/com/google/scp/operator/cpio/jobclient:model",
        "//java/com/google/scp/operator/shared/model",
        "//java/com/google/scp/shared/mapper",
        "//java/com/google/scp/shared/proto",
        "//java/external:acai",
        "//java/external:google_truth",
        "//java/external:google_truth8",
        "//java/external:guice",
        "//java/external:jackson_databind",
        "//java/external:jimfs",
        "//java/external:protobuf_java_util",
        "//operator/protos/shared/backend:java_proto",
    ],
)
