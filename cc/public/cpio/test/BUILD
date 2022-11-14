package(default_visibility = ["//visibility:public"])

cc_library(
    name = "test_aws_metric_client_options",
    hdrs = [
        "test_aws_metric_client_options.h",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/public/cpio/interface/metric_client:type_def",
    ],
)

cc_library(
    name = "test_aws_config_client_options",
    hdrs = [
        "test_aws_config_client_options.h",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/public/cpio/interface/config_client:type_def",
    ],
)

cc_library(
    name = "test_lib_cpio",
    srcs = ["//cc/public/cpio/core/src:cpio.cc"],
    hdrs = [
        "//cc/public/cpio/interface:cpio.h",
    ],
    copts = [
        "-DTEST_CPIO=1",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/core/logger/src:logger_lib",
        "//cc/core/logger/src/log_providers:log_providers_lib",
        "//cc/core/logger/src/log_providers/syslog:syslog_lib",
        "//cc/cpio/client_providers/global_cpio/test:test_global_cpio_lib",
        "//cc/cpio/client_providers/global_cpio/test:test_lib_cpio_provider",
    ],
)

cc_library(
    name = "test_aws_metric_client",
    srcs = [
        "//cc/public/cpio/adapters/metric_client/src:metric_client_srcs",
        "//cc/public/cpio/adapters/metric_client/test:test_aws_metric_client_srcs",
    ],
    hdrs =
        [
            "//cc/public/cpio/interface/metric_client:metric_client_interface.h",
        ],
    deps = [
        ":test_aws_metric_client_options",
        "//cc:cc_base_include_dir",
        "//cc/core/interface:async_context_lib",
        "//cc/core/interface:interface_lib",
        "//cc/core/utils/src:core_utils",
        "//cc/cpio/client_providers/metric_client_provider/aws/test:test_aws_metric_client_provider_lib",
        "//cc/cpio/proto:cc_proto",
        "//cc/public/cpio/interface:type_def",
        "//cc/public/cpio/interface/metric_client:type_def",
    ],
)

cc_library(
    name = "test_aws_config_client",
    srcs = [
        "//cc/public/cpio/adapters/config_client/src:config_client_srcs",
        "//cc/public/cpio/adapters/config_client/test:test_aws_config_client_srcs",
    ],
    hdrs =
        [
            "//cc/public/cpio/interface/config_client:config_client_interface.h",
        ],
    deps = [
        ":test_aws_config_client_options",
        "//cc:cc_base_include_dir",
        "//cc/core/interface:async_context_lib",
        "//cc/core/interface:interface_lib",
        "//cc/core/utils/src:core_utils",
        "//cc/cpio/client_providers/config_client_provider/aws/test:test_aws_config_client_provider_lib",
        "//cc/cpio/proto:cc_proto",
        "//cc/public/cpio/interface:type_def",
        "//cc/public/cpio/interface/config_client:type_def",
    ],
)
