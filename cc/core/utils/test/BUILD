cc_test(
    name = "core_utils_test",
    size = "small",
    srcs = glob(
        [
            "*.cc",
        ],
    ),
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/core/utils/src:core_utils",
        "@com_google_googletest//:gtest_main",
    ],
)
