load("@rules_pkg//:mappings.bzl", "pkg_files")

package(default_visibility = ["//visibility:public"])

pkg_files(
    name = "frontenddashboard",
    srcs = glob(
        ["*"],
        exclude = [
            "BUILD",
        ],
    ),
    prefix = "frontenddashboard",
)
