package(default_visibility = ["//visibility:public"])

java_test(
    name = "S3BlobStorageClientTest",
    srcs = ["S3BlobStorageClientTest.java"],
    runtime_deps = [
        # LocalStackContainers uses v1 API in the background.
        # However, we don't interface with the v1 API.
        # TODO: Find a way to not have to copy this to all other AWS tests.
        "//java/external:aws_s3_java_sdk1",
    ],
    deps = [
        "//java/com/google/scp/operator/cpio/blobstorageclient:aws",
        "//java/com/google/scp/operator/cpio/blobstorageclient:model",
        "//java/external:acai",
        "//java/external:aws_auth",
        "//java/external:aws_core",
        "//java/external:aws_regions",
        "//java/external:aws_s3",
        "//java/external:gcp_storage",
        "//java/external:google_truth",
        "//java/external:google_truth8",
        "//java/external:guice",
        "//java/external:localstack_containers",
        "//java/external:testcontainers",
        "//javatests/com/google/scp/shared/testutils/aws",
    ],
)
