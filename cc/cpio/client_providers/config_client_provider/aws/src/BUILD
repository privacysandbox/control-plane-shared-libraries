package(default_visibility = ["//cc:scp_internal_pkg"])

exports_files([
    "aws_config_client_provider.h",
    "aws_config_client_provider.cc",
    "ssm_error_converter.h",
    "ssm_error_converter.cc",
    "error_codes.h",
])

filegroup(
    name = "aws_config_client_provider_srcs",
    srcs = [
        ":aws_config_client_provider.cc",
        ":aws_config_client_provider.h",
        ":error_codes.h",
        ":ssm_error_converter.cc",
        ":ssm_error_converter.h",
    ],
)
