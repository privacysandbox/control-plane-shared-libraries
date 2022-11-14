# MetricClient

Responsible for recording custom metrics to cloud.  
Batching recording is supported. Set the time duration carefully based on the system's QPS and how much quota you have in cloud.

# Build

## Building the client

    bazel build cc/public/cpio/interface/metric_client/...

## Running tests

    bazel test cc/public/cpio/adapters/metric_client/... && bazel test cc/cpio/client_providers/metric_client_provider/...

# [Example](/cc/public/cpio/examples/metric_client_test.cc)
