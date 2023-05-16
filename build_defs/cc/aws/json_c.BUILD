# Description:
#   JSON implementation in C

load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
    visibility = ["//visibility:public"],
)

# We use cmake instead of bazel as the cmake file in this repo is complex.
cmake(
    name = "json_c",
    cache_entries = {
        "BUILD_TESTING": "0",
        "DISABLE_WERROR": "ON",
    },
    lib_source = ":all_srcs",
    out_shared_libs = [
        "libjson-c.so",
        "libjson-c.so.5",
        "libjson-c.so.5.2.0",
    ],
)
