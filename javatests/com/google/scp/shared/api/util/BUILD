package(default_visibility = ["//visibility:public"])

java_test(
    name = "ErrorUtilTest",
    srcs = [
        "ErrorUtilTest.java",
    ],
    deps = [
        "//java/com/google/scp/shared/api/exception",
        "//java/com/google/scp/shared/api/model",
        "//java/com/google/scp/shared/api/util",
        "//java/com/google/scp/shared/mapper",
        "//java/external:google_truth",
        "//java/external:jackson_core",
        "//java/external:jackson_databind",
        "//java/external:protobuf_java",
        "//java/external:protobuf_java_util",
        "//shared/protos/api/v1:java_proto",
    ],
)

java_test(
    name = "RequestUtilTest",
    srcs = [
        "RequestUtilTest.java",
    ],
    deps = [
        "//java/com/google/scp/shared/api/exception",
        "//java/com/google/scp/shared/api/model",
        "//java/com/google/scp/shared/api/util",
        "//java/external:google_truth",
    ],
)
