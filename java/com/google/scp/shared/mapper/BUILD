package(default_visibility = ["//visibility:public"])

java_library(
    name = "mapper",
    srcs = [
        "GuavaObjectMapper.java",
        "TimeObjectMapper.java",
    ],
    deps = [
        "//java/external:jackson_core",
        "//java/external:jackson_databind",
        "//java/external:jackson_datatype_guava",
        "//java/external:jackson_datatype_jdk8",
        "//java/external:jackson_datatype_jsr310",
    ],
)
