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
    name = "local_cpio_options",
    hdrs = [
        "local_cpio_options.h",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/public/cpio/interface:type_def",
    ],
)

cc_library(
    name = "local_lib_cpio",
    srcs = [
        "local_lib_cpio.cc",
        "//cc/public/cpio/core/src:cpio.cc",
    ],
    hdrs = [
        "local_lib_cpio.h",
        "//cc/public/cpio/interface:cpio.h",
    ],
    copts = [
        "-DLOCAL_CPIO=1",
    ],
    deps = [
        "//cc:cc_base_include_dir",
        "//cc/core/interface:interface_lib",
        "//cc/core/logger/src:logger_lib",
        "//cc/core/logger/src/log_providers:log_providers_lib",
        "//cc/core/logger/src/log_providers/syslog:syslog_lib",
        "//cc/cpio/client_providers/global_cpio/test:local_global_cpio_lib",
        "//cc/cpio/client_providers/global_cpio/test:local_lib_cpio_provider",
    ],
)
