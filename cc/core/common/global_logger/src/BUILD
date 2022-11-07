package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "global_logger_lib",
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
        "//cc/core/interface:errors_lib",
        "//cc/core/interface:interface_lib",
    ],
)
