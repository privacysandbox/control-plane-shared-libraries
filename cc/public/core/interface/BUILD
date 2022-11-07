package(default_visibility = ["//visibility:public"])

cc_library(
    name = "errors",
    srcs = ["//cc/public/core/src:errors.cc"],
    hdrs =
        [
            "errors.h",
        ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
    ],
)

cc_library(
    name = "execution_result",
    srcs = ["//cc/public/core/src:execution_result.cc"],
    hdrs =
        [
            "execution_result.h",
        ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/common/proto:core_common_proto_lib",
    ],
)
