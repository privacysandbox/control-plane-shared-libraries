# Description:
#   AWS C HTTP/1.1 and HTTP/2 implementation

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

cc_library(
    name = "aws_c_http",
    srcs = glob([
        "include/aws/http/*.h",
        "include/aws/http/private/*.h",
        "source/*.c",
    ]),
    hdrs = glob([
        "include/aws/http/private/*.def",
    ]),
    includes = [
        "include",
    ],
    deps = [
        "@aws_c_cal",
        "@aws_c_common",
        "@aws_c_compression",
        "@aws_c_io",
    ],
)
