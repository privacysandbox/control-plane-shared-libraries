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
    name = "queue_client_provider_lib",
    srcs = select({
        "//cc/public/cpio/interface:aws_cpio_lib": ["//cc/cpio/client_providers/queue_client_provider/src/aws:aws_queue_client_provider_srcs"],
        "//cc/public/cpio/interface:gcp_cpio_lib": ["//cc/cpio/client_providers/queue_client_provider/src/gcp:gcp_queue_client_provider_srcs"],
        "//conditions:default": ["//cc/cpio/client_providers/queue_client_provider/src/aws:aws_queue_client_provider_srcs"],
    }) + [
        "//cc/cpio/client_providers/interface:queue_client_provider_interface.h",
    ],
    copts = [
        "-std=c++17",
    ],
    deps = select({
        "//cc/public/cpio/interface:aws_cpio_lib": [
            "//cc/cpio/common/src/aws:aws_utils_lib",
            "@aws_sdk_cpp//:core",
            "@aws_sdk_cpp//:sqs",
        ],
        "//cc/public/cpio/interface:gcp_cpio_lib": [
            "//cc/cpio/common/src/gcp:gcp_utils_lib",
            "@com_github_googleapis_google_cloud_cpp//:pubsub",
        ],
        "//conditions:default": [
            "//cc/cpio/common/src/aws:aws_utils_lib",
            "@aws_sdk_cpp//:core",
            "@aws_sdk_cpp//:sqs",
        ],
    }) + [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:async_context_lib",
        "//cc/core/interface:interface_lib",
        "//cc/cpio/client_providers/global_cpio/src:global_cpio_lib",
        "//cc/cpio/client_providers/interface:type_def",
        "//cc/public/core/interface:execution_result",
        "//cc/public/cpio/interface:cpio_errors",
        "//cc/public/cpio/proto/queue_service/v1:queue_service_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)
