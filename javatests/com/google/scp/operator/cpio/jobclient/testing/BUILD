package(default_visibility = ["//visibility:public"])

java_test(
    name = "ConstantJobClientTest",
    srcs = ["ConstantJobClientTest.java"],
    deps = [
        "//java/com/google/scp/operator/cpio/jobclient:aws",
        "//java/com/google/scp/operator/cpio/jobclient:model",
        "//java/external:google_truth",
        "//java/external:google_truth8",
    ],
)

java_test(
    name = "FakeJobGeneratorTest",
    srcs = ["FakeJobGeneratorTest.java"],
    deps = [
        "//java/com/google/scp/operator/cpio/jobclient:aws",
        "//java/com/google/scp/operator/cpio/jobclient:model",
        "//java/com/google/scp/operator/shared/dao/metadatadb/testing:job_generator",
        "//java/com/google/scp/operator/shared/model",
        "//java/external:google_truth",
        "//operator/protos/shared/backend:java_proto",
    ],
)
