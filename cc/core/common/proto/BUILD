load("@rules_cc//cc:defs.bzl", "cc_proto_library")

package(default_visibility = ["//cc:scp_internal_pkg"])

proto_library(
    name = "core_common_proto",
    srcs = glob(
        [
            "*.proto",
        ],
    ),
)

cc_proto_library(
    name = "core_common_proto_lib",
    deps = [":core_common_proto"],
)
