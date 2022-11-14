package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "test_aws_config_client_provider_lib",
    srcs = select({
        "//cc/public/cpio/interface:aws_cpio_lib": ["//cc/cpio/client_providers/config_client_provider/aws/src:aws_config_client_provider_srcs"],
        "//conditions:default": ["//cc/cpio/client_providers/config_client_provider/aws/src:aws_config_client_provider_srcs"],
    }) + [
        "//cc/cpio/client_providers/interface:config_client_provider_interface.h",
        "//cc/cpio/client_providers/config_client_provider/src:config_client_provider_srcs",
        "test_aws_config_client_provider.h",
        "test_aws_config_client_provider.cc",
    ],
    copts = [
        "-DTEST_CPIO=1",
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/core/message_router/src:message_router_lib",
        "//cc/cpio/client_providers/global_cpio/test:test_global_cpio_lib",
        "//cc/cpio/client_providers/interface:type_def",
        "//cc/cpio/common/aws/src:aws_utils_lib",
        "//cc/cpio/common/aws/test:test_aws_utils_lib",
        "//cc/cpio/proto:cc_proto",
        "//cc/public/cpio/interface/config_client:type_def",
        "//cc/public/cpio/test:test_aws_config_client_options",
        "@aws_sdk_cpp//:ssm",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "ssm_error_converter_test",
    size = "small",
    srcs =
        ["ssm_error_converter_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/cpio/client_providers/config_client_provider/src:config_client_provider_lib",
        "//cc/cpio/common/aws/src:aws_utils_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "aws_config_client_provider_test",
    size = "small",
    timeout = "long",
    srcs =
        ["aws_config_client_provider_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/cpio/client_providers/config_client_provider/aws/mock:aws_config_client_provider_mock",
        "//cc/cpio/client_providers/config_client_provider/src:config_client_provider_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
