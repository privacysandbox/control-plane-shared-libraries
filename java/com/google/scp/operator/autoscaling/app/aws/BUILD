load("@rules_pkg//:mappings.bzl", "pkg_files")

package(default_visibility = ["//visibility:public"])

java_library(
    name = "AsgCapacityHandler",
    srcs = [
        "AsgCapacityHandler.java",
        "AsgCapacityModule.java",
    ],
    deps = [
        "//java/com/google/scp/operator/autoscaling/tasks/aws:AutoScalingTasks",
        "//java/com/google/scp/operator/shared/dao/jobqueue/aws",
        "//java/com/google/scp/shared/api/exception",
        "//java/external:aws_auth",
        "//java/external:aws_auto_scaling",
        "//java/external:aws_http_client_spi",
        "//java/external:aws_lambda_java_core",
        "//java/external:aws_lambda_java_events",
        "//java/external:aws_sqs",
        "//java/external:aws_url_connection_client",
        "//java/external:guice",
    ],
)

alias(
    name = "AsgCapacityHandlerLambdaDeploy",
    actual = ":AsgCapacityHandlerLambda_deploy.jar",
)

pkg_files(
    name = "AsgCapacityHandlerLambdaDeployPackage",
    srcs = [
        ":AsgCapacityHandlerLambdaDeploy",
    ],
    prefix = "jars",
    strip_prefix = ".",
)

java_binary(
    name = "AsgCapacityHandlerLambda",
    runtime_deps = [
        ":AsgCapacityHandler",
    ],
)

java_library(
    name = "TerminatedInstanceHandler",
    srcs = [
        "TerminatedInstanceHandler.java",
        "TerminatedInstanceModule.java",
    ],
    deps = [
        "//java/com/google/scp/operator/autoscaling/tasks/aws:AutoScalingTasks",
        "//java/external:aws_auth",
        "//java/external:aws_auto_scaling",
        "//java/external:aws_http_client_spi",
        "//java/external:aws_lambda_java_core",
        "//java/external:aws_lambda_java_events",
        "//java/external:aws_url_connection_client",
        "//java/external:guice",
    ],
)

alias(
    name = "TerminatedInstanceHandlerLambdaDeploy",
    actual = ":TerminatedInstanceHandlerLambda_deploy.jar",
)

pkg_files(
    name = "TerminatedInstanceHandlerLambdaDeployPackage",
    srcs = [
        ":TerminatedInstanceHandlerLambdaDeploy",
    ],
    prefix = "jars",
    strip_prefix = ".",
)

java_binary(
    name = "TerminatedInstanceHandlerLambda",
    runtime_deps = [
        ":TerminatedInstanceHandler",
    ],
)
