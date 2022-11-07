package(default_visibility = ["//cc:scp_internal_pkg"])

cc_test(
    name = "aggregate_metric_test",
    size = "small",
    srcs = ["aggregate_metric_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/async_executor/mock:core_async_executor_mock",
        "//cc/core/interface:interface_lib",
        "//cc/core/message_router/src:message_router_lib",
        "//cc/core/test/utils:utils_lib",
        "//cc/cpio/client_providers/metric_client_provider/metric_utils/interface:metric_utils_interface_lib",
        "//cc/cpio/client_providers/metric_client_provider/metric_utils/mock:metric_utils_mock",
        "//cc/cpio/client_providers/metric_client_provider/metric_utils/src:metric_utils_lib",
        "//cc/cpio/client_providers/metric_client_provider/mock:metric_client_provider_mock",
        "//cc/cpio/client_providers/metric_client_provider/src:metric_client_provider_lib",
        "//cc/cpio/proto:cc_proto",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "timeseries_metric_test",
    size = "small",
    srcs = ["timeseries_metric_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/async_executor/mock:core_async_executor_mock",
        "//cc/core/interface:interface_lib",
        "//cc/core/message_router/src:message_router_lib",
        "//cc/core/test/utils:utils_lib",
        "//cc/cpio/client_providers/metric_client_provider/metric_utils/interface:metric_utils_interface_lib",
        "//cc/cpio/client_providers/metric_client_provider/metric_utils/mock:metric_utils_mock",
        "//cc/cpio/client_providers/metric_client_provider/metric_utils/src:metric_utils_lib",
        "//cc/cpio/client_providers/metric_client_provider/mock:metric_client_provider_mock",
        "//cc/cpio/client_providers/metric_client_provider/src:metric_client_provider_lib",
        "//cc/cpio/proto:cc_proto",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "simple_metric_test",
    size = "small",
    srcs = ["simple_metric_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/async_executor/mock:core_async_executor_mock",
        "//cc/core/interface:interface_lib",
        "//cc/core/message_router/src:message_router_lib",
        "//cc/core/test/utils:utils_lib",
        "//cc/cpio/client_providers/metric_client_provider/metric_utils/interface:metric_utils_interface_lib",
        "//cc/cpio/client_providers/metric_client_provider/metric_utils/mock:metric_utils_mock",
        "//cc/cpio/client_providers/metric_client_provider/metric_utils/src:metric_utils_lib",
        "//cc/cpio/client_providers/metric_client_provider/mock:metric_client_provider_mock",
        "//cc/cpio/client_providers/metric_client_provider/src:metric_client_provider_lib",
        "//cc/cpio/proto:cc_proto",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "metric_utils_test",
    size = "small",
    srcs = ["metric_utils_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/async_executor/mock:core_async_executor_mock",
        "//cc/core/interface:interface_lib",
        "//cc/core/message_router/src:message_router_lib",
        "//cc/core/test/utils:utils_lib",
        "//cc/cpio/client_providers/metric_client_provider/metric_utils/mock:metric_utils_mock",
        "//cc/cpio/client_providers/metric_client_provider/metric_utils/src:metric_utils_lib",
        "//cc/cpio/proto:cc_proto",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)
