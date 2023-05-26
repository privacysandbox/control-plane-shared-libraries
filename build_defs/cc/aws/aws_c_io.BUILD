# Description:
#   AWS C I/O and TLS work for application protocols

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

cc_library(
    name = "aws_c_io",
    srcs = glob([
        "include/aws/io/*.h",
        "include/aws/io/private/*.h",
        "source/linux/*.c",
        "source/pkcs11/v2.40/*.h",
        "source/posix/*.c",
        "source/s2n/*.c",
        "source/*.c",
        "source/*.h",
    ]),
    defines = ["USE_S2N"],
    includes = [
        "include",
    ],
    deps = [
        "@aws_c_cal",
        "@aws_c_common",
        "@s2n_tls",
    ],
)
