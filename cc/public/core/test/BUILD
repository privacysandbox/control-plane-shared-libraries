package(default_visibility = ["//cc:scp_internal_pkg"])

cc_test(
    name = "errors_test",
    size = "small",
    srcs = ["errors_test.cc"],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/public/core/interface:errors",
        "//cc/public/cpio/interface:cpio_errors",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "execution_result_test",
    size = "small",
    srcs = ["execution_result_test.cc"],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/common/proto:core_common_proto_lib",
        "//cc/public/core/interface:execution_result",
        "@com_google_googletest//:gtest_main",
    ],
)
