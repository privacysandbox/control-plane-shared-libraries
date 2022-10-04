package(default_visibility = ["//visibility:public"])

# This library is to be used as a dummy dependency of all cc targets under this
# directory, so that we don't need to type the full include path containing
# "cc/".
cc_library(
    name = "cc_base_include_dir",
    includes = ["."],
    visibility = [":__subpackages__"],
)
