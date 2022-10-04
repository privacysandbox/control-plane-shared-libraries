package(default_visibility = ["//visibility:public"])

java_library(
    name = "factories",
    srcs = glob(["*.java"]),
    deps = [
        "//java/com/google/scp/operator/frontend/injection/modules",
        "//java/com/google/scp/operator/frontend/serialization",
        "//java/com/google/scp/operator/frontend/service",
        "//java/com/google/scp/operator/frontend/service/aws:aws_helpers",
        "//java/com/google/scp/operator/frontend/service/aws/changehandler",
        "//java/com/google/scp/operator/shared/injection/factories",
        "//java/com/google/scp/operator/shared/injection/modules",
        "//java/com/google/scp/shared/api/exception",
        "//java/com/google/scp/shared/api/model",
        "//java/external:guava",
        "//java/external:guice",
        "//shared/protos/api/v1:java_proto",
    ],
)
