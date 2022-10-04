package(default_visibility = ["//visibility:public"])

load("@rules_java//java:defs.bzl", "java_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

proto_library(
    name = "proto",
    srcs = glob(["*.proto"]),
)

java_proto_library(
    name = "java_proto",
    deps = [":proto"],
)
