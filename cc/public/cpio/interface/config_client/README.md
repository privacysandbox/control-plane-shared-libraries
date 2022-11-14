# ConfigClient

Responsible for fetching pre-stored application data or cloud metadata.  
In AWS, the application data should be pre-stored in ParameterStore and surfaced
through GetParameter function. Environment name should be pre-tagged in EC2 instance and
surfaced through GetEnvironment function.

# Build

## Building the client

    bazel build cc/public/cpio/interface/config_client/...

## Running tests

    bazel test cc/public/cpio/adapters/config_client/... && bazel test cc/cpio/client_providers/config_client_provider/...

# [Example](/cc/public/cpio/examples/config_client_test.cc)
