package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "aws_instance_client_provider_mock",
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
        "@aws_sdk_cpp//:ec2",
    ],
)
