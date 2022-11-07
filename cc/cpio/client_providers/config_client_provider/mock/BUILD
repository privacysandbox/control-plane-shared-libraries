package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "config_client_provider_mock",
    srcs = glob(
        [
            "**/*.h",
        ],
    ),
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/cpio/client_providers/config_client_provider/src:config_client_provider_lib",
        "//cc/cpio/client_providers/instance_client_provider/mock:instance_client_provider_mock",
        "@com_google_protobuf//:protobuf",
    ],
)
