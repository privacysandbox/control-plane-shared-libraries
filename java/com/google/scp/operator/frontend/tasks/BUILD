package(default_visibility = ["//visibility:public"])

java_library(
    name = "tasks",
    srcs = glob(["*.java"]),
    deps = [
        "//java/com/google/scp/operator/frontend/tasks/validation",
        "//java/com/google/scp/operator/shared/dao/metadatadb/common",
        "//java/com/google/scp/operator/shared/model",
        "//java/com/google/scp/shared/api/exception",
        "//java/com/google/scp/shared/api/model",
        "//java/external:guice",
        "//operator/protos/shared/backend:java_proto",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)
