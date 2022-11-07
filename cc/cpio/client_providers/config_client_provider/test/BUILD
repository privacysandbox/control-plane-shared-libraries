package(default_visibility = ["//cc:scp_internal_pkg"])

cc_test(
    name = "config_client_provider_test",
    size = "small",
    timeout = "long",
    srcs = ["config_client_provider_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/cpio/client_providers/config_client_provider/mock:config_client_provider_mock",
        "//cc/cpio/client_providers/config_client_provider/src:config_client_provider_lib",
        "//cc/cpio/proto:cc_proto",
        "@com_google_googletest//:gtest_main",
    ],
)
