# Prism

**Prism** is a modern C++ standard library and runtime environment tailored for freestanding and kernel-space development. It powers the core of [CryptixOS](https://github.com/CryptixOS/CryptixOS), offering high-performance, modular, and allocator-aware components built from scratch for environments without a standard C++ runtime.

---

![License](https://img.shields.io/github/license/CryptixOS/Prism)
![Architecture](https://img.shields.io/badge/arch-x86__64%20%7C%20aarch64-blue)
[![Build](https://github.com/CryptixOS/Prism/actions/workflows/build.yml/badge.svg)](https://github.com/CryptixOS/Prism/actions/workflows/build.yml)
[![Check-Typos](https://github.com/CryptixOS/Prism/actions/workflows/typos.yml/badge.svg)](https://github.com/CryptixOS/Prism/actions/workflows/typos.yml)
[![AUR](https://img.shields.io/aur/version/prism-cxx-git)](https://aur.archlinux.org/packages/prism-cxx-git)



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

## Building

Prism is built with CMake:
```
cmake -S . -B build -G Ninja -DPRISM_TARGET=app
cmake --build build
```

`PRISM_TARGET` selects what Prism is built for:

| Target        | Environment                    | fmt                         |
|---------------|--------------------------------|-----------------------------|
| `app`         | host (Linux, ...)              | system fmt                  |
| `cryptix-app` | CryptixOS userspace (default)  | bundled, patched fmt 10.2   |
| `cryptix`     | CryptixOS kernel               | bundled, patched fmt 10.2   |
| `carbonc`     | carbonc                        | bundled, patched fmt 10.2   |

The fmt choice can be overridden with `-DPRISM_USE_SYSTEM_FMT=ON|OFF`.
Bundled dependencies (patched fmt, magic_enum, demangler) are taken from
`subprojects/` when present, otherwise they're downloaded at configure time.

> **meson is deprecated.** `meson.build` is still there for backward
> compatibility (`meson setup build -Dtarget=app && ninja -C build`), but new
> options and features only land in CMake.

## Running the tests

```
cmake -S . -B build-tests -G Ninja -DPRISM_TARGET=app -DPRISM_BUILD_TESTS=ON
cmake --build build-tests && ctest --test-dir build-tests
```
A test binary can also be run directly, e.g.
`build-tests/Tests/String/Format --list` or `... 'join*'`.

## Installation

On Arch Linux, install [`prism-cxx-git`](https://aur.archlinux.org/packages/prism-cxx-git)
from the AUR. It installs headers to `/usr/include/prism-cxx` and the
libraries to `/usr/lib/prism-cxx`.

From source:
```
cmake --install build
```

Then use it from CMake:
```cmake
find_package(Prism REQUIRED)
target_link_libraries(app PRIVATE Prism::Prism)  # Prism::Test, Prism::TestMain
```
or with pkg-config: `pkg-config --cflags --libs prism` (`prism-cxx` for the
AUR package).

## Usage

Prism is integrated with CryptixOS, but it can be used in any freestanding or embedded project. Just add the `Prism` source tree to your build and configure it as a system or static library.

> Note: Prism assumes a freestanding environment — no exceptions, RTTI, or dynamic allocations unless explicitly provided.

---

## Project Structure

  - CrossFiles => **meson config files for cross compilation** (deprecated)
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
  - subprojects => **bundled dependencies**
  - Tests => **Unit Tests**
  - CMakeLists.txt => **Main Build Script**
  - meson.build => **meson Build Script** (deprecated, kept for backward compatibility)
  - Meta/aur => **AUR package**
  - typos.toml => **typos configuration**

---

## License

Prism is licensed under the **GNU General Public License v2.0 or later**
(`GPL-2.0-or-later`), see [LICENSE](LICENSE). Bundled dependencies keep their
own licenses: fmt and magic_enum (MIT), demangler (Apache-2.0).

---

## Credits

Prism is inspired by the design goals of:
- The C++ STL (with a focus on zero overhead)
- `fmtlib`, `abseil`, and `eastl`
- Operating system development principles (e.g. SerenityOS, Redox, linux, managarm, toaros)

---

## Maintained by

**[CryptixOS Project](https://github.com/CryptixOS)**  

