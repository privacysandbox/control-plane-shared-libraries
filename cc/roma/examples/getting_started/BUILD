# Copyright 2023 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load("@rules_cc//cc:defs.bzl", "cc_binary")

package(default_visibility = ["//visibility:public"])

# Binary that pulls roma
cc_binary(
    name = "hello_roma",
    srcs = ["hello_roma.cc"],
    copts = [
        "-std=c++17",
    ],
    tags = ["manual"],
    deps = [
        "@com_google_scp//cc/roma/interface:roma_interface_lib",
        "@com_google_scp//cc/roma/roma_service/src:roma_service_lib",
    ],
)
