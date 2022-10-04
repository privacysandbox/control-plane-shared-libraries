package(default_visibility = ["//visibility:public"])

java_test(
    name = "FakeMetadataDbTest",
    srcs = ["FakeMetadataDbTest.java"],
    deps = [
        "//java/com/google/scp/operator/shared/dao/metadatadb/common",
        "//java/com/google/scp/operator/shared/dao/metadatadb/model",
        "//java/com/google/scp/operator/shared/dao/metadatadb/testing:fake_metadata_db",
        "//java/com/google/scp/operator/shared/model",
        "//java/com/google/scp/shared/proto",
        "//java/external:google_truth",
        "//java/external:google_truth8",
        "//operator/protos/shared/backend:java_proto",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)
