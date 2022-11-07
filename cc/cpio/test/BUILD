package(default_visibility = ["//cc:scp_internal_pkg"])

cc_test(
    name = "aws_cpio_lib_integration_test",
    size = "small",
    timeout = "long",
    srcs = ["aws_cpio_lib_integration_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/test/utils:utils_lib",
        "//cc/core/test/utils/aws_helper:aws_helper_lib",
        "//cc/core/test/utils/docker_helper:docker_helper_lib",
        "//cc/public/core/interface:execution_result",
        "//cc/public/cpio/interface:cpio_errors",
        "//cc/public/cpio/test:test_aws_config_client",
        "//cc/public/cpio/test:test_aws_metric_client",
        "//cc/public/cpio/test:test_lib_cpio",
        "@com_google_googletest//:gtest_main",
    ],
)
