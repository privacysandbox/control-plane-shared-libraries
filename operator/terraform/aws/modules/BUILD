load("@rules_pkg//:mappings.bzl", "pkg_filegroup")

package(default_visibility = ["//visibility:public"])

pkg_filegroup(
    name = "modules_pkg",
    srcs = [
        "//operator/terraform/aws/modules/alarmsnstopic",
        "//operator/terraform/aws/modules/autoscaling",
        "//operator/terraform/aws/modules/bazel",
        "//operator/terraform/aws/modules/frontend",
        "//operator/terraform/aws/modules/frontenddashboard",
        "//operator/terraform/aws/modules/jobqueue",
        "//operator/terraform/aws/modules/metadatadb",
        "//operator/terraform/aws/modules/parameters",
        "//operator/terraform/aws/modules/worker",
        "//operator/terraform/aws/modules/workerdashboard",
    ],
    prefix = "modules",
)
