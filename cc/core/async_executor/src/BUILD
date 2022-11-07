package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "core_async_executor_lib",
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
        "//cc/core/common/concurrent_queue/src:concurrent_queue_lib",
        "//cc/core/common/global_logger/src:global_logger_lib",
        "//cc/core/interface:async_context_lib",
        "//cc/core/interface:interface_lib",
        "//cc/core/test:core_test_lib",
    ],
)
