package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "aws_metric_client_provider_mock",
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
        "//cc/cpio/client_providers/instance_client_provider/mock:instance_client_provider_mock",
        "//cc/cpio/client_providers/metric_client_provider/src:metric_client_provider_lib",
        "//cc/cpio/proto:cc_proto",
        "@aws_sdk_cpp//:monitoring",
        "@com_google_absl//absl/algorithm:container",
        "@com_google_protobuf//:protobuf",
    ],
)
