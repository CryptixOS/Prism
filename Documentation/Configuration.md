# Configuration macros

Every `PRISM_*` macro that changes how Prism is built. Defaults are set in
`Source/Prism/Core/Config.hpp` (target and platform detection in
`Core/Platform.hpp`), so every macro is always defined and `-Wundef` can catch
typos. Override one with `-D<NAME>=<value>` on the compiler command line, the
same for Prism and for everything that includes Prism's headers.

The CMake build sets the target macros from `PRISM_TARGET`; consumers that
compile Prism's sources themselves (CryptixOS) have to set them.

## Build options (CMake)

| Option                   | Default       | What it does |
|--------------------------|---------------|--------------|
| `PRISM_TARGET`           | `cryptix-app` | `app` (host), `cryptix-app` (CryptixOS userspace), `cryptix` (CryptixOS kernel), `carbonc` |
| `PRISM_BUILD_TESTS`      | `OFF`         | Build the unit tests (hosted targets only) |
| `PRISM_BUILD_BENCHMARKS` | `OFF`         | Build the benchmarks (hosted targets only) |
| `PRISM_WERROR`           | `ON`          | Treat warnings as errors |
| `PRISM_HARDENED`         | `OFF`         | Defines `PRISM_HARDENED=1` (see below) and adds `-D_FORTIFY_SOURCE=3`, `-fstack-protector-strong` and `-ftrivial-auto-var-init=zero` |
| `PRISM_SANITIZE`         | empty         | Comma-separated sanitizers for tests and benchmarks, e.g. `address,undefined` |
| `PRISM_EXTRA_INCS`       | empty         | An extra include directory for the library (the kernel's headers) |
| `PRISM_PKGCONFIG_NAME`   | `prism`       | Name of the installed pkg-config file |

## Target

| Macro                  | Default | Meaning |
|------------------------|---------|---------|
| `PRISM_TARGET_CRYPTIX` | `0`     | `1` when building for the CryptixOS kernel: no SIMD, no floating point in `Limits`, the kernel's panic and log hooks, and the `PRISM_USE_NAMESPACE` names exported globally |
| `PRISM_TARGET_CARBONC` | `0`     | `1` when building for carbonc: no stack traces |
| `PRISM_BUILD_CRYPTIX_KERNEL` | undefined | Sets `PM_CONTEXT_KERNEL` instead of `PM_CONTEXT_USERSPACE` |
| `PRISM_TARGET_X86_64`, `PRISM_TARGET_AARCH64`, `PRISM_TARGET_IA32`, `PRISM_TARGET_RISCV64`, `PRISM_TARGET_LOONGARCH64` | detected | The architecture (`Core/Platform.hpp`), with `PRISM_TARGET_STRING` its name |
| `PRISM_SIMD_SSE2_PRESENT`, `PRISM_SIMD_NEON_PRESENT` | detected | Whether SSE2 / NEON can be used |

## Behaviour

| Macro                        | Default | Meaning |
|------------------------------|---------|---------|
| `PRISM_HARDENED`             | `0`     | Bounds checks in `operator[]`, `Front()`, `Back()` and friends of the containers, views and strings, and overflow checks in their size calculations, also in release builds. A failed check traps (`__builtin_trap`, no message). Without it, the same checks are `assert`s (debug builds only) |
| `PRISM_USE_NAMESPACE`        | `0`     | `1` also exports the most used names (`Vector`, `String`, `Find`, ...) into the global namespace |
| `PRISM_PREFIXLESS_MACROS`    | `0`     | `1` also defines `Assert`, `AssertMsg`, `AssertNotReached`, `ToDo`, ... next to `PrismAssert` & co. (always on for the kernel) |
| `PRISM_LOG_ENABLE`           | `1`     | `0` compiles the `Prism*NoAlloc` log macros out |
| `PRISM_BUILD_DEBUG`          | `0`     | `1` enables `PrismDebug` |
| `PRISM_DEBUG`                | `0`     | `1` enables debug-only helpers such as `BitSpan::PoisonAll` |
| `PRISM_STD_INTEROP`          | `1` hosted, `0` kernel | Prism's iterator and range concepts accept the standard library's iterators and ranges, and Prism's views are std views |
| `PRISM_DISABLE_STDHASH`      | `0`     | `1` skips the `std::hash` specializations for `Path` and `PathView` |
| `PRISM_LANGUAGE_SUPPORT_OWN` | `0`     | `1` always uses Prism's own definitions of the `std::` names the compiler needs (`initializer_list`, comparison categories, ...), even when the standard headers exist |
| `PRISM_NO_PLACEMENT_NEW`     | `0`     | `1` if the environment already defines placement `new` |
| `PRISM_ENUM_RANGE_MIN`, `PRISM_ENUM_RANGE_MAX` | `-128`, `127` | The values `EnumName` and `EnumContains` look at |
| `PRISM_PAGE_SIZE`            | `0x1000` | Page size of the slab allocator |
| `PRISM_MAX_PATH`             | `255`, or the kernel's `Limits::MAX_PATH_LENGTH` | Longest valid `PathView` |

## Set by Prism itself

These are derived, not meant to be set: `PRISM_MUTEX_CAN_YIELD` (whether
`Mutex` can call `sched_yield`), `PRISM_USE_INTRUSIVE_HASH_MAP`,
`PRISM_LS_*` (which standard headers `LanguageSupport.hpp` found) and
`PRISM_HAS_*` (compiler builtins).
