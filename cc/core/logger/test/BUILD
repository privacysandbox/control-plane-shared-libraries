cc_test(
    name = "logger_test",
    size = "small",
    srcs = ["logger_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/core/logger/interface:logger_interface_lib",
        "//cc/core/logger/mock:logger_mock",
        "//cc/core/logger/src:logger_lib",
        "//cc/core/test:core_test_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
