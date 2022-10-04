package(default_visibility = ["//visibility:public"])

java_test(
    name = "AsgCapacityHandlerTest",
    srcs = ["AsgCapacityHandlerTest.java"],
    deps = [
        "//java/com/google/scp/operator/autoscaling/app/aws:AsgCapacityHandler",
        "//java/com/google/scp/operator/autoscaling/tasks/aws:AutoScalingTasks",
        "//java/com/google/scp/shared/api/exception",
        "//java/com/google/scp/shared/api/model",
        "//java/external:aws_lambda_java_core",
        "//java/external:aws_lambda_java_events",
        "//java/external:mockito",
    ],
)

java_test(
    name = "TerminatedInstanceHandlerTest",
    srcs = ["TerminatedInstanceHandlerTest.java"],
    deps = [
        "//java/com/google/scp/operator/autoscaling/app/aws:TerminatedInstanceHandler",
        "//java/com/google/scp/operator/autoscaling/tasks/aws:AutoScalingTasks",
        "//java/external:aws_auto_scaling",
        "//java/external:aws_lambda_java_core",
        "//java/external:aws_lambda_java_events",
        "//java/external:mockito",
    ],
)
