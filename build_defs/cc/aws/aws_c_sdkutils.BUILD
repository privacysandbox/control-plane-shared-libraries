# Description:
#   AWS C SDK Utilities

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

cc_library(
    name = "aws_c_sdkutils",
    srcs = glob([
        "include/aws/sdkutils/*.h",
        "source/*.c",
    ]),
    includes = [
        "include",
    ],
    deps = [
        "@aws_c_common",
    ],
)
