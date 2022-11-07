package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "aws_config_client_provider_mock",
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
        "//cc/cpio/client_providers/instance_client_provider/aws/mock:aws_instance_client_provider_mock",
        "//cc/cpio/client_providers/instance_client_provider/mock:instance_client_provider_mock",
        "@aws_sdk_cpp//:ssm",
        "@com_google_absl//absl/algorithm:container",
    ],
)
