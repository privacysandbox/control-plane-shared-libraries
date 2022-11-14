package(default_visibility = ["//cc:scp_internal_pkg"])

exports_files([
    "cpio_provider_interface.h",
    "instance_client_provider_interface.h",
    "config_client_provider_interface.h",
    "metric_client_provider_interface.h",
    "private_key_client_provider_interface.h",
    "kms_client_provider_interface.h",
])

cc_library(
    name = "type_def",
    srcs =
        [
            "type_def.h",
        ],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/cpio/proto:cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)
