package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "message_router_lib",
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
        "//cc/core/interface:async_context_lib",
        "//cc/core/interface:interface_lib",
        "@com_google_protobuf//:protobuf",
    ],
)
