package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "core_test_lib",
    srcs = glob(
        [
            "*.h",
        ],
    ),
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
    ],
)

cc_test(
    name = "service_interface_test",
    size = "small",
    srcs = ["service_interface_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "errors_test",
    size = "small",
    srcs = ["errors_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/common/concurrent_queue/src:concurrent_queue_lib",
        "//cc/core/interface:interface_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
