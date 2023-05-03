package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE"])

cc_library(
    name = "aws-lc",
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
    name = "s2n-tls",
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
    name = "aws-c-common",
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
    name = "aws-c-sdkutils",
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
        ":aws-c-common",
    ],
)

cc_library(
    name = "aws-c-io",
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
        ":aws-c-common",
    ],
)

cc_library(
    name = "aws-c-compression",
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
        ":aws-c-common",
    ],
)

cc_library(
    name = "aws-c-cal",
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
        ":aws-c-common",
        ":aws-lc",
    ],
)

cc_library(
    name = "aws-c-http",
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
        ":aws-c-cal",
        ":aws-c-common",
        ":aws-c-compression",
        ":aws-c-io",
        ":aws-lc",
        ":s2n-tls",
    ],
)

cc_library(
    name = "aws-c-auth",
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
        ":aws-c-http",
    ],
)

cc_library(
    name = "json-c",
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
    name = "aws-nitro-enclaves-nsm-api",
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
    name = "aws-nitro-enclaves-sdk",
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
        ":aws-c-auth",
        ":aws-c-sdkutils",
        ":aws-nitro-enclaves-nsm-api",
    ],
)
