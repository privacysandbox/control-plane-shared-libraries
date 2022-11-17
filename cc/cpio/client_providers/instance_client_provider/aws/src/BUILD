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

exports_files([
    "aws_instance_client_provider.h",
    "aws_instance_client_provider.cc",
    "ec2_error_converter.h",
    "ec2_error_converter.cc",
    "error_codes.h",
])

filegroup(
    name = "aws_instance_client_provider_srcs",
    srcs = [
        ":aws_instance_client_provider.cc",
        ":aws_instance_client_provider.h",
        ":ec2_error_converter.cc",
        ":ec2_error_converter.h",
        ":error_codes.h",
    ],
)
