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

load("//build_defs/cc/bazels:protobuf.bzl", "protobuf")
load("//build_defs/cc/bazels:boost.bzl", "boost")
load("//build_defs/cc/bazels:nghttp2.bzl", "nghttp2")
load("//build_defs/cc/bazels:bazel_rules_cpp.bzl", "bazel_rules")
load("//build_defs/cc/bazels:bazel_container_rules.bzl", "bazel_container_rules")
load("//build_defs/cc/bazels:gtest.bzl", "google_test")
load("//build_defs/cc/bazels:enclaves_kmstools.bzl", "enclaves_kmstools_libraries")
load("//build_defs/cc/bazels:utils.bzl", "utils")
load("//build_defs/cc/bazels:golang.bzl", "go_deps")
load("//build_defs/cc/bazels:grpc.bzl", "grpc")
load("//build_defs/cc/aws:aws_sdk_cpp_deps.bzl", "import_aws_sdk_cpp")
load("//build_defs/cc/bazels:google_cloud_cpp.bzl", "import_google_cloud_cpp")

def sdk_dependencies(protobuf_version, protobuf_repo_hash):
    protobuf(protobuf_version, protobuf_repo_hash)
    boost()
    nghttp2()
    bazel_rules()
    bazel_container_rules()
    google_test()
    enclaves_kmstools_libraries()
    utils()
    go_deps()
    grpc()
    import_aws_sdk_cpp()
    import_google_cloud_cpp()
