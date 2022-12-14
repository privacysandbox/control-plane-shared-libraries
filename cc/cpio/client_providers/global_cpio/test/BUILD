# Copyright 2022 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

package(default_visibility = ["//cc:scp_internal_pkg"])

cc_library(
    name = "local_lib_cpio_provider",
    srcs = select({
        "//cc/public/cpio/interface:aws_cpio_lib": ["//cc/cpio/client_providers/global_cpio/src/cpio_provider:lib_cpio_provider_srcs"],
        "//conditions:default": ["//cc/cpio/client_providers/global_cpio/src/cpio_provider:lib_cpio_provider_srcs"],
    }) + [
        "//cc/cpio/client_providers/interface:cpio_provider_interface.h",
        "local_lib_cpio_provider.h",
        "local_lib_cpio_provider.cc",
    ],
    copts = [
        "-DLOCAL_CPIO=1",
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/async_executor/src:core_async_executor_lib",
        "//cc/core/http2_client/src:http2_client_lib",
        "//cc/core/logger/src:logger_lib",
        "//cc/core/logger/src/log_providers:log_providers_lib",
        "//cc/core/logger/src/log_providers/syslog:syslog_lib",
        "//cc/core/message_router/src:message_router_lib",
        "//cc/cpio/client_providers/instance_client_provider:instance_client_provider_lib",
        "//cc/cpio/client_providers/instance_client_provider/test:local_instance_client_provider_lib",
        "//cc/public/cpio/interface:type_def",
        "//cc/public/cpio/local:local_cpio_options",
    ],
)

cc_library(
    name = "local_global_cpio_lib",
    srcs =
        [
            "//cc/cpio/client_providers/global_cpio/src:global_cpio_srcs",
        ],
    copts = [
        "-DLOCAL_CPIO=1",
        "-std=c++17",
    ],
    deps = [
        ":local_lib_cpio_provider",
        "//cc:cc_base_include_dir",
        "//cc/core/logger/src:logger_lib",
        "//cc/core/logger/src/log_providers:log_providers_lib",
        "//cc/core/logger/src/log_providers/syslog:syslog_lib",
    ],
)

cc_library(
    name = "test_lib_cpio_provider",
    srcs = select({
        "//cc/public/cpio/interface:aws_cpio_lib": ["//cc/cpio/client_providers/global_cpio/src/cpio_provider:lib_cpio_provider_srcs"],
        "//conditions:default": ["//cc/cpio/client_providers/global_cpio/src/cpio_provider:lib_cpio_provider_srcs"],
    }) + [
        "//cc/cpio/client_providers/interface:cpio_provider_interface.h",
        "test_lib_cpio_provider.h",
        "test_lib_cpio_provider.cc",
    ],
    copts = [
        "-DTEST_CPIO=1",
        "-std=c++17",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/async_executor/src:core_async_executor_lib",
        "//cc/core/http2_client/src:http2_client_lib",
        "//cc/core/logger/src:logger_lib",
        "//cc/core/logger/src/log_providers:log_providers_lib",
        "//cc/core/logger/src/log_providers/syslog:syslog_lib",
        "//cc/core/message_router/src:message_router_lib",
        "//cc/cpio/client_providers/instance_client_provider:instance_client_provider_lib",
        "//cc/cpio/client_providers/instance_client_provider/test:test_instance_client_provider_lib",
        "//cc/public/cpio/interface:type_def",
    ],
)

cc_library(
    name = "test_global_cpio_lib",
    srcs =
        [
            "//cc/cpio/client_providers/global_cpio/src:global_cpio_srcs",
        ],
    copts = [
        "-DTEST_CPIO=1",
        "-std=c++17",
    ],
    deps = [
        ":test_lib_cpio_provider",
        "//cc:cc_base_include_dir",
        "//cc/core/logger/src:logger_lib",
        "//cc/core/logger/src/log_providers:log_providers_lib",
        "//cc/core/logger/src/log_providers/syslog:syslog_lib",
    ],
)

cc_test(
    name = "lib_cpio_provider_test",
    size = "small",
    srcs = ["lib_cpio_provider_test.cc"],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/cpio/client_providers/global_cpio/mock:global_cpio_mock",
        "//cc/public/cpio/interface:cpio",
        "@aws_sdk_cpp//:core",
        "@com_google_googletest//:gtest_main",
    ],
)
