package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "concurrent_queue_lib",
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
        "//cc/core/interface:type_def_lib",
        "@oneTBB//:tbb",
    ],
)
