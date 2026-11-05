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
- Minimal `assert`-based test framework
- Kernel-friendly output via serial or logging interface

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
      - Utility
  - subprojects => **meson subprojects**
  - Tests => **Unit Tests**
  - meson.build => **Main Build Script**
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

