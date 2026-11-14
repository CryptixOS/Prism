# Contributing to Prism

Prism is the standard library CryptixOS is built on, so it has to keep
building freestanding (no libc, no libstdc++, no exceptions, no RTTI) for
x86_64 and aarch64, with GCC and Clang. CI checks all of that on every push.

## Building and testing

```
cmake -S . -B build -G Ninja -DPRISM_TARGET=app -DPRISM_BUILD_TESTS=ON
cmake --build build && ctest --test-dir build --output-on-failure
```

- Every `Tests/<Suite>/<Name>/main.cpp` must be listed in both
  `Tests/CMakeLists.txt` and `Tests/<Suite>/meson.build`.
- A bug fix comes with a test that fails without it.
- `PRISM_HARDENED=ON` builds with bounds checks and overflow checks; the
  `sanitize` CI jobs run the tests under ASan and UBSan.
- `Tests/String/FormatGolden` holds `Prism::Format`'s output for a corpus of
  format strings, recorded from the patched fmt 10.2. If you change the corpus,
  run `Meta/format-golden/generate.sh`.

## Layout

- One named module per file: `Source/Prism/<Dir>/<Name>.ixx` is
  `Prism.<Dir>.<Name>`, with its out-of-line code in `<Name>.cpp`.
- Macros live in the few `.hpp` headers (`Core/Types.hpp`, `Core/Compiler.hpp`,
  `Debug/Assertions.hpp`, `Debug/Log.hpp`, ...). These headers never `import`:
  GCC can't mix that with modules reliably.
- Every new module goes into `PRISM_MODULE_INTERFACES` in `CMakeLists.txt` and
  into the install list in `meson.build`.
- One concept, one implementation. Before you add a second container, hash or
  range utility, extend the existing one.
- Check preconditions (indices, non-empty, sizes) with `PrismCheck(cond)`. It
  traps with `PRISM_HARDENED` and is an `assert` otherwise, and release builds
  without hardening drop the condition, so it must have no side effects:
  `bool ok = CheckedAdd(a, b, sum); PrismCheck(ok);`, never
  `PrismCheck(CheckedAdd(a, b, sum))`.

## Naming

| What                              | Style                     | Example                        |
|-----------------------------------|---------------------------|--------------------------------|
| Types, concepts, aliases          | `PascalCase`              | `StringView`, `IsIntegralV`    |
| Functions and member functions    | `PascalCase`              | `PushBack`, `FindFirstOf`      |
| Fallible variants                 | `Try` prefix, `ErrorOr`   | `TryEmplace`, `TryAppend`      |
| Enum values                       | `e` prefix, `PascalCase`  | `LogLevel::eWarn`              |
| Constants                         | `k` prefix or `PascalCase`| `kMaxPath`, `Npos`             |
| Template parameters               | `PascalCase`              | `T`, `Allocator`               |
| Data members                      | `m_` prefix               | `m_Size`                       |
| Static data members / globals     | `s_` / `g_` prefix        | `s_Instance`                   |
| Public data members of aggregates | `PascalCase`              | `KeyValuePair::Key`            |
| Locals and parameters             | `camelCase`               | `newSize`                      |
| Macros                            | `Prism` or `PRISM_` prefix| `PrismAssert`, `PRISM_HARDENED`|
| Namespaces                        | `PascalCase`              | `Prism::Ranges`                |

The language and range-for require a few lower-case names, and those are the
only exceptions: `begin`/`end` (and `cbegin`, `rbegin`, ...), `operator`s, and
the tuple protocol (`get`, `tuple_size`). The std-interop shims in
`Core/LanguageSupport.hpp` keep their std spelling.

The platform hooks (`panic`, `earlyPanic`, `Logger::Print`, see
`Core/PlatformHooks.hpp`) keep their names because the kernel defines them.

## Commits

Commit messages start with a tag: `[feat]`, `[fix]`, `[refactor]`, `[test]`,
`[build]`, `[ci/cd]`, `[docs]`, `[improvement]`.
