# Prism

**Prism** is a modern C++ standard library and runtime environment tailored for freestanding and kernel-space development. It powers the core of [CryptixOS](https://github.com/CryptixOS/CryptixOS), offering high-performance, modular, and allocator-aware components built from scratch for environments without a standard C++ runtime.

---

![License](https://img.shields.io/github/license/CryptixOS/CryptixOS)
![Architecture](https://img.shields.io/badge/arch-x86__64%20%7C%20aarch64-blue)
[![Build](https://github.com/CryptixOS/CryptixOS/actions/workflows/build.yml/badge.svg)](https://github.com/CryptixOS/CryptixOS/actions/workflows/build.yml)
[![Check-Typos](https://github.com/CryptixOS/CryptixOS/actions/workflows/typos.yml/badge.svg)](https://github.com/CryptixOS/CryptixOS/actions/workflows/typos.yml)



## Goals

- Freestanding and header-only (where possible)
- Designed for kernel use and low-level systems programming
- Clean, STL-like API with zero-cost abstractions
- No dependencies on libstdc++, libc++, or libm

---

## Features

### Core Types
- `BasicString`, `Vector`, `Deque`, `UnorderedMap`, `RedBlackTree`, `Optional`, `Span`, etc.
- SSO-enabled string implementation (`BasicString`)

### Utilities
- `Format` — type-safe formatting engine (like `fmtlib`)
- `Expected`, `Result`, `ErrorCode` — error handling without exceptions
- Type traits, concepts, and compile-time utilities
- Smart pointers: `Ref`, `Scope`, `WeakRef`

### Testing Support
- `Prism/Test`: unit testing framework modelled after SerenityOS' LibTest
  - `TEST_CASE`, `BENCHMARK_CASE`, `TEST_SETUP`
  - `EXPECT`, `EXPECT_EQ`, `EXPECT_NE`, `EXPECT_EQ_TRUTH`, `EXPECT_EQ_FORCE`,
    `EXPECT_APPROXIMATE`, `FAIL`, `TRY_OR_FAIL`, `MUST`, `VERIFY`
  - crash tests: `EXPECT_CRASH`, `EXPECT_CRASH_WITH_SIGNAL`, `EXPECT_NO_CRASH`
  - property-based tests: `RANDOMIZED_TEST_CASE`, `GEN`, `ASSUME`, generators
    in `Prism::Test::Gen`, automatic shrinking of failing inputs
  - test runner options: `--list`, `--tests`, `--bench`,
    `--benchmark_repetitions N`, `--randomized_runs N`, `--seed N`, `[pattern]`

---

## Building the Kernel & Image

Just initialize the build directory with: 
```
meson setup build --buildtype=debugoptimized
```
and then you can build the Prism library like so
```
ninja -C build
```

Or with CMake:
```
cmake -S . -B build -G Ninja
cmake --build build
```

## Running the tests

```
meson setup build-tests -Dbuild_tests=true
meson test -C build-tests
```
or
```
cmake -S . -B build-tests -G Ninja -DPRISM_BUILD_TESTS=ON
cmake --build build-tests && ctest --test-dir build-tests
```
A test binary can also be run directly, e.g.
`build-tests/Tests/String/Format --list` or `... 'join*'`.

## Installation 

To install Prism you just run this just after you've built it
```
ninja -C build install
```

## Usage

Prism is integrated with CryptixOS, but it can be used in any freestanding or embedded project. Just add the `Prism` source tree to your build and configure it as a system or static library.

> Note: Prism assumes a freestanding environment — no exceptions, RTTI, or dynamic allocations unless explicitly provided.

---

## Project Structure

  - CrossFiles => **meson config files for cross compilation**
  - Source/ => **Source Root**
    - Prism/
      - Algorithm/ 
      - Containers/
      - Core/
      - Debug/
      - Memory/
      - String/
      - Test/ => **unit testing framework**
      - Utility
  - subprojects => **meson subprojects**
  - Tests => **Unit Tests**
  - meson.build => **Main Build Script**
  - CMakeLists.txt => **CMake Build Script** (kept equivalent to meson.build)
  - typos.toml => **typos configuration**

---

## License

Prism is licensed under the **GPL-3 License** 

---

## Credits

Prism is inspired by the design goals of:
- The C++ STL (with a focus on zero overhead)
- `fmtlib`, `abseil`, and `eastl`
- Operating system development principles (e.g. SerenityOS, Redox, linux, managarm, toaros)

---

## Maintained by

**[CryptixOS Project](https://github.com/CryptixOS)**  

