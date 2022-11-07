package(default_visibility = ["//cc:scp_internal_pkg"])

cc_test(
    name = "concurrent_queue_test",
    size = "small",
    srcs = ["concurrent_queue_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/common/concurrent_queue/src:concurrent_queue_lib",
        "//cc/core/interface:type_def_lib",
        "//cc/core/test/utils:utils_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
