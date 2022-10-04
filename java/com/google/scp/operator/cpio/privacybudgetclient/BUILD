package(default_visibility = ["//visibility:public"])

java_library(
    name = "privacybudgetclient",
    srcs = glob([
        "*.java",
        "local/*.java",
        "testing/*.java",
    ]),
    deps = [
        "//java/com/google/scp/coordinator/privacy/budgeting/model",
        "//java/com/google/scp/shared/api/exception",
        "//java/com/google/scp/shared/api/model",
        "//java/com/google/scp/shared/api/util",
        "//java/com/google/scp/shared/mapper",
        "//java/external:apache_httpclient",
        "//java/external:apache_httpcore",
        "//java/external:guava",
        "//java/external:guice",
        "//java/external:jackson_core",
        "//java/external:jackson_databind",
        "//java/external:jackson_datatype_jsr310",
        "//java/external:javax_inject",
        "//java/external:slf4j",
        "//shared/protos/api/v1:java_proto",
    ],
)
