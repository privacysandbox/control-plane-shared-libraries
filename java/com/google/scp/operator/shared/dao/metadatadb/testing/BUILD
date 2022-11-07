package(default_visibility = ["//visibility:public"])

java_library(
    name = "fake_metadata_db",
    srcs = ["FakeMetadataDb.java"],
    deps = [
        "//java/com/google/scp/operator/shared/dao/metadatadb/common",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)

java_library(
    name = "fake_asg_instances_dao",
    srcs = ["FakeAsgInstancesDao.java"],
    deps = [
        "//java/com/google/scp/operator/shared/dao/metadatadb/common",
        "//operator/protos/shared/backend/asginstance:java_proto",
    ],
)

java_library(
    name = "job_generator",
    srcs = ["JobGenerator.java"],
    deps = [
        "//java/com/google/scp/operator/shared/model",
        "//java/com/google/scp/shared/proto",
        "//java/external:aws_lambda_java_core",
        "//java/external:aws_lambda_java_events",
        "//java/external:guava",
        "//java/external:guice",
        "//operator/protos/shared/backend:java_proto",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)
