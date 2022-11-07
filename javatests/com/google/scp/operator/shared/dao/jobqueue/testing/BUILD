package(default_visibility = ["//visibility:public"])

java_test(
    name = "FakeJobQueueTest",
    srcs = ["FakeJobQueueTest.java"],
    deps = [
        "//java/com/google/scp/operator/shared/dao/jobqueue/common",
        "//java/com/google/scp/operator/shared/dao/jobqueue/testing",
        "//java/com/google/scp/operator/shared/model",
        "//java/com/google/scp/shared/proto",
        "//java/external:google_truth",
        "//java/external:google_truth8",
        "//java/external:protobuf_java_util",
        "//operator/protos/shared/backend:java_proto",
        "//operator/protos/shared/backend/jobqueue:java_proto",
    ],
)
