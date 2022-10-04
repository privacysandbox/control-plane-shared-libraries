#!/bin/bash
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

#
# Builds enclave image inside the /home/ec2-user directory as part of automatic
# AMI generation.
sudo amazon-linux-extras install docker
sudo systemctl enable docker
sudo systemctl start docker

sudo docker load --input /tmp/{container_file}

sudo amazon-linux-extras install aws-nitro-enclaves-cli

sudo yum install aws-nitro-enclaves-cli-devel -y
sudo systemctl enable nitro-enclaves-allocator.service

sudo rpm -i /tmp/rpms/*.rpm
sudo systemctl enable vsockproxy
sudo systemctl enable aggregate-worker

# This script runs in the boot phase in which environment variables are not set.
# This variable is needed to run nitro-cli build-enclave.
export NITRO_CLI_ARTIFACTS="/tmp/enclave-images/"

sudo nitro-cli build-enclave --docker-uri bazel/{docker_repo}:{docker_tag} --output-file /opt/google/worker/enclave.eif

# Move config files which require elevated permissions.
# https://github.com/hashicorp/packer/issues/1551#issuecomment-59131451
sudo mv /tmp/allocator.yaml /etc/nitro_enclaves/allocator.yaml
sudo mkdir /licenses
sudo tar xf /tmp/licenses.tar -C /licenses
