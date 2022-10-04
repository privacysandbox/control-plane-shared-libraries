load("@rules_pkg//:mappings.bzl", "pkg_filegroup", "pkg_files", "pkg_mklink")

package(default_visibility = ["//visibility:public"])

# Files to go both in the "shared" folder and the "demo" folder.
# Not symlinked for simplicity in interacting with various operator
# environments.
SHARED_FILES = [
    # LINT.IfChange
    # ami_params.auto.tfvars
    "//operator/terraform/aws:ami_params_auto_tfvars",
    "operator_service.tf",
    "operator_service_variables.tf",
    # LINT.ThenChange(:README.export.md)
]

pkg_files(
    name = "shared_files",
    srcs = SHARED_FILES,
    prefix = "shared",
)

pkg_files(
    name = "demo_files",
    srcs = [
        ".terraform.lock.hcl",
        # Note: README.md is the repo-internal readme and README.export.md is
        # the one intended to be included in the tar.
        "README.export.md",
        "example.auto.tfvars",
        "main.tf",
    ],
    prefix = "demo",
    renames = {
        "README.export.md": "README.md",
    },
)

# pkg_tar requires links be declared with mklink, otherwise the linked file is
# copied.
pkg_mklink(
    name = "operator_service_link",
    src = "../shared/operator_service.tf",
    dest = "demo/operator_service.tf",
)

pkg_mklink(
    name = "operator_service_vars_link",
    src = "../shared/operator_service_variables.tf",
    dest = "demo/operator_service_variables.tf",
)

pkg_mklink(
    name = "ami_params_link",
    src = "../shared/ami_params.auto.tfvars",
    dest = "demo/ami_params.auto.tfvars",
)

pkg_filegroup(
    name = "demo_environment_pkg",
    srcs = [
        ":ami_params_link",
        ":demo_files",
        ":operator_service_link",
        ":operator_service_vars_link",
        ":shared_files",
    ],
    prefix = "environments",
)
