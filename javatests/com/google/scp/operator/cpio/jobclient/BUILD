package(default_visibility = ["//visibility:public"])

java_test(
    name = "JobClientImplTest",
    srcs = ["JobClientImplTest.java"],
    deps = [
        "//java/com/google/scp/operator/cpio/jobclient:aws",
        "//java/com/google/scp/operator/cpio/jobclient:model",
        "//java/com/google/scp/operator/cpio/lifecycleclient:local",
        "//java/com/google/scp/operator/cpio/metricclient",
        "//java/com/google/scp/operator/shared/dao/jobqueue/common",
        "//java/com/google/scp/operator/shared/dao/jobqueue/testing",
        "//java/com/google/scp/operator/shared/dao/metadatadb/common",
        "//java/com/google/scp/operator/shared/dao/metadatadb/testing:fake_metadata_db",
        "//java/com/google/scp/operator/shared/model",
        "//java/com/google/scp/shared/proto",
        "//java/external:acai",
        "//java/external:google_truth",
        "//java/external:google_truth8",
        "//java/external:guice",
        "//java/external:protobuf_java_util",
        "//operator/protos/shared/backend:java_proto",
        "//operator/protos/shared/backend/jobqueue:java_proto",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)

java_test(
    name = "JobValidatorCheckRetryLimitTest",
    srcs = ["JobValidatorCheckRetryLimitTest.java"],
    deps = [
        "//java/com/google/scp/operator/cpio/jobclient:aws",
        "//java/com/google/scp/operator/cpio/jobclient:model",
        "//java/external:acai",
        "//java/external:google_truth",
        "//java/external:guice",
    ],
)
