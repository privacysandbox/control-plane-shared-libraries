package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "test_lib_cpio_provider",
    srcs = select({
        "//cc/public/cpio/interface:aws_cpio_lib": ["//cc/cpio/client_providers/global_cpio/src/cpio_provider:lib_cpio_provider_srcs"],
        "//conditions:default": ["//cc/cpio/client_providers/global_cpio/src/cpio_provider:lib_cpio_provider_srcs"],
    }) + [
        "//cc/cpio/client_providers/interface:cpio_provider_interface.h",
        "test_lib_cpio_provider.h",
        "test_lib_cpio_provider.cc",
    ],
    copts = [
        "-DCPIO_TESTING=1",
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/async_executor/src:core_async_executor_lib",
        "//cc/core/logger/src:logger_lib",
        "//cc/core/logger/src/log_providers:log_providers_lib",
        "//cc/core/logger/src/log_providers/syslog:syslog_lib",
        "//cc/core/message_router/src:message_router_lib",
        "//cc/cpio/client_providers/instance_client_provider:instance_client_provider_lib",
        "//cc/cpio/client_providers/instance_client_provider/test:test_instance_client_provider_lib",
    ],
)

cc_library(
    name = "test_global_cpio_lib",
    srcs =
        [
            "//cc/cpio/client_providers/global_cpio/src:global_cpio_srcs",
        ],
    copts = [
        "-DCPIO_TESTING=1",
        "-std=c++17",
    ],
    deps = [
        ":test_lib_cpio_provider",
        "//cc:cc_base_include_dir",
        "//cc/core/logger/src:logger_lib",
        "//cc/core/logger/src/log_providers:log_providers_lib",
        "//cc/core/logger/src/log_providers/syslog:syslog_lib",
    ],
)

cc_test(
    name = "lib_cpio_provider_test",
    size = "small",
    srcs = ["lib_cpio_provider_test.cc"],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/cpio/client_providers/global_cpio/mock:global_cpio_mock",
        "//cc/public/cpio/interface:cpio",
        "@aws_sdk_cpp//:core",
        "@com_google_googletest//:gtest_main",
    ],
)
