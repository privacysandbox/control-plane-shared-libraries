package(default_visibility = ["//visibility:public"])

java_test(
    name = "BackendModelUtilTest",
    srcs = [
        "BackendModelUtilTest.java",
    ],
    deps = [
        "//java/com/google/scp/operator/shared/dao/metadatadb/testing:job_generator",
        "//java/com/google/scp/operator/shared/model",
        "//java/com/google/scp/shared/mapper",
        "//java/external:google_truth",
        "//java/external:jackson_core",
        "//java/external:jackson_databind",
        "//java/external:protobuf_java",
        "//java/external:protobuf_java_util",
        "//operator/protos/shared/backend:java_proto",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)
