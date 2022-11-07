package(default_visibility = ["//visibility:public"])

java_library(
    name = "fake_data_module",
    srcs = glob(["*.java"]),
    deps = [
        "//java/com/google/scp/operator/shared/dao/metadatadb/common",
        "//java/com/google/scp/operator/shared/dao/metadatadb/testing:fake_metadata_db",
        "//java/com/google/scp/operator/shared/injection/modules",
        "//java/com/google/scp/operator/shared/testing",
        "//java/external:autoservice",
        "//java/external:autoservice_annotations",
        "//java/external:guava",
        "//java/external:guice",
    ],
)
