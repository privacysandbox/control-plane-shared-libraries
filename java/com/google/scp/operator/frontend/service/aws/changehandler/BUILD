package(default_visibility = ["//visibility:public"])

java_library(
    name = "changehandler",
    srcs = glob(["*.java"]),
    deps = [
        "//java/com/google/scp/operator/shared/dao/jobqueue/common",
        "//java/com/google/scp/operator/shared/dao/metadatadb/common",
        "//java/com/google/scp/operator/shared/model",
        "//java/com/google/scp/shared/proto",
        "//java/external:guava",
        "//java/external:guice",
        "//java/external:slf4j",
        "//operator/protos/shared/backend:java_proto",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)
