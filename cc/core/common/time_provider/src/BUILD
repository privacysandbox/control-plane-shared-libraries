package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "time_provider_lib",
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
        "//cc/core/interface:type_def_lib",
        "//cc/public/core/interface:execution_result",
    ],
)
