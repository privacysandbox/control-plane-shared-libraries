package(default_visibility = ["//cc:scp_internal_pkg"])

cc_binary(
    name = "config_client_test",
    srcs = ["config_client_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/test/utils:utils_lib",
        "//cc/public/cpio/interface:cpio",
        "//cc/public/cpio/interface/config_client",
        "@aws_sdk_cpp//:core",
    ],
)

cc_binary(
    name = "metric_client_test",
    srcs = ["metric_client_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/test/utils:utils_lib",
        "//cc/public/cpio/interface:cpio",
        "//cc/public/cpio/interface/metric_client",
        "@aws_sdk_cpp//:core",
    ],
)
