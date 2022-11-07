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
    name = "private_key_client",
    hdrs =
        [
            "private_key_client_interface.h",
        ],
    deps = [
        ":type_def",
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/public/cpio/interface:type_def_lib",
    ],
)
