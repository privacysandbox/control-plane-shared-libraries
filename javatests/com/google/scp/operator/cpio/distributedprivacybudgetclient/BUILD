package(default_visibility = ["//visibility:public"])

java_test(
    name = "DistributedPrivacyBudgetClientTest",
    srcs = ["DistributedPrivacyBudgetClientTest.java"],
    deps = [
        "//java/com/google/scp/coordinator/privacy/budgeting/model",
        "//java/com/google/scp/operator/cpio/distributedprivacybudgetclient:distributed_privacy_budget_client",
        "//java/external:google_truth",
        "//java/external:mockito",
    ],
)

java_test(
    name = "PrivacyBudgetClientTest",
    srcs = ["PrivacyBudgetClientTest.java"],
    deps = [
        "//java/com/google/scp/coordinator/privacy/budgeting/model",
        "//java/com/google/scp/operator/cpio/distributedprivacybudgetclient:distributed_privacy_budget_client",
        "//java/com/google/scp/shared/aws/util",
        "//java/external:autovalue",
        "//java/external:autovalue_annotations",
        "//java/external:google_truth",
        "//java/external:mockito",
    ],
)

java_test(
    name = "TransactionEngineTest",
    srcs = ["TransactionEngineTest.java"],
    deps = [
        "//java/com/google/scp/coordinator/privacy/budgeting/model",
        "//java/com/google/scp/operator/cpio/distributedprivacybudgetclient:distributed_privacy_budget_client",
        "//java/external:google_truth",
        "//java/external:guava",
        "//java/external:mockito",
    ],
)

java_test(
    name = "TransactionPhaseManagerTest",
    srcs = ["TransactionPhaseManagerTest.java"],
    deps = [
        "//java/com/google/scp/operator/cpio/distributedprivacybudgetclient:distributed_privacy_budget_client",
        "//java/external:google_truth",
    ],
)
