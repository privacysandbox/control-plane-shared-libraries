package(default_visibility = ["//cc:scp_internal_pkg"])

exports_files([
    "global_cpio.h",
    "global_cpio.cc",
])

filegroup(
    name = "global_cpio_srcs",
    srcs = [
        ":global_cpio.cc",
        ":global_cpio.h",
    ],
)

cc_library(
    name = "global_cpio_lib",
    srcs =
        [
            ":global_cpio_srcs",
        ],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/logger/src:logger_lib",
        "//cc/core/logger/src/log_providers:log_providers_lib",
        "//cc/core/logger/src/log_providers/syslog:syslog_lib",
        "//cc/cpio/client_providers/global_cpio/src/cpio_provider:cpio_provider_lib",
    ],
)
