package(default_visibility = ["//visibility:public"])

load("@com_github_bazelbuild_buildtools//buildifier:def.bzl", "buildifier")
load("@rules_pkg//:pkg.bzl", "pkg_tar")

buildifier(
    name = "buildifier_check",
    mode = "check",
)

buildifier(
    name = "buildifier_fix",
    mode = "fix",
)

# This rule is used to copy the source code from other bazel rules.
# This can be used for reproducible builds.
# Only cc targets are needed at this point, so only the files needed to build
# cc targets are copied.
pkg_tar(
    name = "source_code_tar",
    srcs = [
        ".bazelrc",
        "BUILD",
        "WORKSPACE",
        "build_defs",
        "cc",
    ] + glob(["*.bzl"]),
    mode = "0777",
    package_dir = "scp",
)
