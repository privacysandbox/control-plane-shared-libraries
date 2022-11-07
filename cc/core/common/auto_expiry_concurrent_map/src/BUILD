package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "auto_expiry_concurrent_map_lib",
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
        "//cc/core/common/concurrent_map/src:concurrent_map_lib",
        "//cc/core/interface:interface_lib",
        "//cc/core/interface:type_def_lib",
        "@oneTBB//:tbb",
    ],
)
