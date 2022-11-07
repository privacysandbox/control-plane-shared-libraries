package(default_visibility = ["//cc:scp_internal_pkg"])

cc_test(
    name = "ec2_error_converter_test",
    size = "small",
    srcs =
        ["ec2_error_converter_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/cpio/client_providers/instance_client_provider:instance_client_provider_lib",
        "//cc/cpio/common/aws/src:aws_utils_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "aws_instance_client_provider_test",
    size = "small",
    timeout = "long",
    srcs =
        ["aws_instance_client_provider_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/cpio/client_providers/instance_client_provider:instance_client_provider_lib",
        "//cc/cpio/client_providers/instance_client_provider/aws/mock:aws_instance_client_provider_mock",
        "//cc/cpio/common/aws/src:aws_utils_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
