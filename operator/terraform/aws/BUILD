load("@rules_pkg//:pkg.bzl", "pkg_tar")
load("@rules_pkg//:mappings.bzl", "pkg_files")

package(default_visibility = ["//visibility:public"])

load(":build_defs.bzl", "ami_params_tfvar_file")

# Shared ami_params.auto.tfvars value which contains the values configured the build flags.
ami_params_tfvar_file(
    name = "ami_params_auto_tfvars",
    file_name = "ami_params.auto.tfvars",
)

pkg_files(
    name = "dist_jars",
    srcs = [
        "//java/com/google/scp/operator/autoscaling/app/aws:AsgCapacityHandlerLambdaDeploy",
        "//java/com/google/scp/operator/autoscaling/app/aws:TerminatedInstanceHandlerLambdaDeploy",
        "//java/com/google/scp/operator/frontend:AwsApiGatewayFrontendDeployPackage",
        "//java/com/google/scp/operator/frontend/service/aws:AwsChangeHandlerLambdaDeploy",
        "//java/com/google/scp/operator/frontend/service/aws:AwsFrontendCleanupLambdaDeploy",
    ],
    prefix = "jars",
)

pkg_tar(
    name = "operator_tar",
    srcs = [
        ":dist_jars",
        "//licenses",
        "//operator/aws/docs",
        "//operator/terraform/aws/applications:applications_pkg",
        "//operator/terraform/aws/environments/demo:demo_environment_pkg",
        "//operator/terraform/aws/modules:modules_pkg",
    ],
    extension = "tgz",
    strip_prefix = "/",
)
