package(default_visibility = ["//cc:scp_internal_pkg"])

exports_files(["test_aws_metric_client.h"])

filegroup(
    name = "test_aws_metric_client_srcs",
    srcs = [
        ":test_aws_metric_client.h",
    ],
)

cc_test(
    name = "metric_client_test",
    size = "small",
    timeout = "long",
    srcs = ["metric_client_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/core/test/utils:utils_lib",
        "//cc/cpio/client_providers/metric_client_provider/src:metric_client_provider_lib",
        "//cc/cpio/proto:cc_proto",
        "//cc/public/cpio/adapters/metric_client/mock:metric_client_mock",
        "//cc/public/cpio/core/mock:lib_cpio_mock",
        "//cc/public/cpio/interface:cpio_errors",
        "//cc/public/cpio/interface:type_def_lib",
        "//cc/public/cpio/interface/metric_client",
        "//cc/public/cpio/interface/metric_client:type_def",
        "@com_google_googletest//:gtest_main",
    ],
)
