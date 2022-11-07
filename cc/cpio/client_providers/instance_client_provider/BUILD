package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "instance_client_provider_lib",
    srcs = select({
        "//cc/public/cpio/interface:aws_cpio_lib": ["//cc/cpio/client_providers/instance_client_provider/aws/src:aws_instance_client_provider_srcs"],
        "//conditions:default": ["//cc/cpio/client_providers/instance_client_provider/aws/src:aws_instance_client_provider_srcs"],
    }) + ["//cc/cpio/client_providers/interface:instance_client_provider_interface.h"],
    copts = [
        "-std=c++17",
    ],
    deps = select({
        "//cc/public/cpio/interface:aws_cpio_lib": [
            "@aws_sdk_cpp//:core",
            "//cc/cpio/common/aws/src:aws_utils_lib",
            "@aws_sdk_cpp//:ec2",
        ],
        "//conditions:default": [
            "@aws_sdk_cpp//:core",
            "//cc/cpio/common/aws/src:aws_utils_lib",
            "@aws_sdk_cpp//:ec2",
        ],
    }) + [
        "//cc:cc_base_include_dir",
        "//cc/core/common/uuid/src:uuid_lib",
        "//cc/core/interface:async_context_lib",
        "//cc/core/interface:interface_lib",
        "//cc/public/cpio/interface:cpio_errors",
    ],
)
