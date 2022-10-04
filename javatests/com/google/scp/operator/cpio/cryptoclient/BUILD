package(default_visibility = ["//visibility:public"])

java_test(
    name = "HttpPrivateKeyFetchingServiceTest",
    srcs = ["HttpPrivateKeyFetchingServiceTest.java"],
    deps = [
        "//java/com/google/scp/operator/cpio/cryptoclient",
        "//java/com/google/scp/shared/api/exception",
        "//java/com/google/scp/shared/api/model",
        "//java/external:acai",
        "//java/external:apache_httpclient",
        "//java/external:apache_httpcore",
        "//java/external:google_truth",
        "//java/external:guice",
        "//java/external:mockito",
    ],
)

java_test(
    name = "HttpEncryptionKeyFetchingServiceTest",
    srcs = ["HttpEncryptionKeyFetchingServiceTest.java"],
    deps = [
        "//java/com/google/scp/operator/cpio/cryptoclient",
        "//java/com/google/scp/shared/api/exception",
        "//java/com/google/scp/shared/api/model",
        "//java/external:acai",
        "//java/external:apache_httpclient",
        "//java/external:apache_httpcore",
        "//java/external:google_truth",
        "//java/external:guice",
        "//java/external:mockito",
    ],
)

java_test(
    name = "DecryptionKeyServiceImplTest",
    srcs = ["DecryptionKeyServiceImplTest.java"],
    deps = [
        "//java/com/google/scp/operator/cpio/cryptoclient",
        "//java/external:acai",
        "//java/external:google_truth",
        "//java/external:guice",
        "//java/external:mockito",
        "//java/external:protobuf_java",
        "//java/external:tink",
        "//javatests/com/google/scp/shared/testutils/crypto:tink_utils",
    ],
)

java_test(
    name = "MultiPartyDecryptionKeyServiceImplTest",
    srcs = ["MultiPartyDecryptionKeyServiceImplTest.java"],
    deps = [
        "//coordinator/protos/keymanagement/keyhosting/api/v1:java_proto",
        "//coordinator/protos/keymanagement/shared/api/v1:java_proto",
        "//java/com/google/scp/coordinator/keymanagement/shared/util",
        "//java/com/google/scp/operator/cpio/cryptoclient",
        "//java/com/google/scp/shared/crypto/tink",
        "//java/external:acai",
        "//java/external:google_truth",
        "//java/external:guice",
        "//java/external:mockito",
        "//java/external:protobuf_java",
        "//java/external:tink",
        "//javatests/com/google/scp/shared/testutils/crypto:tink_utils",
    ],
)
