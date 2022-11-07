package(default_visibility = ["//cc:scp_internal_pkg"])

cc_test(
    name = "time_provider_test",
    size = "small",
    srcs =
        [
            "time_provider_test.cc",
        ],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/async_executor/mock:core_async_executor_mock",
        "//cc/core/test/utils:utils_lib",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)
