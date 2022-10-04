load("//build_defs:release.bzl", "gcs_package_release")

package(default_visibility = ["//visibility:public"])

gcs_package_release(
    name = "operator_aws_release",
    artifact_base_name = "scp-operator-{VERSION}.tgz",
    package_target = "//operator/terraform/aws:operator_tar",
    release_bucket = "admcloud-scp-repo-dev",
    release_key = "operator-dms",
)
