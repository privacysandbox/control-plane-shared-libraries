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

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "test_aws_metric_client_options",
    hdrs = [
        "test_aws_metric_client_options.h",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/public/cpio/interface/metric_client:type_def",
    ],
)

cc_library(
    name = "test_aws_config_client_options",
    hdrs = [
        "test_aws_config_client_options.h",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/public/cpio/interface/config_client:type_def",
    ],
)

cc_library(
    name = "test_lib_cpio",
    srcs = ["//cc/public/cpio/core/src:cpio.cc"],
    hdrs = [
        "//cc/public/cpio/interface:cpio.h",
    ],
    copts = [
        "-DTEST_CPIO=1",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/core/logger/src:logger_lib",
        "//cc/core/logger/src/log_providers:log_providers_lib",
        "//cc/core/logger/src/log_providers/syslog:syslog_lib",
        "//cc/cpio/client_providers/global_cpio/test:test_global_cpio_lib",
        "//cc/cpio/client_providers/global_cpio/test:test_lib_cpio_provider",
    ],
)

cc_library(
    name = "test_aws_metric_client",
    srcs = [
        "//cc/public/cpio/adapters/metric_client/src:metric_client_srcs",
        "//cc/public/cpio/adapters/metric_client/test:test_aws_metric_client_srcs",
    ],
    hdrs =
        [
            "//cc/public/cpio/interface/metric_client:metric_client_interface.h",
        ],
    deps = [
        ":test_aws_metric_client_options",
        "//cc:cc_base_include_dir",
        "//cc/core/interface:async_context_lib",
        "//cc/core/interface:interface_lib",
        "//cc/core/utils/src:core_utils",
        "//cc/cpio/client_providers/metric_client_provider/aws/test:test_aws_metric_client_provider_lib",
        "//cc/cpio/proto:cc_proto",
        "//cc/public/cpio/interface:type_def",
        "//cc/public/cpio/interface/metric_client:type_def",
    ],
)

cc_library(
    name = "test_aws_config_client",
    srcs = [
        "//cc/public/cpio/adapters/config_client/src:config_client_srcs",
        "//cc/public/cpio/adapters/config_client/test:test_aws_config_client_srcs",
    ],
    hdrs =
        [
            "//cc/public/cpio/interface/config_client:config_client_interface.h",
        ],
    deps = [
        ":test_aws_config_client_options",
        "//cc:cc_base_include_dir",
        "//cc/core/interface:async_context_lib",
        "//cc/core/interface:interface_lib",
        "//cc/core/utils/src:core_utils",
        "//cc/cpio/client_providers/config_client_provider/aws/test:test_aws_config_client_provider_lib",
        "//cc/cpio/proto:cc_proto",
        "//cc/public/cpio/interface:type_def",
        "//cc/public/cpio/interface/config_client:type_def",
    ],
)
