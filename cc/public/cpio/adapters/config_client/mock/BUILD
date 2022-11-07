package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "config_client_mock",
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
        "//cc/cpio/client_providers/config_client_provider/mock:config_client_provider_mock",
        "//cc/public/cpio/interface/config_client",
    ],
)
