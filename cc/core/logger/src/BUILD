package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "logger_lib",
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
        "//cc/core/interface:interface_lib",
        "//cc/core/logger/interface:logger_interface_lib",
    ],
)
