# Copyright 2023 Google LLC
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

load("@io_bazel_rules_closure//closure:defs.bzl", "closure_js_binary", "closure_js_library")

package(default_visibility = ["//visibility:public"])

closure_js_library(
    name = "proto_as_hook_argument_js_lib",
    srcs = [
        "externs.js",  # File contains functions that have no definitons (Roma hooks)
        "proto_as_hook_argument.js",  # Contains the Roma handler
    ],
    convention = "NONE",
    deps = [
        "//cc/roma/examples/proto_in_hooks/proto:collection_of_doubles_js_proto",
    ],
)

closure_js_binary(
    name = "proto_as_hook_argument_js",
    deps = [
        "proto_as_hook_argument_js_lib",
    ],
)
