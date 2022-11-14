package(default_visibility = ["//cc:scp_internal_pkg"])

cc_binary(
    name = "config_client_test",
    srcs = ["config_client_test.cc"],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/test/utils:utils_lib",
        "//cc/public/core/interface:errors",
        "//cc/public/cpio/interface:cpio",
        "//cc/public/cpio/interface/config_client",
        "@aws_sdk_cpp//:core",
    ],
)

cc_binary(
    name = "metric_client_test",
    srcs = ["metric_client_test.cc"],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/test/utils:utils_lib",
        "//cc/public/core/interface:errors",
        "//cc/public/cpio/interface:cpio",
        "//cc/public/cpio/interface/metric_client",
        "@aws_sdk_cpp//:core",
    ],
)

cc_binary(
    name = "local_config_client_test",
    srcs = ["local_config_client_test.cc"],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/test/utils:utils_lib",
        "//cc/public/core/interface:errors",
        "//cc/public/cpio/interface/config_client",
        "//cc/public/cpio/local:local_lib_cpio",
        "@aws_sdk_cpp//:core",
    ],
)

cc_binary(
    name = "local_metric_client_test",
    srcs = ["local_metric_client_test.cc"],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/test/utils:utils_lib",
        "//cc/public/core/interface:errors",
        "//cc/public/cpio/interface/metric_client",
        "//cc/public/cpio/local:local_lib_cpio",
        "@aws_sdk_cpp//:core",
    ],
)
