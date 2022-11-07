package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "metric_utils_lib",
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
        "//cc/core/message_router/src:message_router_lib",
        "//cc/cpio/client_providers/metric_client_provider/metric_utils/interface:metric_utils_interface_lib",
        "//cc/cpio/client_providers/metric_client_provider/src:metric_client_provider_lib",
        "//cc/cpio/proto:cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)
