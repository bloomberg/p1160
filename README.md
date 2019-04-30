# P1160 Add Test Polymorphic Memory Resource To The Standard Library

This repository contains the source code and build system for an example implementation of a `std::pmr`
compatible polymorphic memory resource, intended for use in instrumented tests. The paper which describes
the motivation, operation, and other aspects of this code can be found in the
[WG21 paper repository](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1160r0.pdf "P1160R0").

# Prerequisites

At this time only the Microsoft Visual Studio 2017 toolchain and trunk gcc implements `std::pmr` as specified
in the C++17 specification, so this example code requires that toolchain.

While this code has only been tested with the *Professional* variant of Microsoft Visual Studio 2017,
it should be compatible with all variants including the *Community* variant.

To use gcc (at the time of writing, 2019.Apr.20), in the word of Jonathan Wakely: See https://gcc.gnu.org/wiki/InstallingGCC and replace gcc-4.6.0.tar.gz with a snapshot such as https://gcc.gnu.org/pub/gcc/snapshots/LATEST-9/

The latest code uses cmake to generate the build system, so you will need cmake.  Haven't yet tried with Visual Studio cmake support, I will update here when I do.  
  
clang+libc++ does not yet have a standard `pmr` implementation.  I will attempt later to see if I can make the code work with `<experimental/memory_resource>` that libc++ implements currently (2019.Apr.30.)

# How to Understand the Code

The [Visual Studio project](pmr_test_resource.sln) consists of 5 major parts:
  * supportlib -- macros and printing helpers (static lib)
  * stdpmr -- the implementations of the proposed types and the exception testing algorithm (static lib)
  * pstring -- a series of examples of testing and fixing an imaginary (and quite pathological) string class (executables)
  * monitoring -- an example of test resource monitoring and replacing the default resource with a test resource
  * exception_testing -- an example using the `exception_test_loop`

Please read the paper to follow the logic.

# Versioning

Git tag: R0 -- Corresponds to the paper revision P1160R0

Git tag: CppNow2019 -- added gcc support, reorganized the code, switched to cmake

# Troubleshooting

If you find any issue or have any concerns please do not hesitate to open an [issue](../../issues).

Pull requests are welcome, however since this code accompanies a paper, changes that would change the essence of
the proposal would be better be addressed during WG21 discussions.
