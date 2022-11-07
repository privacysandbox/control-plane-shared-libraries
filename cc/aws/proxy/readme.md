# Build
## Building the proxy
    bazel build //cc/aws/proxy/...

## Building and running tests

    bazel test //cc/aws/proxy/...
# Other useful tips
## Running your own application
If you'd like to try out this proxy with your own application, follow these steps:
1. `bazel build //cc/aws/proxy:reproducible_proxy_outputs`
1. Under directory `bazel-bin/cc/aws/proxy`, find the built binaries  `proxy`, `proxify`, `libproxy_preload.so`, `socket_vendor`, upload to
your EC2 instance.
1. Run proxy in background on the EC2 instance.
1. Add `proxify`, `libproxy_preload.so` and `socket_vendor` to the docker image under the same directory.
1. Put CMD/ENTRYPOINT as `["/path/to/proxify", "your_app", "one_arg", "more_args"]`
1. Build and run the enclave image.
