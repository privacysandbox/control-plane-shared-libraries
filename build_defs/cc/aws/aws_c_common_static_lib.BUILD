package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

cc_library(
    name = "aws_c_common",
    srcs = glob(
        [
            "lib/libaws-c-common.a",
        ],
    ),
    hdrs = glob(
        [
            "include/aws/common/*.h",
            "include/aws/common/external/*.h",
            "include/aws/common/private/*.h",
        ],
    ),
    defines = [
        "AWS_AFFINITY_METHOD",
    ],
    includes = [
        "include",
    ],
    linkopts = ["-ldl"],
    textual_hdrs = glob([
        "include/**/*.inl",
    ]),
)
