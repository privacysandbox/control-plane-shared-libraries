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
    name = "config_client_provider_lib",
    srcs = select({
        "//cc/public/cpio/interface:aws_cpio_lib": ["//cc/cpio/client_providers/config_client_provider/aws/src:aws_config_client_provider_srcs"],
        "//conditions:default": ["//cc/cpio/client_providers/config_client_provider/aws/src:aws_config_client_provider_srcs"],
    }) + [
        ":config_client_provider_srcs",
        "//cc/cpio/client_providers/interface:config_client_provider_interface.h",
    ],
    copts = [
        "-std=c++17",
    ],
    deps = select({
        "//cc/public/cpio/interface:aws_cpio_lib": [
            "@aws_sdk_cpp//:core",
            "//cc/cpio/common/aws/src:aws_utils_lib",
            "@aws_sdk_cpp//:ssm",
        ],
        "//conditions:default": [
            "@aws_sdk_cpp//:core",
            "//cc/cpio/common/aws/src:aws_utils_lib",
            "@aws_sdk_cpp//:ssm",
        ],
    }) + [
        "//cc:cc_base_include_dir",
        "//cc/core/message_router/src:message_router_lib",
        "//cc/cpio/client_providers/global_cpio/src:global_cpio_lib",
        "//cc/cpio/client_providers/instance_client_provider:instance_client_provider_lib",
        "//cc/cpio/client_providers/interface:type_def",
        "//cc/cpio/proto:cc_proto",
        "//cc/public/cpio/interface:cpio_errors",
        "//cc/public/cpio/interface/config_client:type_def",
    ],
)

exports_files([
    "config_client_provider.h",
    "config_client_provider.cc",
    "error_codes.h",
])

filegroup(
    name = "config_client_provider_srcs",
    srcs = [
        ":config_client_provider.cc",
        ":config_client_provider.h",
        ":error_codes.h",
    ],
)
