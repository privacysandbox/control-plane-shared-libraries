package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "auto_expiry_concurrent_map_mock",
    srcs = glob(
        [
            "**/*.cc",
            "**/*.h",
        ],
    ),
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/common/auto_expiry_concurrent_map/src:auto_expiry_concurrent_map_lib",
        "//cc/core/interface:interface_lib",
    ],
)
