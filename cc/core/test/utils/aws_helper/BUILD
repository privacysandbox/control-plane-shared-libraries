package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "aws_helper_lib",
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
        "@aws_sdk_cpp//:core",
        "@aws_sdk_cpp//:dynamodb",
        "@aws_sdk_cpp//:s3",
        "@aws_sdk_cpp//:ssm",
    ],
)
