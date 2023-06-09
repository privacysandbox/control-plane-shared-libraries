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
    name = "kms_client_provider_lib",
    srcs =
        select({
            "//cc/public/cpio/interface:aws_cpio_lib_inside_tee": [
                "//cc/cpio/client_providers/kms_client_provider/src/aws:tee_aws_kms_client_provider_srcs",
            ],
            "//cc/public/cpio/interface:aws_cpio_lib_outside_tee": [
                "//cc/cpio/client_providers/kms_client_provider/src/aws:nontee_aws_kms_client_provider_srcs",
            ],
            "//cc/public/cpio/interface:gcp_cpio_lib_inside_tee": [
                "//cc/cpio/client_providers/kms_client_provider/src/gcp:tee_gcp_kms_client_provider_srcs",
            ],
            "//cc/public/cpio/interface:gcp_cpio_lib_outside_tee": [
                "//cc/cpio/client_providers/kms_client_provider/src/gcp:nontee_gcp_kms_client_provider_srcs",
            ],
        }) + ["//cc/cpio/client_providers/interface:kms_client_provider_interface.h"],
    copts = [
        "-std=c++17",
    ],
    deps =
        select({
            "//cc/public/cpio/interface:aws_cpio_lib_inside_tee": [
            ],
            "//cc/public/cpio/interface:aws_cpio_lib_outside_tee": [
                "@aws_sdk_cpp//:kms",
            ],
            "//cc/public/cpio/interface:gcp_cpio_lib_inside_tee": [
                "//cc/cpio/client_providers/kms_client_provider/interface/gcp:gcp_kms_client_provider_interface",
                "@com_github_googleapis_google_cloud_cpp//:kms",
                "@com_google_absl//absl/strings",
                "@nlohmann_json//:lib",
            ],
            "//cc/public/cpio/interface:gcp_cpio_lib_outside_tee": [
                "//cc/cpio/client_providers/kms_client_provider/interface/gcp:gcp_kms_client_provider_interface",
                "@com_github_googleapis_google_cloud_cpp//:kms",
                "@com_google_absl//absl/strings",
                "@nlohmann_json//:lib",
            ],
        }) + [
            "//cc:cc_base_include_dir",
            "//cc/core/common/uuid/src:uuid_lib",
            "//cc/core/interface:async_context_lib",
            "//cc/core/interface:interface_lib",
            "//cc/core/utils/src:core_utils",
            "//cc/cpio/client_providers/global_cpio/src:global_cpio_lib",
            "//cc/public/cpio/interface:cpio_errors",
            "//cc/public/cpio/interface:type_def",
            "@tink_cc",
        ],
)
