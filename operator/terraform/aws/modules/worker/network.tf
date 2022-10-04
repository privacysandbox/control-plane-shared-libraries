/**
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

################################################################################
# VPC, Subnets, Security Groups
################################################################################

resource "aws_vpc" "worker_vpc" {
  count                = var.enable_customized_vpc ? 0 : 1
  cidr_block           = var.vpc_cidr
  enable_dns_hostnames = true
  tags = {
    Name        = "${var.service}-${var.environment}"
    service     = var.service
    environment = var.environment
  }
}

data "aws_availability_zones" "worker_az" {
  state = "available"
}

data "aws_availability_zone" "worker_az_info" {
  for_each = toset(data.aws_availability_zones.worker_az.names)

  name = each.key
}

resource "aws_subnet" "worker_subnet" {
  for_each = {
    for az, az_info in data.aws_availability_zone.worker_az_info : az => az_info.name_suffix
    if !var.enable_customized_vpc && contains(keys(var.az_map), az_info.name_suffix)
  }

  vpc_id = aws_vpc.worker_vpc[0].id
  # Use 4 bits to represent AZs (16 AZs max) and 12 bits for ips per subnet (4096 IPs per AZ)
  cidr_block              = cidrsubnet(aws_vpc.worker_vpc[0].cidr_block, 4, var.az_map[each.value])
  availability_zone       = each.key
  map_public_ip_on_launch = true
  tags = {
    Name        = "${var.service}-${var.environment}"
    service     = var.service
    environment = var.environment
  }
}

resource "aws_internet_gateway" "worker_igw" {
  count  = var.enable_customized_vpc ? 0 : 1
  vpc_id = aws_vpc.worker_vpc[0].id
  tags = {
    Name        = "${var.service}-${var.environment}"
    service     = var.service
    environment = var.environment
  }
}

resource "aws_route_table" "worker_route_table" {
  count  = var.enable_customized_vpc ? 0 : 1
  vpc_id = aws_vpc.worker_vpc[0].id

  route {
    cidr_block = "0.0.0.0/0"
    gateway_id = aws_internet_gateway.worker_igw[0].id
  }
  tags = {
    Name        = "${var.service}-${var.environment}"
    service     = var.service
    environment = var.environment
  }
}

resource "aws_route_table_association" "worker_route_table_assoc" {
  for_each = aws_subnet.worker_subnet

  route_table_id = aws_route_table.worker_route_table[0].id
  subnet_id      = each.value.id
}

resource "aws_security_group" "worker_sg" {
  count  = var.enable_customized_vpc ? 0 : 1
  name   = "worker-sg-${var.environment}"
  vpc_id = aws_vpc.worker_vpc[0].id
  tags = {
    Name        = "${var.service}-${var.environment}"
    service     = var.service
    environment = var.environment
  }
}

resource "aws_security_group_rule" "worker_sg_ingress" {
  count             = var.enable_customized_vpc ? 0 : 1
  type              = "ingress"
  description       = "ssh access"
  from_port         = 22
  to_port           = 22
  protocol          = "tcp"
  cidr_blocks       = ["0.0.0.0/0"]
  security_group_id = aws_security_group.worker_sg[0].id
}

resource "aws_security_group_rule" "worker_sg_egress_http" {
  count             = var.enable_customized_vpc ? 0 : 1
  type              = "egress"
  from_port         = 80
  to_port           = 80
  protocol          = "tcp"
  cidr_blocks       = ["0.0.0.0/0"]
  security_group_id = aws_security_group.worker_sg[0].id
  lifecycle {
    create_before_destroy = true
  }
}

resource "aws_security_group_rule" "worker_sg_egress_https" {
  count             = var.enable_customized_vpc ? 0 : 1
  type              = "egress"
  from_port         = 443
  to_port           = 443
  protocol          = "tcp"
  cidr_blocks       = ["0.0.0.0/0"]
  security_group_id = aws_security_group.worker_sg[0].id
  lifecycle {
    create_before_destroy = true
  }
}

resource "aws_security_group_rule" "worker_sg_egress_dns" {
  count             = var.enable_customized_vpc ? 0 : 1
  type              = "egress"
  from_port         = 53
  to_port           = 53
  protocol          = "tcp"
  cidr_blocks       = ["0.0.0.0/0"]
  security_group_id = aws_security_group.worker_sg[0].id
  lifecycle {
    create_before_destroy = true
  }
}
