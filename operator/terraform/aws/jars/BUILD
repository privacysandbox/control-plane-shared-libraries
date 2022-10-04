load("@rules_pkg//:pkg.bzl", "pkg_tar")

# Collect all of the jars used in the distribution tar and collect them in a
# single tar to make it easy to extract locally.
pkg_tar(
    name = "jar_tar",
    srcs = [
        "//operator/terraform/aws:dist_jars",
    ],
    extension = "tar",
)
