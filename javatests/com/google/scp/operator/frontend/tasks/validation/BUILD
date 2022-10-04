package(default_visibility = ["//visibility:public"])

java_test(
    name = "JobRequestIdLengthValidatorTest",
    srcs = ["JobRequestIdLengthValidatorTest.java"],
    deps = [
        "//java/com/google/scp/operator/frontend/tasks/validation",
        "//java/com/google/scp/operator/shared/dao/metadatadb/testing:job_generator",
        "//java/com/google/scp/operator/shared/model",
        "//java/external:google_truth8",
        "//java/external:guava",
        "//operator/protos/shared/backend:java_proto",
    ],
)

java_test(
    name = "JobRequestIdCharactersValidatorTest",
    srcs = ["JobRequestIdCharactersValidatorTest.java"],
    deps = [
        "//java/com/google/scp/operator/frontend/tasks/validation",
        "//java/com/google/scp/operator/shared/dao/metadatadb/testing:job_generator",
        "//java/com/google/scp/operator/shared/model",
        "//java/external:google_truth8",
        "//java/external:guava",
        "//java/external:test_parameter_injector",
        "//operator/protos/shared/backend:java_proto",
    ],
)
