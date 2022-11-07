package(default_visibility = ["//cc:scp_internal_pkg"])

cc_test(
    name = "concurrent_map_test",
    size = "small",
    srcs = ["concurrent_map_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/common/auto_expiry_concurrent_map/mock:auto_expiry_concurrent_map_mock",
        "//cc/core/common/concurrent_map/src:concurrent_map_lib",
        "//cc/core/interface:type_def_lib",
        "//cc/core/test/utils:utils_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
