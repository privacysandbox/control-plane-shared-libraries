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

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def import_aws_nitro_enclaves_sdk():
    """
    Import AWS Nitro Enclaves SDK
    """
    maybe(
        http_archive,
        name = "s2n_tls",
        build_file = Label("//build_defs/cc/aws:s2n_tls.BUILD"),
        sha256 = "9d05578e881005db18ab979780b6af6c6099bae60bb6081b57e21c5e0615771d",
        strip_prefix = "s2n-tls-1.3.20",
        urls = [
            "https://github.com/aws/s2n-tls/archive/refs/tags/v1.3.20.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "aws_c_common",
        build_file = Label("//build_defs/cc/aws:aws_c_common.BUILD"),
        sha256 = "43a95662f958df6284d502a191cfdedea8ef76a96746a06cbdec94eeba97de61",
        strip_prefix = "aws-c-common-0.8.0",
        urls = [
            "https://github.com/awslabs/aws-c-common/archive/refs/tags/v0.8.0.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "aws_c_sdkutils",
        build_file = Label("//build_defs/cc/aws:aws_c_sdkutils.BUILD"),
        sha256 = "d654670c145212ed3ce0699a988b9f83ebf3e7c44ed74d4d0772dc95ad46b38e",
        strip_prefix = "aws-c-sdkutils-0.1.2",
        urls = [
            "https://github.com/awslabs/aws-c-sdkutils/archive/refs/tags/v0.1.2.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "aws_c_cal",
        build_file = Label("//build_defs/cc/aws:aws_c_cal.BUILD"),
        # Support BoringSSL, committed on May 2, 2023.
        sha256 = "99bbfbfbee523d3ea43e6db08fac83dbb81b215112c55b9731cda9fbe7c4550c",
        strip_prefix = "aws-c-cal-c20ec7e001a8967dbfdbeaf488ccf01afc75c217",
        urls = [
            "https://github.com/awslabs/aws-c-cal/archive/c20ec7e001a8967dbfdbeaf488ccf01afc75c217.zip",
        ],
    )

    maybe(
        http_archive,
        name = "aws_c_io",
        build_file = Label("//build_defs/cc/aws:aws_c_io.BUILD"),
        sha256 = "cd8fdcae6dcab2c91f699369220230f4973fd9c7dbda569c42326c8ead073be7",
        strip_prefix = "aws-c-io-0.11.0",
        urls = [
            "https://github.com/awslabs/aws-c-io/archive/refs/tags/v0.11.0.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "aws_c_compression",
        build_file = Label("//build_defs/cc/aws:aws_c_compression.BUILD"),
        sha256 = "8737863ced57d92f5a0bdde554bf0fe70eaa76aae118fec09a6c361dfc55d0d5",
        strip_prefix = "aws-c-compression-0.2.14",
        urls = [
            "https://github.com/awslabs/aws-c-compression/archive/refs/tags/v0.2.14.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "aws_c_http",
        build_file = Label("//build_defs/cc/aws:aws_c_http.BUILD"),
        sha256 = "4f6a36a580f7ed2206f97ef6d70cada23a05da43868af32b765fb429d576f47c",
        strip_prefix = "aws-c-http-0.6.19",
        urls = [
            "https://github.com/awslabs/aws-c-http/archive/refs/tags/v0.6.19.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "aws_c_auth",
        build_file = Label("//build_defs/cc/aws:aws_c_auth.BUILD"),
        sha256 = "52b1bf58155e726cc135257c1ea1e9af7c7f4e0f674d066165b11db20bb85477",
        strip_prefix = "aws-c-auth-0.6.15",
        urls = [
            "https://github.com/awslabs/aws-c-auth/archive/refs/tags/v0.6.15.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "json_c",
        build_file = Label("//build_defs/cc/aws:json_c.BUILD"),
        sha256 = "3ecaeedffd99a60b1262819f9e60d7d983844073abc74e495cb822b251904185",
        strip_prefix = "json-c-json-c-0.16-20220414",
        urls = [
            "https://github.com/json-c/json-c/archive/refs/tags/json-c-0.16-20220414.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "aws_nitro_enclaves_nsm_api",
        build_file = Label("//build_defs/cc/aws:aws_nitro_enclaves_nsm_api.BUILD"),
        sha256 = "a6b8c22c2d6dfe4166bc97ce3b78d7a7aee7be76b980aab64633417af8f00acb",
        strip_prefix = "aws-nitro-enclaves-nsm-api-0.3.0",
        urls = [
            "https://github.com/aws/aws-nitro-enclaves-nsm-api/archive/refs/tags/v0.3.0.tar.gz",
        ],
    )

    maybe(
        http_archive,
        name = "aws_nitro_enclaves_sdk",
        build_file = Label("//build_defs/cc/aws:aws_nitro_enclaves_sdk_source_code.BUILD"),
        patch_args = ["-p1"],
        patches = [Label("//build_defs/cc/aws:aws_nitro_enclaves_sdk.patch")],
        sha256 = "daeb96f0481bfc216d781bcd59605cf2535052ee3c4208efe4045b35e8ae8b65",
        strip_prefix = "aws-nitro-enclaves-sdk-c-0.3.2",
        urls = [
            "https://github.com/aws/aws-nitro-enclaves-sdk-c/archive/refs/tags/v0.3.2.tar.gz",
        ],
    )
