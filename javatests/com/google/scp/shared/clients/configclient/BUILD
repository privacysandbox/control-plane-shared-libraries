package(default_visibility = ["//visibility:public"])

java_test(
    name = "InstanceProfileCredentialsWithRetryProviderTest",
    srcs = [
        "InstanceProfileCredentialsWithRetryProviderTest.java",
    ],
    deps = [
        "//java/com/google/scp/shared/clients/configclient:aws",
        "//java/external:aws_auth",
        "//java/external:aws_core",
        "//java/external:aws_utils",
        "//java/external:google_truth",
        "//java/external:mockserver_client",
        "//java/external:mockserver_core",
        "//java/external:mockserver_junit_rule",
        "//java/external:resilience4j_core",
        "//java/external:resilience4j_retry",
    ],
)

java_test(
    name = "ParameterClientUtilsTest",
    srcs = [
        "ParameterClientUtilsTest.java",
    ],
    deps = [
        "//java/com/google/scp/shared/clients/configclient",
        "//java/com/google/scp/shared/clients/configclient:model",
        "//java/external:google_truth",
    ],
)
