load("@rules_cc//cc:defs.bzl", "cc_test")
load("@rules_python//python:defs.bzl", "py_test")

package(default_visibility = ["//visibility:public"])

cc_test(
    name = "server_test",
    size = "small",
    srcs = ["server_test.cc"],
    copts = [
        "-std=c++17",
    ],
    flaky = True,  # Remove this flag once fixed. (see b/200961920 for details)
    deps = [
        "//cc/aws/proxy/src:proxy_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "socks5_state_test",
    size = "small",
    srcs = ["socks5_state_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc/aws/proxy/src:proxy_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "synchronization_test",
    size = "small",
    srcs = ["synchronization_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc/aws/proxy/src:proxy_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "socks5_state_input_test",
    size = "small",
    srcs = ["socks5_state_input_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc/aws/proxy/src:proxy_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "protocol_test",
    size = "small",
    srcs = ["protocol_test.cc"],
    copts = [
        "-std=c++17",
    ],
    deps = [
        "//cc/aws/proxy/src:proxy_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "preload_test",
    size = "small",
    srcs = ["preload_test.cc"],
    copts = [
        "-std=c++17",
    ],
    linkopts = ["-ldl"],
    deps = [
        "//cc/aws/proxy/src:preload",
        "@com_google_googletest//:gtest_main",
    ],
)

py_test(
    name = "socks5_test",
    size = "small",
    srcs = ["socks5_test.py"],
    args = ["$(location //cc/aws/proxy/src:proxy)"],
    data = [
        "//cc/aws/proxy/src:proxy",
    ],
)

sh_test(
    name = "preload_curl_test",
    size = "small",
    srcs = ["curl_client_test.sh"],
    args = [
        "$(location //cc/aws/proxy/src:proxy)",
        "$(location //cc/aws/proxy/src:proxy_preload)",
        "$(location //cc/aws/proxy/src:proxify)",
    ],
    data = [
        "//cc/aws/proxy/src:proxify",
        "//cc/aws/proxy/src:proxy",
        "//cc/aws/proxy/src:proxy_preload",
    ],
    tags = ["manual"],
)

sh_test(
    name = "preload_exported_symbols_test",
    size = "small",
    srcs = ["preload_exported_symbols.sh"],
    args = [
        "$(location //cc/aws/proxy/src:proxy_preload)",
    ],
    data = [
        "//cc/aws/proxy/src:proxy_preload",
    ],
)

cc_test(
    name = "buffer_test",
    size = "small",
    srcs = ["buffer_test.cc"],
    deps = [
        "//cc/aws/proxy/src:proxy_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "socket_test",
    size = "small",
    srcs = ["socket_test.cc"],
    deps = [
        "//cc/aws/proxy/src:proxy_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
