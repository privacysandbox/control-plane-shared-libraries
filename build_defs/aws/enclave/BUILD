load(":allocator.bzl", "allocator_yaml")

package(default_visibility = ["//visibility:public"])

exports_files([
    "allocator.template.yaml",
    "generic_enclave_ami.pkr.hcl",
])

# Small allocator.yaml file suitable for running on a c5.xlarge (4 host cpus,
# 8GB host RAM).
allocator_yaml(
    name = "small.allocator.yaml",
    enclave_cpus = 2,
    enclave_memory_mib = 7168,
)
