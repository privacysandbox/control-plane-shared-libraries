# Description:
#   AWS C Huffman Encoding/Decoding

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

cc_library(
    name = "aws_c_compression",
    srcs = glob([
        "include/aws/compression/*.h",
        "include/aws/compression/private/*.h",
        "source/huffman_generator/*.c",
        "source/*.c",
    ]),
    includes = [
        "include",
    ],
    deps = [
        "@aws_c_common",
    ],
)
