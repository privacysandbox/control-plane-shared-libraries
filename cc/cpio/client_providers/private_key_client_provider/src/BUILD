package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "private_key_client_provider_lib",
    srcs = [
        "private_key_client_provider.cc",
        "private_key_client_provider.h",
        "//cc/cpio/client_providers/interface:private_key_client_provider_interface.h",
    ],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/message_router/src:message_router_lib",
        "//cc/cpio/client_providers/global_cpio/src:global_cpio_lib",
        "//cc/cpio/client_providers/interface:type_def",
        "//cc/cpio/proto:cc_proto",
        "//cc/public/cpio/interface/private_key_client:type_def",
    ],
)

exports_files([
    "private_key_client_provider.h",
    "private_key_client_provider.cc",
])

filegroup(
    name = "private_key_client_provider_srcs",
    srcs = [
        ":private_key_client_provider.cc",
        ":private_key_client_provider.h",
    ],
)
