package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "interface_lib",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
        exclude = [
            "errors.cc",
            "errors.h",
            "async_context.h",
            "type_def.h",
        ],
    ),
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/common/concurrent_map/src:concurrent_map_lib",
        "//cc/core/common/proto:core_common_proto_lib",
        "//cc/core/common/uuid/src:uuid_lib",
    ],
)

cc_library(
    name = "async_context_lib",
    srcs =
        [
            "async_context.h",
        ],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/common/global_logger/src:global_logger_lib",
        "//cc/core/common/uuid/src:uuid_lib",
    ],
)

cc_library(
    name = "type_def_lib",
    srcs =
        [
            "type_def.h",
        ],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/common/proto:core_common_proto_lib",
    ],
)

cc_library(
    name = "errors_lib",
    srcs =
        [
            "errors.cc",
            "errors.h",
        ],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/public/core/interface:execution_result",
    ],
)
