package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "global_cpio_mock",
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
        "//cc/cpio/client_providers/global_cpio/src/cpio_provider:cpio_provider_lib",
        "//cc/cpio/client_providers/instance_client_provider/mock:instance_client_provider_mock",
    ],
)
