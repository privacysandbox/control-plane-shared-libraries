package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "core_utils",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
    ),
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/core/interface:type_def_lib",
        "//cc/public/core/interface:execution_result",
        "@boringssl//:crypto",
    ],
)
