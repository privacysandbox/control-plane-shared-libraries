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

load("@bazel_skylib//rules:common_settings.bzl", "string_flag")

package(default_visibility = ["//visibility:public"])

legacy_service_srcs = [
    "roma_service.cc",
    "roma_service.h",
    "roma.cc",
]

legacy_service_deps = [
    "//cc:cc_base_include_dir",
    "//cc/core/interface:interface_lib",
    "//cc/roma/dispatcher/src:roma_dispatcher_lib",
    "//cc/roma/interface:roma_interface_lib",
    "//cc/roma/worker/src:roma_worker_lib",
    "//cc/core/os/src/linux:system_resource_info_provider_linux_lib",
]

sandboxed_service_deps = [
    "//cc:cc_base_include_dir",
    "//cc/core/interface:interface_lib",
    "//cc/roma/sandbox/dispatcher/src:roma_dispatcher_lib",
    "//cc/roma/sandbox/roma_service/src:roma_service_lib",
    "//cc/core/os/src/linux:system_resource_info_provider_linux_lib",
]

service_copts = select({
    "roma_service_sandboxed": [
        "-D_SCP_ROMA_SANDBOXED_LIBRARY=1",
    ],
    "//conditions:default": [],
})

cc_library(
    name = "roma_service_lib",
    srcs = select({
        "roma_service_legacy": legacy_service_srcs,
        "roma_service_sandboxed": ["roma.cc"],
    }),
    copts = service_copts + [
        "-std=c++17",
    ],
    deps = select({
        "roma_service_legacy": legacy_service_deps,
        "roma_service_sandboxed": sandboxed_service_deps,
    }),
)

string_flag(
    name = "service",
    build_setting_default = "sandboxed",
    values = [
        "legacy",
        "sandboxed",
    ],
)

config_setting(
    name = "roma_service_legacy",
    flag_values = {"service": "legacy"},
)

config_setting(
    name = "roma_service_sandboxed",
    flag_values = {"service": "sandboxed"},
)
