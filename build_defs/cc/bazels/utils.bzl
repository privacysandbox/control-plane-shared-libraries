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

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def utils():
    maybe(
        git_repository,
        name = "com_github_google_rpmpack",
        # Lastest commit in main branch as of 2021-11-29
        commit = "d0ed9b1b61b95992d3c4e83df3e997f3538a7b6c",
        remote = "https://github.com/google/rpmpack.git",
        shallow_since = "1637822718 +0200",
    )

    maybe(
        new_git_repository,
        name = "moodycamel_concurrent_queue",
        build_file = Label("//build_defs/cc/build_targets:moodycamel.BUILD"),
        # Commited Mar 20, 2022
        commit = "22c78daf65d2c8cce9399a29171676054aa98807",
        remote = "https://github.com/cameron314/concurrentqueue.git",
        shallow_since = "1647803790 -0400",
    )

    maybe(
        new_git_repository,
        name = "nlohmann_json",
        build_file = Label("//build_defs/cc/build_targets:nlohmann.BUILD"),
        # Commits on Apr 6, 2022
        commit = "15fa6a342af7b51cb51a22599026e01f1d81957b",
        remote = "https://github.com/nlohmann/json.git",
    )

    maybe(
        git_repository,
        name = "oneTBB",
        # Commits on Apr 18, 2022
        commit = "9d2a3477ce276d437bf34b1582781e5b11f9b37a",
        remote = "https://github.com/oneapi-src/oneTBB.git",
        shallow_since = "1648820995 +0300",
    )

    maybe(
        http_archive,
        name = "curl",
        build_file = Label("//build_defs/cc/build_targets:curl.BUILD"),
        sha256 = "ff3e80c1ca6a068428726cd7dd19037a47cc538ce58ef61c59587191039b2ca6",
        strip_prefix = "curl-7.49.1",
        urls = [
            "https://mirror.bazel.build/curl.haxx.se/download/curl-7.49.1.tar.gz",
        ],
    )

    # Bazel Skylib. Required by absl.
    maybe(
        http_archive,
        name = "bazel_skylib",
        sha256 = "f7be3474d42aae265405a592bb7da8e171919d74c16f082a5457840f06054728",
        urls = ["https://github.com/bazelbuild/bazel-skylib/releases/download/1.2.1/bazel-skylib-1.2.1.tar.gz"],
    )

    maybe(
        http_archive,
        name = "com_google_absl",
        sha256 = "81311c17599b3712069ded20cca09a62ab0bf2a89dfa16993786c8782b7ed145",
        strip_prefix = "abseil-cpp-20230125.1",
        # Committed on Jan 25, 2023.
        urls = [
            "https://github.com/abseil/abseil-cpp/archive/20230125.1.tar.gz",
        ],
    )

    maybe(
        git_repository,
        name = "boringssl",
        # Committed on Oct 3, 2022
        # https://github.com/google/boringssl/commit/c2837229f381f5fcd8894f0cca792a94b557ac52
        commit = "c2837229f381f5fcd8894f0cca792a94b557ac52",
        remote = "https://github.com/google/boringssl.git",
    )
