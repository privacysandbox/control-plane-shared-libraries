package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "metric_client_mock",
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
        "//cc/core/async_executor/mock:core_async_executor_mock",
        "//cc/cpio/client_providers/metric_client_provider/mock:metric_client_provider_mock",
        "//cc/public/cpio/interface/metric_client",
    ],
)
