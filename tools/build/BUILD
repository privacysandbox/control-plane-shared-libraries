load("@rules_python//python:defs.bzl", "py_runtime_pair")

py_runtime(
    name = "py_wrapper_runtime",
    interpreter = "py_wrapper.sh",
    python_version = "PY3",
)

py_runtime_pair(
    name = "py_wrapper_runtime_pair",
    py2_runtime = None,
    py3_runtime = ":py_wrapper_runtime",
)

toolchain(
    name = "py_wrapper_toolchain",
    toolchain = ":py_wrapper_runtime_pair",
    toolchain_type = "@rules_python//python:toolchain_type",
)
