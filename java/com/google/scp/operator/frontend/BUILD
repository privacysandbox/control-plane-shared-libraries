load("@rules_pkg//:mappings.bzl", "pkg_files")
load("@io_bazel_rules_docker//container:container.bzl", "container_image")

package(default_visibility = ["//visibility:public"])

java_binary(
    name = "AwsApigatewayFrontend",
    runtime_deps = [
        "//java/com/google/scp/operator/frontend/injection/modules:frontend_module",
        "//java/com/google/scp/operator/frontend/service/aws",
        "//java/external:commons_logging",
    ],
)

alias(
    name = "AwsApiGatewayFrontendDeploy",
    actual = ":AwsApigatewayFrontend_deploy.jar",
)

pkg_files(
    name = "AwsApiGatewayFrontendDeployPackage",
    srcs = [
        ":AwsApiGatewayFrontendDeploy",
    ],
    prefix = "jars",
    strip_prefix = ".",
)
