package(default_visibility = ["//visibility:public"])

java_test(
    name = "HttpClientRetryTest",
    srcs = ["HttpClientRetryTest.java"],
    deps = [
        "//java/com/google/scp/shared/clients/configclient:aws",
        "//java/external:apache_httpclient",
        "//java/external:apache_httpcore",
        "//java/external:google_truth",
        "//java/external:guice",
    ],
)

java_test(
    name = "AwsParameterClientTest",
    srcs = [
        "AwsParameterClientTest.java",
    ],
    deps = [
        "//java/com/google/scp/shared/clients/configclient:aws",
        "//java/com/google/scp/shared/clients/configclient:model",
        "//java/external:aws_ec2",
        "//java/external:aws_ssm",
        "//java/external:google_truth",
        "//java/external:google_truth8",
        "//java/external:guice",
        "//java/external:mockito",
    ],
)
