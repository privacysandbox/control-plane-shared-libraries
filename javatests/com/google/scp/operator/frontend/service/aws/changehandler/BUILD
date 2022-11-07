java_test(
    name = "JobQueueWriteHandlerTest",
    srcs = ["JobQueueWriteHandlerTest.java"],
    deps = [
        "//java/com/google/scp/operator/frontend/service/aws/changehandler",
        "//java/com/google/scp/operator/shared/dao/jobqueue/common",
        "//java/com/google/scp/operator/shared/dao/jobqueue/testing",
        "//java/com/google/scp/operator/shared/dao/metadatadb/testing:job_generator",
        "//java/com/google/scp/operator/shared/model",
        "//java/external:acai",
        "//java/external:google_truth",
        "//java/external:guice",
        "//operator/protos/shared/backend:java_proto",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)

java_test(
    name = "MarkJobFailedToEnqueueHandlerTest",
    srcs = ["MarkJobFailedToEnqueueHandlerTest.java"],
    deps = [
        "//java/com/google/scp/operator/frontend/service/aws/changehandler",
        "//java/com/google/scp/operator/shared/dao/metadatadb/common",
        "//java/com/google/scp/operator/shared/dao/metadatadb/testing:fake_metadata_db",
        "//java/com/google/scp/operator/shared/dao/metadatadb/testing:job_generator",
        "//java/com/google/scp/operator/shared/model",
        "//java/com/google/scp/shared/proto",
        "//java/external:acai",
        "//java/external:google_truth",
        "//java/external:guice",
        "//operator/protos/shared/backend:java_proto",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)
