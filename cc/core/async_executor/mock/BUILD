package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "core_async_executor_mock",
    srcs = glob(
        [
            "**/*.h",
        ],
    ),
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/async_executor/src:core_async_executor_lib",
        "//cc/core/interface:interface_lib",
    ],
)
