package(default_visibility = ["//visibility:public"])

java_library(
    name = "validation",
    srcs = glob(["*.java"]),
    deps = [
        "//java/com/google/scp/operator/frontend/service/model",
        "//java/com/google/scp/operator/shared/model",
        "//java/external:guava",
        "//java/external:guice",
        "//operator/protos/frontend/api/v1:java_proto",
        "//operator/protos/shared/backend:java_proto",
    ],
)
