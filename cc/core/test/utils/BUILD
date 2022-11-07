package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "utils_lib",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
    ),
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/common/time_provider/src:time_provider_lib",
        "//cc/core/interface:interface_lib",
        "//cc/core/test:core_test_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
