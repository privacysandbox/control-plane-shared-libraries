package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "uuid_lib",
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
        "//cc/core/common/time_provider/src:time_provider_lib",
        "//cc/core/interface:errors_lib",
        "//cc/public/core/interface:execution_result",
    ],
)
