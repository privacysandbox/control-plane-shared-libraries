# Privacy Sandbox Shared Libraries

The Privacy Sandbox Shared Libaries provide facilities to deploy and operate applications in a secure execution environment at scale. This includes managing cryptographic keys, buffering requests, keeping track of the privacy budget, accessing storage, orchestrating a policy-based horizontal autoscaling, and more. The shared libraries are used by the Privacy Sandbox Aggregation Service.

The shared libraries isolates the application from the specifics of the cloud environment, allowing for the application to be deployed on any supported cloud vendor without changes to the application. The shared libraries provide abstractions for communicating with the cloud environment under the directory name "CPIO".

# Building

This repo depends on Bazel 4.2.2 with JDK 11 and GCC 9.  The following environment variables should be set in your local environment (the exact location will depend on your environment):

```
JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
CXX=/usr/bin/g++-9
CC=/usr/bin/gcc-9
```

The `.bazelrc` file specifies the minimum JDK and GCC version.

To build the repo, run `bazel build ...`.

# Contribution

Please see CONTRIBUTING.md for details.
