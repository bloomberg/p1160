# Purpose

Code in this repository is for demonstration purposes only.  It is not meant to be used as-is in for any other purpose than being able to follow the WG21 ISO C++ Standard Library proposal paper P1160, or its accompanying presentations available on YouTube.

Since this code places the proposed names into the `std::pmr` namespace while it is *not* part of the "implementation" (what ships with the compiler) it, technically, is undefined behavior.

The code and the simple "build system" worked with compilers and standard libraries at the time of the proposal, but it is not maintained as the proposal could not go forward.

The code itself was a first introduction into the idea, not what a modern, or postmodern C++ library component should look like.  The paper discusses this, and notes that the "API" should be rethinked in light of all the new core language and library features.

# P1160 Add Test Polymorphic Memory Resource To The Standard Library

This repository contains the source code and build system for an example implementation of a `std::pmr` compatible polymorphic memory resource, intended for use in instrumented tests. The paper which describes the motivation, operation, and other aspects of this code can be found in the [WG21 paper repository](http://wg21.link/p1160 "P1160R0").

# Prerequisites

 * CMake
 * MSVC 2017 or gcc-9 

At the time of writing the Microsoft Visual Studio 2017 and gcc-9 (trunk) are the only compilers that implement `std::pmr` as specified in the C++17 specification, so those are the only to toolchains that have been tested (it's likely to compatible with more recent versions of those compilers)

The most relevant instructions for installing gcc (at the time of writing, 2019.Apr.20): See https://gcc.gnu.org/wiki/InstallingGCC and replace gcc-4.6.0.tar.gz with a snapshot such as https://gcc.gnu.org/pub/gcc/snapshots/LATEST-9/

libc++ (i.e. the native library for clang) does not yet have a standard `pmr` implementation.  I was able to compile the code in this repository using `<experimental/memory_resource>` that libc++ implements currently (May 2019).

# Recorded Presentations

## C++ Now 2019, Aspen, CO

[![C++ Now 2019, Aspen, CO](https://img.youtube.com/vi/48oAZqlyx_g/maxresdefault.jpg)](https://youtu.be/48oAZqlyx_g)

# How to Understand the Code

The repository consists of 5 major parts:

  * supportlib -- macros and printing helpers (static lib)
  * stdpmr -- the implementations of the proposed types and the exception testing algorithm (static lib)
  * pstring -- a series of examples of testing and fixing an imaginary (and quite pathological) string class (executables)
  * exception_testing -- an example using the `exception_test_loop`
  * patchpmr -- hacks to make clang with libc++ and older GNU libraries with experimental support work

Please read the paper, or watch the presentation, to better understand the repository contents.

# Versions of this Repository

Please use the `main` branch of this repository *unless* you are following a presentation from a conference, or for a specific paper revision, in which case I suggest checking out the corresponding tag.

# Versioning

Git tag: R0 -- Corresponds to the paper revision P1160R0

Git tag: CppNow2019 -- added gcc support, reorganized the code, switched to cmake

Git tag: CppCon2019 -- fixed handling of 0 sized allocations to the standard way (allocating and not returning 0)

# Troubleshooting

If you find any issue or have any concerns please do not hesitate to open an [issue](../../issues).

Pull requests are welcome, however since this code accompanies a paper, changes that would change the essence of
the proposal would be better be addressed during WG21 discussions.
