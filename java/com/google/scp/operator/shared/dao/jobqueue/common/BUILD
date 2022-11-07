package(default_visibility = ["//visibility:public"])

java_library(
    name = "common",
    srcs = glob(["*.java"]),
    deps = [
        "//java/com/google/scp/operator/shared/model",
        "//java/external:guice",
        "//operator/protos/shared/backend:java_proto",
        "//operator/protos/shared/backend/jobqueue:java_proto",
    ],
)
