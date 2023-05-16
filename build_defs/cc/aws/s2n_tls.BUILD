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
    name = "s2n_tls",
    cache_entries = {
        "BUILD_TESTING": "0",
        "DISABLE_WERROR": "ON",
        "S2N_LIBCRYPTO": "boringssl",
        "BUILD_SHARED_LIBS": "ON",
        "BUILD_S2N": "true",
    },
    includes = [
        "include",
    ],
    lib_source = ":all_srcs",
    out_shared_libs = [
        "libs2n.so",
        "libs2n.so.1",
        "libs2n.so.1.0.0",
    ],
    deps = [
        "@boringssl//:crypto",
        "@boringssl//:ssl",
    ],
)
