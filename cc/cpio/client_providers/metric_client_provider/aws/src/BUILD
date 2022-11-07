package(default_visibility = ["//cc:scp_internal_pkg"])

exports_files([
    "aws_metric_client_provider.h",
    "aws_metric_client_provider.cc",
    "cloud_watch_error_converter.h",
    "cloud_watch_error_converter.cc",
    "aws_metric_client_utils.h",
    "aws_metric_client_utils.cc",
    "error_codes.h",
])

filegroup(
    name = "aws_metric_client_provider_srcs",
    srcs = [
        ":aws_metric_client_provider.cc",
        ":aws_metric_client_provider.h",
        ":aws_metric_client_utils.cc",
        ":aws_metric_client_utils.h",
        ":cloud_watch_error_converter.cc",
        ":cloud_watch_error_converter.h",
        ":error_codes.h",
    ],
)
