package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "test_aws_utils_lib",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
    ),
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/public/cpio/interface:cpio_errors",
        "@aws_sdk_cpp//:core",
    ],
)
