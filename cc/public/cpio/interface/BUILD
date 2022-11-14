load("@bazel_skylib//rules:common_settings.bzl", "bool_flag", "string_flag")

package(default_visibility = ["//visibility:public"])

# Flag to determine which platform the system will be built and run on.
# Example:
# bazel build target --//cc/cpio:platform=aws
string_flag(
    name = "platform",
    build_setting_default = "aws",
    values = [
        "aws",
    ],
)

# Flag to determine if we use CPIO server or native library.
bool_flag(
    name = "use_cpio_server",
    build_setting_default = False,
)

config_setting(
    name = "aws_cpio_lib",
    flag_values = {
        ":platform": "aws",
        ":use_cpio_server": "False",
    },
)

config_setting(
    name = "aws_cpio_server",
    flag_values = {
        ":platform": "aws",
        ":use_cpio_server": "True",
    },
)

cc_library(
    name = "type_def",
    hdrs =
        [
            "type_def.h",
        ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
    ],
)

cc_library(
    name = "cpio",
    srcs = ["//cc/public/cpio/core/src:cpio.cc"],
    hdrs = [
        "cpio.h",
    ],
    deps = [
        ":type_def",
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/core/logger/src:logger_lib",
        "//cc/core/logger/src/log_providers:log_providers_lib",
        "//cc/core/logger/src/log_providers/syslog:syslog_lib",
        "//cc/cpio/client_providers/global_cpio/src:global_cpio_lib",
        "//cc/cpio/client_providers/global_cpio/src/cpio_provider:cpio_provider_lib",
    ],
)

exports_files(["cpio.h"])

cc_library(
    name = "cpio_errors",
    hdrs = [
        "error_codes.h",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
    ],
)
