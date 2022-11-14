package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "test_instance_client_provider_lib",
    srcs = [
        "test_instance_client_provider.cc",
        "test_instance_client_provider.h",
        "//cc/cpio/client_providers/interface:instance_client_provider_interface.h",
    ],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
    ],
)

cc_library(
    name = "local_instance_client_provider_lib",
    srcs = [
        "local_instance_client_provider.cc",
        "local_instance_client_provider.h",
        "//cc/cpio/client_providers/interface:instance_client_provider_interface.h",
    ],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/public/cpio/local:local_cpio_options",
    ],
)
