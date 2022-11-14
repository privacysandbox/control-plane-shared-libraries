package(default_visibility = ["//cc:scp_internal_pkg"])

exports_files([
    "lib_cpio_provider.h",
    "lib_cpio_provider.cc",
])

filegroup(
    name = "lib_cpio_provider_srcs",
    srcs = [
        ":lib_cpio_provider.cc",
        ":lib_cpio_provider.h",
    ],
)

cc_library(
    name = "cpio_provider_lib",
    srcs = select({
        "//cc/public/cpio/interface:aws_cpio_lib": [":lib_cpio_provider_srcs"],
        "//conditions:default": [":lib_cpio_provider_srcs"],
    }) + ["//cc/cpio/client_providers/interface:cpio_provider_interface.h"],
    copts = [
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
        "//cc/public/cpio/interface:type_def",
    ],
)
