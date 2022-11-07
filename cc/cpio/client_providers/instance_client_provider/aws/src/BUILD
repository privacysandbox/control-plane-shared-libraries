package(default_visibility = ["//cc:scp_internal_pkg"])

exports_files([
    "aws_instance_client_provider.h",
    "aws_instance_client_provider.cc",
    "ec2_error_converter.h",
    "ec2_error_converter.cc",
    "error_codes.h",
])

filegroup(
    name = "aws_instance_client_provider_srcs",
    srcs = [
        ":aws_instance_client_provider.cc",
        ":aws_instance_client_provider.h",
        ":ec2_error_converter.cc",
        ":ec2_error_converter.h",
        ":error_codes.h",
    ],
)
