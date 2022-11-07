package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "docker_helper_lib",
    srcs = glob(
        [
            "*.cc",
            "*.h",
        ],
        exclude = [
            "docker_helper_test.cc",
        ],
    ),
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "docker_helper_test",
    size = "small",
    srcs = ["docker_helper_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        ":docker_helper_lib",
        "//cc:cc_base_include_dir",
        "@com_google_googletest//:gtest_main",
    ],
)
