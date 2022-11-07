package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "private_key_client_provider_mock",
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
        "//cc/cpio/client_providers/private_key_client_provider/src:private_key_client_provider_lib",
        "//cc/cpio/proto:cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)
