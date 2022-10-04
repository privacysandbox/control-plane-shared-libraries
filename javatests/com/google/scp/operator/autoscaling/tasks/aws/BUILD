package(default_visibility = ["//visibility:public"])

java_test(
    name = "GetMessageCountTaskTest",
    srcs = [
        "GetMessageCountTaskTest.java",
    ],
    deps = [
        "//java/com/google/scp/operator/autoscaling/tasks/aws:AutoScalingTasks",
        "//java/external:aws_sqs",
        "//java/external:mockito",
    ],
)

java_test(
    name = "SetDesiredCapacityTaskTest",
    srcs = [
        "SetDesiredCapacityTaskTest.java",
    ],
    deps = [
        "//java/com/google/scp/operator/autoscaling/tasks/aws:AutoScalingTasks",
        "//java/com/google/scp/shared/api/exception",
        "//java/external:aws_auto_scaling",
        "//java/external:mockito",
    ],
)

java_test(
    name = "ManageTerminatedInstanceTaskTest",
    srcs = [
        "ManageTerminatedInstanceTaskTest.java",
    ],
    deps = [
        "//java/com/google/scp/operator/autoscaling/tasks/aws:AutoScalingTasks",
        "//java/external:aws_auto_scaling",
        "//java/external:mockito",
    ],
)
