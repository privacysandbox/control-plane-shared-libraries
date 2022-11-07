package(default_visibility = ["//cc:scp_internal_pkg"])

cc_test(
    name = "lib_cpio_test",
    size = "small",
    srcs = ["lib_cpio_test.cc"],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/cpio/client_providers/global_cpio/test:test_global_cpio_lib",
        "//cc/public/cpio/test:test_lib_cpio",
        "@aws_sdk_cpp//:core",
        "@com_google_googletest//:gtest_main",
    ],
)
