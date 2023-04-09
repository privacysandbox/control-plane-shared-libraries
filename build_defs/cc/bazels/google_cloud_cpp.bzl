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

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

# Builds Google cloud cpp
# --> must be before grpc_deps() and grpc_extra_deps()
# --> and after http_archive com_github_grpc_grpc

def import_google_cloud_cpp():
    maybe(
        http_archive,
        name = "com_github_googleapis_google_cloud_cpp",
        sha256 = "21fb441b5a670a18bb16b6826be8e0530888d0b94320847c538d46f5a54dddbc",
        strip_prefix = "google-cloud-cpp-2.8.0",
        url = "https://github.com/googleapis/google-cloud-cpp/archive/v2.8.0.tar.gz",
    )
