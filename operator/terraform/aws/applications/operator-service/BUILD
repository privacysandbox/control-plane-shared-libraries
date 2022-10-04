load("@bazel_skylib//rules:common_settings.bzl", "string_flag")
load("@rules_pkg//:mappings.bzl", "pkg_files")

package(default_visibility = ["//visibility:public"])

pkg_files(
    name = "operator-service",
    srcs = glob(
        # Omit template and bzl files from generated tar.
        ["*.tf"],
    ),
    prefix = "operator-service",
)

alias(
    name = "frontendLambdaDeployable",
    actual = "//java/com/google/scp/operator/frontend:AwsApiGatewayFrontendDeploy",
)

alias(
    name = "changeHandlerLambdaDeployable",
    actual = "//java/com/google/scp/operator/frontend/service/aws:AwsChangeHandlerLambdaDeploy",
)

alias(
    name = "sqsWriteCleanupLambdaDeployable",
    actual = "//java/com/google/scp/operator/frontend/service/aws:AwsFrontendCleanupLambdaDeploy",
)

alias(
    name = "asgCapacityLambdaDeployable",
    actual = "//java/com/google/scp/operator/autoscaling/app/aws:AsgCapacityHandlerLambdaDeploy",
)

alias(
    name = "terminatedInstanceLambdaDeployable",
    actual = "//java/com/google/scp/operator/autoscaling/app/aws:TerminatedInstanceHandlerLambdaDeploy",
)

string_flag(
    name = "ami_name_flag",
    build_setting_default = "scp-enclave-fast-staging",
)

string_flag(
    name = "ami_owners_flag",
    build_setting_default = '["self"]',
)

exports_files([
    "env.template",
    "env-test.template",
])
