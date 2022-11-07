package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "instance_client_provider_mock",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
        [
            "//cc/cpio/client_providers/interface:instance_client_provider_interface.h",
        ],
    ),
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "@aws_sdk_cpp//:ec2",
    ],
)
