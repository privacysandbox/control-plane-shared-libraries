package(default_visibility = ["//visibility:public"])

java_library(
    name = "AutoScalingTasks",
    srcs = glob(["*.java"]),
    deps = [
        "//java/com/google/scp/operator/shared/dao/jobqueue/aws",
        "//java/com/google/scp/shared/api/exception",
        "//java/com/google/scp/shared/api/model",
        "//java/external:aws_auto_scaling",
        "//java/external:aws_sqs",
        "//java/external:findbugs",
        "//java/external:guice",
        "//java/external:slf4j",
        "//java/external:slf4j_simple",
    ],
)
