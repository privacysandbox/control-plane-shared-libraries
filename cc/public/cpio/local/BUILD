package(default_visibility = ["//visibility:public"])

cc_library(
    name = "local_cpio_options",
    hdrs = [
        "local_cpio_options.h",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/public/cpio/interface:type_def",
    ],
)

cc_library(
    name = "local_lib_cpio",
    srcs = [
        "local_lib_cpio.cc",
        "//cc/public/cpio/core/src:cpio.cc",
    ],
    hdrs = [
        "local_lib_cpio.h",
        "//cc/public/cpio/interface:cpio.h",
    ],
    copts = [
        "-DLOCAL_CPIO=1",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/core/logger/src:logger_lib",
        "//cc/core/logger/src/log_providers:log_providers_lib",
        "//cc/core/logger/src/log_providers/syslog:syslog_lib",
        "//cc/cpio/client_providers/global_cpio/test:local_global_cpio_lib",
        "//cc/cpio/client_providers/global_cpio/test:local_lib_cpio_provider",
    ],
)
