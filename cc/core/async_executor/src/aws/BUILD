package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "core_aws_async_executor_lib",
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
        "//cc/core/async_executor/src:core_async_executor_lib",
        "@aws_sdk_cpp//:core",
    ],
)
