package(default_visibility = ["//visibility:public"])

java_library(
    name = "converter",
    srcs = glob(["*.java"]),
    deps = [
        "//java/com/google/scp/operator/frontend/service/model",
        "//java/com/google/scp/operator/shared/dao/metadatadb/model",
        "//java/com/google/scp/operator/shared/model",
        "//java/com/google/scp/shared/proto",
        "//java/external:guava",
        "//java/external:guice",
        "//java/external:protobuf_java_util",
        "//operator/protos/frontend/api/v1:java_proto",
        "//operator/protos/shared/backend:java_proto",
        "//operator/protos/shared/backend/metadatadb:java_proto",
    ],
)
