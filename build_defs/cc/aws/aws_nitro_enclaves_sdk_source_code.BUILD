package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

cc_library(
    name = "aws_nitro_enclaves_sdk",
    srcs = glob([
        "source/attestation.c",
        "source/cms.c",
        "source/kms.c",
        "source/nitro_enclaves.c",
        "source/rest.c",
    ]),
    hdrs = glob([
        "include/aws/nitro_enclaves/attestation.h",
        "include/aws/nitro_enclaves/exports.h",
        "include/aws/nitro_enclaves/kms.h",
        "include/aws/nitro_enclaves/nitro_enclaves.h",
        "include/aws/nitro_enclaves/rest.h",
        "include/aws/nitro_enclaves/internal/cms.h",
    ]),
    defines = [
    ],
    includes = [
        "include",
    ],
    deps = [
        "@aws_c_auth",
        "@aws_c_common",
        "@aws_c_http",
        "@aws_c_io",
        "@aws_nitro_enclaves_nsm_api",
        "@json_c",
    ],
)
