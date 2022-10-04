# Build
## Building the proxy
    bazel build //cc/aws/proxy/...

## Building and running tests

    bazel test //cc/aws/proxy/...
# Other useful tips
## Running your own application
If you'd like to try out this proxy with your own application, follow these steps:
1. `export CC=/usr/bin/gcc-9; export CXX=/usr/bin/g++-9`
1. `bazel build //cc/aws/proxy/src/...`
1. Find the built binaries: `proxy`, `proxify`, `libproxy_preload.so`, upload to
your EC2 instance.
1. Run proxy in background on the EC2 instance.
1. Add `proxify` and `libproxy_preload.so` to the docker image under the same directory.
1. Put CMD/ENTRYPOINT as `["/path/to/proxify", "your_app", "one_arg", "more_args"]`
1. Build and run the enclave image.

# FAQ
1. Why not libev/libuv/libhv/asio ? Why so many threads?
   * This grew from a small test utility and we are in the process of improving it.
     We did not want to include much dependency so that this proxy is as easily portable
     as possible.
