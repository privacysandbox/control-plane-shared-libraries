package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

cc_library(
    name = "aws_lc",
    srcs = glob(
        [
            "lib/libcrypto.a",
            "lib/libssl.a",
            "lib/libdecrepit.a",
        ],
    ),
    hdrs = glob(
        [
            "include/openssl/*.h",
        ],
    ),
    includes = [
        "include",
    ],
    deps = [
    ],
)

cc_library(
    name = "s2n_tls",
    srcs = glob(
        [
            "lib/libs2n.a",
        ],
    ),
    hdrs = glob(
        [
            "include/s2n.h",
        ],
    ),
    includes = [
        "include",
    ],
    deps = [
    ],
)

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
        ],
    ),
    defines = [
        "AWS_AFFINITY_METHOD",
    ],
    includes = [
        "include",
    ],
    textual_hdrs = glob(
        [
            "include/aws/common/error.inl",
        ],
    ),
    deps = [
    ],
)

cc_library(
    name = "aws_c_sdkutils",
    srcs = glob(
        [
            "lib/libaws-c-sdkutils.a",
        ],
    ),
    hdrs = glob(
        [
            "include/aws/sdkutils/*.h",
        ],
    ),
    includes = [
        "include",
    ],
    deps = [
        ":aws_c_common",
    ],
)

cc_library(
    name = "aws_c_io",
    srcs = glob(
        [
            "lib/libaws-c-io.a",
        ],
    ),
    hdrs = glob(
        [
            "include/aws/io/*.h",
        ],
    ),
    includes = [
        "include",
    ],
    deps = [
        ":aws_c_common",
    ],
)

cc_library(
    name = "aws_c_compression",
    srcs = glob(
        [
            "lib/libaws-c-compression.a",
        ],
    ),
    hdrs = glob(
        [
            "include/aws/compression/*.h",
        ],
    ),
    includes = [
        "include",
    ],
    deps = [
        ":aws_c_common",
    ],
)

cc_library(
    name = "aws_c_cal",
    srcs = glob(
        [
            "lib/libaws-c-cal.a",
        ],
    ),
    hdrs = glob(
        [
            "include/aws/cal/*.h",
        ],
    ),
    includes = [
        "include",
    ],
    deps = [
        ":aws_c_common",
        ":aws_lc",
    ],
)

cc_library(
    name = "aws_c_http",
    srcs = glob(
        [
            "lib/libaws-c-http.a",
        ],
    ),
    hdrs = glob(
        [
            "include/aws/http/*.h",
        ],
    ),
    includes = [
        "include",
    ],
    deps = [
        ":aws_c_cal",
        ":aws_c_common",
        ":aws_c_compression",
        ":aws_c_io",
        ":aws_lc",
        ":s2n_tls",
    ],
)

cc_library(
    name = "aws_c_auth",
    srcs = glob(
        [
            "lib/libaws-c-auth.a",
        ],
    ),
    hdrs = glob(
        [
            "include/aws/auth/*.h",
        ],
    ),
    includes = [
        "include",
    ],
    deps = [
        ":aws_c_http",
    ],
)

cc_library(
    name = "json_c",
    srcs = glob(
        [
            "lib/libjson-c.a",
        ],
    ),
    hdrs = glob(
        [
            "include/json-c/*.h",
        ],
    ),
    includes = [
        "include",
    ],
    deps = [
    ],
)

cc_library(
    name = "aws_nitro_enclaves_nsm_api",
    srcs = glob(
        [
            "lib/libnsm.so",
        ],
    ),
    hdrs = glob(
        [
            "include/nsm.h",
        ],
    ),
    includes = [
        "include",
    ],
    deps = [
    ],
)

cc_library(
    name = "aws_nitro_enclaves_sdk",
    srcs = glob(
        [
            "lib/libaws-nitro-enclaves-sdk-c.a",
        ],
    ),
    hdrs = glob(
        [
            "include/aws/nitro_enclaves/attestation.h",
            "include/aws/nitro_enclaves/exports.h",
            "include/aws/nitro_enclaves/kms.h",
            "include/aws/nitro_enclaves/nitro_enclaves.h",
            "include/aws/nitro_enclaves/rest.h",
        ],
    ),
    includes = [
        "include",
    ],
    deps = [
        ":aws_c_auth",
        ":aws_c_sdkutils",
        ":aws_nitro_enclaves_nsm_api",
    ],
)
