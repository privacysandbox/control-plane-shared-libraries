package(default_visibility = ["//visibility:public"])

cc_library(
    name = "type_def",
    hdrs =
        [
            "type_def.h",
        ],
    deps = [
        "//cc:cc_base_include_dir",
    ],
)

cc_library(
    name = "metric_client",
    srcs = ["//cc/public/cpio/adapters/metric_client/src:metric_client_srcs"],
    hdrs =
        [
            "metric_client_interface.h",
        ],
    deps = [
        ":type_def",
        "//cc:cc_base_include_dir",
        "//cc/core/interface:async_context_lib",
        "//cc/core/interface:interface_lib",
        "//cc/core/utils/src:core_utils",
        "//cc/cpio/client_providers/metric_client_provider/src:metric_client_provider_lib",
        "//cc/cpio/proto:cc_proto",
        "//cc/public/cpio/interface:type_def",
    ],
)

exports_files(["metric_client_interface.h"])
