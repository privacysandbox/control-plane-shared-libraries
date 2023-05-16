# Description:
#   AWS C Client-side Authentication

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

cc_library(
    name = "aws_c_auth",
    srcs = glob([
        "include/aws/auth/*.h",
        "include/aws/auth/external/*.h",
        "include/aws/auth/private/*.h",
        "source/external/*.c",
        "source/*.c",
    ]),
    includes = [
        "include",
    ],
    deps = [
        "@aws_c_compression",
        "@aws_c_http",
        "@aws_c_io",
        "@aws_c_sdkutils",
    ],
)
