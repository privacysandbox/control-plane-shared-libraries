load("@rules_cc//cc:defs.bzl", "cc_proto_library", "cc_test")

package(default_visibility = ["//cc:scp_internal_pkg"])

proto_library(
    name = "test",
    srcs = ["test.proto"],
)

cc_proto_library(
    name = "test_cc_proto",
    deps = [":test"],
)

cc_test(
    name = "message_router_test",
    size = "small",
    srcs = ["message_router_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        ":test_cc_proto",
        "//cc:cc_base_include_dir",
        "//cc/core/common/concurrent_queue/src:concurrent_queue_lib",
        "//cc/core/interface:interface_lib",
        "//cc/core/message_router/src:message_router_lib",
        "//cc/core/test/utils:utils_lib",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)
