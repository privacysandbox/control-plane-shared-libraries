load("//operator/worker/aws/build_defs:deploy.bzl", "worker_aws_deployment")
load("@bazel_skylib//rules:common_settings.bzl", "string_flag")
load("@rules_pkg//:pkg.bzl", "pkg_tar")

package(default_visibility = ["//visibility:public"])

exports_files([
    "setup_enclave.sh",
    "aggregation_worker_ami.pkr.hcl",
])

# Ensure libnsm ends up in the load path.
pkg_tar(
    name = "libnsm-tar",
    srcs = [
        "@libnsm//file",
    ],
    mode = "0644",
    package_dir = "/lib/x86_64-linux-gnu/",
    visibility = ["//visibility:public"],
)
