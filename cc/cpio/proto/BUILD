load(
    "@com_google_googleapis_imports//:imports.bzl",
    "cc_grpc_library",
    "cc_proto_library",
)

package(default_visibility = ["//cc:scp_internal_pkg"])

proto_library(
    name = "proto",
    srcs = glob(
        [
            "*.proto",
        ],
        exclude = [
            "cpio_service.proto",
        ],
    ),
)

proto_library(
    name = "cpio_service_proto",
    srcs = ["cpio_service.proto"],
    deps = [
        "//cc/core/common/proto:core_common_proto",
        "@com_google_protobuf//:any_proto",
    ],
)

##############################################################################
# C++
##############################################################################
cc_proto_library(
    name = "cc_proto",
    deps = [":proto"],
)

cc_proto_library(
    name = "cpio_service_cc_proto",
    deps = [":cpio_service_proto"],
)

cc_grpc_library(
    name = "cpio_service_cc_grpc",
    srcs = [":cpio_service_proto"],
    generate_mocks = True,
    grpc_only = True,
    deps = [":cpio_service_cc_proto"],
)
