# 🗺️ Prism Roadmap (0.1 → 1.0)

This is the version-by-version plan for Prism, the freestanding C++23 standard library that
CryptixOS is built on. After a first version that makes everything that exists today reliable,
it is built around **milestones you can show off**: every version ends with a demo you can run,
benchmark, or post. The essential work (correctness, tests, API cleanup) is not skipped. It is
scheduled *alongside* the demos, so the base never rots underneath them.

0.1 is a plain checklist with exit criteria. From 0.2 on, each version has:

- **Headline**: the demo that marks the version as done. It is split into small stages, so you
  get a win every few weeks instead of one at the very end.
- **What the demo needs**: the library features the headline depends on.
- **Floor**: tests, cleanup and completeness work that must land in the *same* version, even
  though the demo doesn't strictly need it.
- **Exit criteria**: the demo works *and* the floor holds.

It is a plan, not a schedule. A version ships when it is *done*, not on a date.

**How to read the checkboxes:**
- `[x]`: done and working.
- `[ ]`: not done yet.
- `[ ]` 🚧: work in progress. It exists in the tree but is incomplete.

The full catalogue of planned features, grouped by area rather than by version, is in the
[feature roadmap](Documentation/FeatureRoadmap.md).

## Up next: stricter compiler warnings

The build uses `-Wall -Wextra`, plus `-Werror` while `PRISM_WERROR` is on (the default). The
flags are `PRIVATE` to Prism's own targets, so they don't leak into consumers. CryptixOS
compiles Prism's sources with its *own* flags. `-Wpedantic` is intentionally **not** used:
Prism relies on GNU extensions (statement expressions in `PM_Try*`, named variadic macro
arguments, `__int128`, …).

Every header is also compiled as part of a consumer's code, so whatever warnings a consumer
enables apply to Prism's headers too. Being clean under strict flags here keeps CryptixOS clean.

Roll each flag out as a warning first (`-Wno-error=<flag>`), fix the fallout, then promote it to
an error. Guard GCC-only / clang-only flags with `check_cxx_compiler_flag`.

- [x] `-Wshadow`: locals and parameters shadowing members (common in containers)
- [x] `-Wnon-virtual-dtor` / `-Wdelete-non-virtual-dtor`: `LogSinkBase`, `RefCounted` hierarchies
- [x] `-Wsuggest-override` / `-Winconsistent-missing-override`
- [x] `-Wold-style-cast`: library headers should use named casts
- [x] `-Wcast-align` (`=strict` on GCC): unaligned access in `ByteStream`/`Endian`, matters on aarch64
- [x] `-Wnull-dereference`: provable null derefs (mostly GCC, needs optimization)
- [x] `-Wundef`: `#if PRISM_FOO` with `PRISM_FOO` undefined (config macro typos)
- [x] `-Wdouble-promotion`: accidental `float` → `double` in `Math/`
- [x] `-Wimplicit-fallthrough`: already used by CryptixOS, so Prism headers must pass it
- [x] `-Wmissing-declarations` (`-Wmissing-prototypes` on clang): non-inline functions without
  a prior declaration

Deliberately skipped for now: `-Wconversion` / `-Wsign-conversion` (too noisy for size and bit
math; maybe enable per directory later).

## Contents

- [The milestone ladder](#the-milestone-ladder)
- [Where we are: 0.0.x](#where-we-are-00x)
- [0.1: Stable Base](#01-stable-base)
- [0.2: CryptixOS Without std:: or fmt](#02-cryptixos-without-std-or-fmt)
- [0.3: Fast, and Proven Fast](#03-fast-and-proven-fast)
- [0.4: Text & Parsing](#04-text--parsing)
- [0.5: Hosted Prism](#05-hosted-prism)
- [0.6: Concurrency](#06-concurrency)
- [0.7: Kernel Toolkit](#07-kernel-toolkit)
- [0.8: Hardened](#08-hardened)
- [0.9: Packaged, Documented & Beta](#09-packaged-documented--beta)
- [1.0: First Stable Release](#10-first-stable-release)
- [Side quests](#side-quests)
- [Hall of fame](#hall-of-fame)
- [Must-Have Before 1.0](#must-have-before-10)

---

## The milestone ladder

The short version, if you only read one section:

| Version | Headline demo | Unlocked by |
|---|---|---|
| **0.1** | Every test builds and passes with GCC and Clang, in CI, on every commit | fixing broken tests, one API per concept, fmt gone |
| **0.2** | The CryptixOS kernel builds with **no `std::` and no fmt** | Format parity, Tuple/Apply/Invoke, LanguageSupport |
| **0.3** | A published benchmark table: Prism vs libstdc++ and fmt | benchmark runner, `HashMap`, sort, Format fast paths |
| **0.4** | Parse, edit and pretty-print a real JSON file, in the kernel *and* hosted | `GenericLexer`, number parsing, float formatting, UTF-8 views |
| **0.5** | A real CLI tool built on Prism alone: no libstdc++, optionally no libc | `Prism::OS`: files, streams, processes, `ArgsParser` |
| **0.6** | A tiny HTTP server on a Prism event loop, serving your browser | threads, sync primitives, `EventLoop`, sockets |
| **0.7** | CryptixOS loads ELF modules and a gzip initrd through Prism's parsers | ELF/DTB/ACPI parsers, DEFLATE, LZ4, address types |
| **0.8** | A 24-hour fuzzing campaign with zero crashes, sanitizer-clean | fuzz targets, ASan/UBSan CI, `Checked<T>`, SipHash |
| **0.9** | `import Prism;` + `find_package(Prism)`: a sample kernel boots in QEMU | modules, package config, docs, aarch64 parity |
| **1.0** | Use Prism in a new project and trust it not to break | API stability policy, release process |

The kernel cutover (no `std::`, no fmt) comes at **0.2**, because it's what CryptixOS needs
most. Hosted support comes at **0.5**, once the freestanding core is fast and complete.
Hardening comes late, when there is a lot of real code (JSON, ELF, DEFLATE) worth fuzzing.

Every feature from the [feature roadmap](Documentation/FeatureRoadmap.md) appears in a version
or in the nice-to-have list.

---

## Where we are: 0.0.x

Prism is at version `0.2.0`; this section describes where 0.0.x stood. It is consumed by the CryptixOS kernel and its userland, and ships
on the AUR. It builds with CMake (the primary build) and meson (deprecated), for the `app`,
`cryptix-app` and kernel targets.

The library already has:
- **Core**: type traits, concepts, `Limits`, `Bits`, `Integer128`, `UUID`, `Iterator`, and
  `LanguageSupport.hpp` replacing the standard library headers the compiler needs;
- **Containers**: `Vector`, `Array`, `Deque`, `Span`, `Tuple` (tuple-like, with `Apply` and
  `Invoke`), `UnorderedMap`, `Set`, `RedBlackTree`, `IntervalTree`, `RadixTree`, `RingBuffer`,
  `CircularQueue`, `Bitmap`/`BitSpan`, and several intrusive lists;
- **Strings**: `String` (SSO), `StringView`, `StringBuilder`, `StringUtils`, `CodePoints`;
- **Formatting**: `Prism::Format`, a compile-time-checked replacement for fmt 10.2 (`Format`,
  `FormatTo`, `FormatToN`, named arguments, `Join`, ANSI `Styled` output), plus `Printf`;
- **Memory**: `Ref`/`WeakRef`/`Scope`, slab, slob and bump allocators, `Endian<T>` storage,
  `ByteStream`/`BitStream`;
- **Utility**: `Optional`, `Expected`/`ErrorOr`, `Variant`, `Delegate`, `Atomic`, `Mutex`,
  `Path`/`PathView`, `Time`, `RangeAllocator`, `Checksum` (CRC32, Adler32);
- **Ranges**: Prism's own iterator concepts, plus views (`Filter`, `Transform`, `Take`,
  `Drop`, `Join`, `Reverse`, `Enumerate`, `Iota`, …);
- **Debug**: `Logger` with sinks, `SourceLocation`, assertions, `Stacktrace`, a UBSan runtime;
- **Test**: a test framework with crash tests, randomized (property) tests and benchmarks.

There are about 330 test cases across 40 test executables. CI builds and tests with GCC and
Clang through CMake, and builds the kernel cross target with meson.

What still holds it back:
- Some tests don't compile (`Ranges/Views`, `Utility/SimdIntrinsics`), and some are never
  built at all (`Find`, `NumericLimits`, `Variant`, `OptionParser`).
- Several concepts have more than one implementation (three intrusive lists, two `Hash`
  headers, two `SimdVector`s, two `Ranges`).
- fmt is gone from the sources, but the build still downloads and links it.

**Correctness and cleanup come before new features.** That is what 0.1 is for.

---

## 0.1: Stable Base

> **Theme:** everything that exists today must build, pass its tests and have exactly one
> obvious way to use it.

### Build & CI

- [x] **CMake is the primary build**
  meson still works but is deprecated.
- [x] **CI on every commit**
  GCC and Clang, Debug and Release, with the tests run through CTest.
- [x] **AUR package**
  Built and deployed automatically after CI passes on `master`.
- [x] **Every test directory is registered**
  `Find`, `NumericLimits`, `Variant` and `OptionParser` are built and pass. `OptionParser` is
  now a real getopt / getopt_long parser, checked by the mlibc getopt suite.
- [x] **Every test compiles and passes**
  Including `Ranges/Views` and `Utility/SimdIntrinsics`.
- [x] **Freestanding CI job**
  `cryptix` and `carbonc` build with `-ffreestanding -fno-exceptions -fno-rtti`, and Prism's
  own sources with `-nostdinc++` (`PRISM_STRICT_FREESTANDING`), for x86_64 and aarch64.
  `Meta/ci/check-hooks.sh` checks that the library needs nothing beyond the platform hooks.
- [x] **Every header compiles on its own**
  `prism_header_check` compiles each `.hpp` in its own translation unit (the modules are
  compiled on their own anyway).

### Drop fmt

- [x] **No fmt in the sources**
  No fmt includes or calls. Every Prism type has a Prism `Formatter`. `Log`, `Logger` and the
  assertion macros take `Prism::FormatString`.
- [x] **No fmt in the build**
  Remove the fmt download, `fmt-10.2.0.patch`, `PRISM_USE_SYSTEM_FMT`, the `FMT_*` defines,
  and fmt from the package config, pkg-config and CI dependencies. (The patch now lives in
  `Meta/format-golden`, which only uses fmt to regenerate the golden tests.)

### One API per concept

- [x] **One intrusive list**
  `IntrusiveListV3` won and is now `Prism::IntrusiveList`; the ownership comes from the hook
  (`IntrusiveListHook<T>`, `IntrusiveRefListHook<T>`, `IntrusiveOwningListHook<T>`).
- [x] **No duplicate headers**
  `Utility/SimdVector` and the empty `Utility/Hash` are gone, `Core/Ranges` is
  `Ranges/Access` (`Prism.Ranges.Access`).
- [x] **Naming pass**
  The convention is in [`CONTRIBUTING.md`](CONTRIBUTING.md); the std-style leftovers
  (`has_value`, `value_or`, `synth_three_way`) are gone.
- [x] **Stale headers fixed or removed**
  The last one, `Version.hpp.in`, included headers the module conversion removed; it is now
  the generated module `Prism.Core.Version`.

### Contracts

- [x] **Platform hook contract**
  [`Core/PlatformHooks.hpp`](Source/Prism/Core/PlatformHooks.hpp), checked in CI against the
  symbols a freestanding `libprism.a` really leaves undefined.
- [x] **Config macro reference**
  [`Documentation/Configuration.md`](Documentation/Configuration.md). Every macro has a default
  in `Core/Config.hpp`, so `-Wundef` catches typos.

### Exit criteria for 0.1

- [x] CI is green: every test directory builds and passes with GCC and Clang.
  (Verified locally on x86_64; the aarch64 jobs only run in CI.)
- [x] The freestanding build succeeds for x86_64 and aarch64.
- [x] Nothing in the build, the sources or CI references fmt.
- [x] Each concept has one implementation.

---

## 0.2: CryptixOS Without std:: or fmt

> **Theme:** the kernel stops depending on libstdc++ and fmt. Prism is the only standard
> library it uses.

### Headline

- [ ] **Stage 1: fmt-free kernel**
  CryptixOS builds with fmt removed from its tree. Every `fmt::format` becomes
  `Prism::Format`, and every `fmt::formatter` becomes a `Prism::Formatter`.
  Prism's side is done (no fmt anywhere, golden-tested `Format`); what is left is in the
  CryptixOS tree, which still has `fmt::` in 18 files and pins a pre-modules Prism.
- [ ] **Stage 2: std-free kernel**
  `grep -r 'std::' Kernel/` returns nothing outside `LanguageSupport.hpp`. Syscall dispatch
  uses `Prism::Tuple`, `Apply` and `Invoke`. (Kernel work: 10 files still use `std::`.)
- [ ] **Stage 3: same output**
  The kernel's boot log is the same before and after, character for character. (Kernel work.
  Note that the patched fmt printed nothing for `{:#p}` with an integer; Prism prints the
  address, so those boot log lines will differ on purpose.)

### What the demo needs

- [x] **`Tuple`, `Apply`, `Invoke`**
  `Tuple` is tuple-like, with rvalue `Get`, `Swap` and cv-qualified `TupleSize`, and covers
  everything the syscall layer did with `std::tuple`, `std::apply` and `std::invoke`.
- [x] **`LanguageSupport.hpp`**
  The few `std::` names the compiler itself requires (`initializer_list`, comparison
  categories, `coroutine_traits`, …) are provided in one place.
- [x] **`Logger` takes `FormatString`**
- [x] **Format parity with patched fmt 10.2**
  Every format string the kernel uses produces the same output it did with fmt. Stay as lenient
  as the patched fmt (`{:#p}` and other specs real fmt rejects still work). The golden tests
  check 5745 spec/value combinations byte for byte; the few deliberate differences (fmt bugs,
  and `{:#p}` with integers) are listed in `Tests/String/FormatGolden/Corpus.hpp`.
- [x] **`Formatter` for every kernel type**
  Addresses, PIDs, error codes, device IDs and enums (through `magic_enum` names).
- [x] **`std::` audit of Prism itself**
  Remaining uses in `Iterator.hpp`, `Concepts.hpp`, `StringView.hpp`, `Range.hpp`,
  `Views.hpp` and `TypeTraits.hpp` are either removed or moved behind `LanguageSupport.hpp`.
  `std::` is now only in `LanguageSupport.hpp` and behind `PRISM_STD_INTEROP`.

### Floor

- [x] **Format golden tests**
  Run a corpus of format strings and arguments through both fmt 10.2 and `Prism::Format` once,
  and check the recorded output into the tests. `Tests/String/FormatGolden`, regenerated by
  `Meta/format-golden/generate.sh`.
- [x] **Format compile-time errors are readable**
  A bad format string produces one short diagnostic naming the problem, not a template wall:
  the error points at the format string and the next notes say why
  (`InvalidFormatString`, `"argument not found"`). `Tests/String/FormatCompileFail` checks it.
- [x] **Remove the deprecated intrusive lists and duplicate headers**
- [x] **Container tests**
  Every container has tests for the basics plus edge cases (empty, one element, growth,
  move-only types): `Tests/Containers/EdgeCases`, which found broken `Span::SubSpan`, a
  `SplitBuffer` that didn't compile (and moved its gap the wrong way), a `CircularQueue` that
  only compiled for bytes, `Array<T, 0>`, `Set` iterators and duplicate inserts, and an
  `IntervalTree` whose subtree maxima went stale on rotation.

### Exit criteria for 0.2

- [ ] CryptixOS builds and boots with no fmt and no `std::` outside `LanguageSupport.hpp`.
  (Kernel work: Stages 1 and 2.)
- [ ] The boot log is unchanged.
- [x] The format golden tests pass.

---

## 0.3: Fast, and Proven Fast

> **Theme:** the kernel now leans on Prism for everything. Measure it, and make the hot paths
> fast.

### Headline

- [ ] **Stage 1: the benchmark runner**
  `ctest -L Benchmark` runs every `BENCHMARK_CASE` and prints a table with min, median and
  spread, in a stable format.
- [ ] **Stage 2: the comparison**
  Each benchmark has a libstdc++ (or fmt) twin. The table shows Prism vs standard side by
  side.
- [ ] **Stage 3: post it**
  Publish the table in `Documentation/Benchmarks.md`, with the compiler, flags and CPU.

### What the demo needs

- [ ] **`HashMap` / `HashTable` with open addressing**
  Benchmark it against `UnorderedMap`, and decide whether it replaces it.
- [ ] **Sorting**
  Introsort for `Sort`, plus `StableSort` and `QuickSelect`.
- [ ] **Format fast paths**
  Fast integer-to-string conversion, no allocation for `FormatTo` into a buffer, and
  `FormattedSize` without formatting twice.
- [ ] **`Vector` / `String` growth**
  Growth policy tuned, trivially relocatable types moved with `memcpy`.
- [ ] **`InlineVector<T, N>` and `FixedArray<T>`**
  For hot paths that shouldn't touch the heap.

### Floor

- [ ] **Binary size report**
  CI records the size of a sample program using `Format`, compared to the same program using
  fmt. A size regression fails the job.
- [ ] **Compile-time report**
  The time to compile the `all headers` translation unit is tracked over time.
- [ ] **Allocator hook**
  Containers allocate through one global allocator hook (or an explicit allocator), so the
  kernel can count and limit Prism's allocations.

### Exit criteria for 0.3

- [ ] The benchmark table is published and regenerated by CI.
- [ ] `HashMap`, sort and `Format` are within 10% of libstdc++/fmt, or faster.
- [ ] Binary size of the sample `Format` program is no bigger than with fmt.

---

## 0.4: Text & Parsing

> **Theme:** kernels and tools parse a lot of text: command lines, config files, JSON. Prism
> should make that easy and safe.

### Headline

- [ ] **Stage 1: the kernel command line**
  CryptixOS parses its boot command line with `GenericLexer` instead of hand-written loops.
- [ ] **Stage 2: JSON**
  Parse a real JSON file (for example a `compile_commands.json`), change a field, and
  pretty-print it back, hosted.
- [ ] **Stage 3: in the kernel**
  The same JSON code runs in CryptixOS, with no floating point if it's disabled.

### What the demo needs

- [ ] **`GenericLexer`**
  Peek, consume, consume-until and number reading, over a `StringView`.
- [ ] **Number parsing**
  `ToInt<T>` and `ToFloat` with `from_chars` behaviour: no locale, explicit error on
  overflow and trailing garbage.
- [ ] **Floating-point formatting**
  Shortest round-trip output (Ryu or Dragonbox), behind a switch so the kernel can leave it
  out.
- [ ] **UTF views**
  `Utf8View`, `Utf16View` and `Utf32View` with validation and code point iteration.
- [ ] **`JsonValue`, `JsonObject`, `JsonArray`**
  A parser and serializer with an allocation-free streaming reader.

### Floor

- [ ] **`FlyString`** (interned strings) for identifiers that are compared a lot.
- [ ] **`Base64`, `Hex` and URL encoding helpers**
- [ ] **`String` tests for UTF-8**
  Invalid sequences, overlong encodings, surrogates and display width.

### Exit criteria for 0.4

- [ ] The JSON round trip produces byte-identical output for a corpus of real files.
- [ ] Number parsing and float formatting round-trip every `u64`, `i64` and a large random
      set of doubles.
- [ ] CryptixOS uses `GenericLexer` for its command line.

---

## 0.5: Hosted Prism

> **Theme:** Prism is good enough for ordinary programs, not just kernels. Add a thin,
> optional OS layer, the way LibCore sits on top of AK.

### Headline

- [ ] **Stage 1: hello, files**
  A program reads a file, counts lines, words and bytes (`prism-wc`), and prints the result
  with `Println`.
- [ ] **Stage 2: a real tool**
  `prism-tree` or `prism-grep`: walks a directory tree, takes options through `ArgsParser`,
  and prints colored output with `Styled`.
- [ ] **Stage 3: no libstdc++**
  The tool links without libstdc++. Stretch goal: without libc, on raw Linux syscalls.

### What the demo needs

- [ ] **`Prism::OS` target**
  A separate target, enabled with `PRISM_HOSTED`, which freestanding headers never include.
- [ ] **Files**
  `File`, `Directory`, `DirIterator`, `MappedFile`, and `FileSystem::` helpers (copy, remove,
  stat).
- [ ] **Streams**
  One `Stream` interface (read, write, seek), with file, buffered, memory and
  stdin/stdout/stderr implementations.
- [ ] **`ArgsParser`**
  Replaces `OptionParser`: typed options, positional arguments, generated `--help`.
- [ ] **Processes and environment**
  `Process` (spawn, wait), `Environment`.
- [ ] **Time**
  `MonotonicTime`, `UnixDateTime`, `Duration`, and date/time formatting.

### Floor

- [ ] **Linux backend on raw syscalls**
  So libc stays optional. A POSIX/libc backend can come later.
- [ ] **Hosted tests in CI**
  The OS layer is tested on Linux with a temporary directory.

### Exit criteria for 0.5

- [ ] The CLI tool works, and its output matches the GNU equivalent on a test corpus.
- [ ] It links without libstdc++.
- [ ] The freestanding CI job still passes: nothing from `Prism::OS` leaked into the core.

---

## 0.6: Concurrency

> **Theme:** both the kernel and hosted programs are multi-threaded. Prism's primitives must
> be correct under contention, not just in a single-threaded test.

### Headline

- [ ] **Stage 1: a thread pool**
  A work-stealing thread pool runs a parallel sort on 8 threads.
- [ ] **Stage 2: a lock-free queue under stress**
  An MPSC queue passes a 1-hour stress test, with every item accounted for.
- [ ] **Stage 3: a tiny HTTP server**
  A server built on Prism's `EventLoop` and sockets serves a page to your browser.

### What the demo needs

- [ ] 🚧 **`Atomic<T>` completeness**
  All memory orders, and `Wait`/`Notify` through platform hooks.
- [ ] 🚧 **Locks**
  `Mutex` exists. Add `SpinLock`, `RecursiveSpinLock`, `RWLock` and `SeqLock`, with
  `Locker<T>` RAII and an IRQ-safe variant for the kernel.
- [ ] **`Once` and lazy initialization**
  Including cleaning up `Eternal`.
- [ ] **Hosted threads**
  `Thread`, `ConditionVariable`, `Semaphore` and thread-local storage.
- [ ] **Lock-free queues**
  SPSC and MPSC, usable in the kernel.
- [ ] **`EventLoop`**
  Timers and fd notifiers (epoll on Linux).
- [ ] **Sockets**
  TCP, UDP and Unix sockets, with `IPv6Address`, `SocketAddress` and `MACAddress` next to
  `IPv4Address`.

### Floor

- [ ] **ThreadSanitizer job**
  Every concurrency test runs under TSan in CI.
- [ ] **Lock ordering checks**
  An optional debug mode that reports lock-order inversions.

### Exit criteria for 0.6

- [ ] The stress tests pass under TSan.
- [ ] The HTTP server handles 10 000 requests from a load generator without leaking.
- [ ] CryptixOS uses Prism's locks for its own spinlocks.

---

## 0.7: Kernel Toolkit

> **Theme:** the things a kernel needs that a generic library usually skips.

### Headline

- [ ] **Stage 1: ELF**
  CryptixOS loads kernel modules through Prism's ELF parser.
- [ ] **Stage 2: compressed initrd**
  The initrd ships gzip-compressed and is inflated at boot with Prism's DEFLATE.
- [ ] **Stage 3: device tree**
  The aarch64 port reads its memory map and devices through Prism's DTB parser.

### What the demo needs

- [ ] **Parsers**
  `ELF`, `DeviceTree` and `ACPI` tables, in a separate `Prism::Formats` target. Every parser
  checks every offset and length.
- [ ] **Compression**
  DEFLATE/zlib inflate and LZ4.
- [ ] **Address types**
  `PhysicalAddress`, `VirtualAddress` and `PageRange` as distinct types (`DistinctNumeric`),
  so they can't be mixed up.
- [ ] **Per-CPU storage helpers**
- [ ] **Hashes**
  CRC32C, xxHash, SHA-1/SHA-256 and MD5, for filesystems and network protocols.

### Floor

- [ ] **UBSan runtime complete**
  Every handler is implemented and tested.
- [ ] **KASAN-style shadow memory helper**
- [ ] **`IntervalTree` / `IntrusiveRedBlackTree` augmentation hooks**
  For the kernel's VM region and MMIO range trees.

### Exit criteria for 0.7

- [ ] CryptixOS boots using Prism's ELF and DEFLATE code.
- [ ] Every parser has tests built from real files (a real kernel module, a real DTB, real
      ACPI tables).

---

## 0.8: Hardened

> **Theme:** by now Prism parses untrusted input: JSON, ELF, DEFLATE, format strings. Prove
> it can't be crashed.

### Headline

- [ ] 🚧 **Stage 1: fuzz targets**
  libFuzzer targets for format strings, number parsing, `Path`, UTF-8 decoding, JSON, ELF,
  DEFLATE and `ArgsParser`. Done for what exists: `Fuzz/` has format strings, number parsing,
  `Path`, the option parser and strings. UTF-8, JSON, ELF and DEFLATE come with their parsers.
  First runs found `StringView::RFind` underflowing (and searching from 0 by default) and
  `String` appends reallocating on every call.
- [ ] **Stage 2: 24 hours**
  A 24-hour fuzzing campaign finds zero crashes, hangs or sanitizer reports.
- [ ] 🚧 **Stage 3: continuous**
  The fuzz targets run in CI on every commit for a short time, and on a schedule for longer.
  The per-commit run (60 s per target) is in CI; the scheduled long run is not.

### What the demo needs

- [x] **ASan and UBSan in CI**
  Every hosted test runs under both (`PRISM_SANITIZE`, the `sanitize` job, with
  `PRISM_HARDENED` on).
- [x] **`Checked<T>`**
  Overflow-checked arithmetic, plus saturating math, used in every size calculation
  (`Prism.Core.Checked`: `Checked<T>`, `CheckedAdd/Sub/Mul`, `Saturating*`, and
  `AllocationSize`/`GrowCapacity` in the allocating containers).
- [x] **Hash flooding resistance**
  SipHash (seeded) as the default hash for `HashMap` keys that come from outside.
  `SipHasher<T>` with `SipHash::SetGlobalKey`, checked against the reference vectors.
- [x] **Bounds checks**
  `operator[]` on containers and views asserts in debug builds. There is a cheap
  always-on mode for the kernel: `PRISM_HARDENED`, where every `PrismCheck` traps.

### Floor

- [x] **Stricter warnings promoted to errors**
  Everything from [Up next](#up-next-stricter-compiler-warnings).
- [ ] **Static analysis**
  clang-tidy (or GCC `-fanalyzer`) runs in CI with a curated set of checks.
- [ ] **Coverage**
  A coverage report is generated in CI. Core containers and Format are above 90%.

### Exit criteria for 0.8

- [ ] The 24-hour campaign is clean.
- [ ] CI runs ASan, UBSan, TSan and the short fuzzing job on every commit.

---

## 0.9: Packaged, Documented & Beta

> **Theme:** someone who isn't you can pick Prism up and use it.

### Headline

- [ ] **Stage 1: `find_package(Prism)`**
  A fresh CMake project finds an installed Prism and links `Prism::Prism`, `Prism::OS` and
  `Prism::Test`.
- [ ] **Stage 2: `import Prism;`**
  The same project uses the C++ module instead of headers.
- [ ] **Stage 3: the sample kernel**
  A minimal Limine kernel, built only from the docs, boots in QEMU and prints with
  `Prism::Format`.

### What the demo needs

- [ ] 🚧 **C++ modules**
  `Prism.cppm` stays in sync with the headers, and CI builds and tests it.
- [ ] 🚧 **Package config**
  Exported CMake targets and pkg-config. Packages for the AUR (exists), Nix, and vcpkg or
  Conan.
- [ ] **API documentation**
  Doxygen or mdBook, including a "freestanding integration guide": which hooks to implement and
  the minimal linker setup.
- [ ] **Examples**
  A Limine kernel, a UEFI application and a hosted CLI tool.

### Floor

- [ ] **aarch64 parity**
  Everything that works on x86_64 builds and passes its tests on aarch64 (under QEMU user
  mode in CI).
- [ ] **Changelog**
  Every user-visible change since 0.1 is listed.
- [ ] **Release candidates**
  `1.0-rc1`, `rc2`, …. The last one goes an agreed period with no API changes and no
  critical bugs.

### Exit criteria for 0.9

- [ ] Someone else builds the sample kernel from the docs alone.
- [ ] The modules build, the package config and both architectures are green in CI.

---

## 1.0: First Stable Release

> **Theme:** a small but complete, fast, freestanding standard library, with an optional
> hosted layer.
>
> 1.0 means: *you can build a kernel or a program on Prism, and a 1.x update will not break
> your code.*

- [ ] **API stability policy**
  Says which headers and names are stable for all 1.x releases, and which are explicitly
  experimental (a `Prism::Experimental` namespace or a documented list).
- [ ] **Versioning policy**
  `1.x.y` patch releases are bug fixes only. `1.x` minor releases add features without
  breaking source compatibility. Deprecations last at least one minor release.
- [ ] **Release artifacts**
  A tagged release with source tarball, SHA-256 checksums and release notes.
- [ ] **Security process**
  Explains how to report vulnerabilities and how fixes are released.

---

## Side quests

Small, self-contained wins for when you need a quick bit of progress between bigger stages.
Each one lists what it depends on. None of them blocks a version, so pick any one that is
unlocked.

| Side quest | Unlocked after | What it proves |
|---|---|---|
| `constexpr` `Format` used inside a `static_assert` | now | the formatting engine is fully constexpr |
| A colored, aligned table printer using `Styled` and `{:^20}` | now | the FormatStyle API |
| `Badge<T>`, `ScopeGuard`, `TemporaryChange` | now | tiny, immediately useful utilities |
| Port a few SerenityOS AK tests to Prism | 0.1 | API parity with AK |
| `Format` vs fmt binary size on a "hello world" kernel | 0.2 | the fmt replacement isn't bloated |
| Ranges: `Zip`, `Split`, `Chunk`, `Stride` views | 0.2 | ranges completeness |
| A Mandelbrot renderer using `FixedPoint<N>` | 0.3 | float-free math for the kernel |
| PRNGs: xoshiro and PCG behind the `Random` hook | 0.3 | `Algorithm/Random` beyond the basics |
| An INI `ConfigFile` reader for Aurora | 0.4 | `GenericLexer` in real use |
| `prism-hexdump` | 0.5 | streams + `Format` padding |
| A Brainfuck or Lua-subset interpreter | 0.5 | containers and strings under load |
| A chat server on `EventLoop` | 0.6 | many long-lived sockets |
| A `readelf` clone on Prism's ELF parser | 0.7 | the parser on arbitrary binaries |

---

## Hall of fame

Record every milestone the day it happens: date, commit, and a screenshot, benchmark or log
in `Documentation/milestones/`.

| Milestone | Date | Commit | Proof |
|---|---|---|---|
| CI fully green | | | |
| First freestanding-only build | | | |
| No fmt in the sources | | | |
| No fmt in the build | | | |
| CryptixOS boots without fmt | | | |
| CryptixOS boots without `std::` | | | |
| First benchmark table | | | |
| Prism `Format` beats fmt on a benchmark | | | |
| First JSON round trip | | | |
| First CLI tool on Prism alone | | | |
| First HTTP response from a Prism server | | | |
| CryptixOS loads a module through Prism's ELF parser | | | |
| 24-hour fuzzing campaign clean | | | |
| `import Prism;` works | | | |
| Someone else's project uses Prism | | | |

---

## Must-Have Before 1.0

If the version plan above slips or is reordered, these remain **non-negotiable** for 1.0:

- [ ] **Freestanding core**
  Core, containers, strings, memory, algorithms, utilities, ranges and math build with
  `-ffreestanding -nostdlib -fno-exceptions -fno-rtti`, for x86_64 and aarch64.
- [ ] **No external dependencies**
  No fmt and no libstdc++ in the core. magic_enum and demangler are vendored or replaced.
- [ ] **Fallible by default**
  Every allocating operation has a `Try*` variant returning `ErrorOr<T>`.
- [ ] **Formatting**
  Compile-time-checked `Format` with fmt 10.2 parity, readable errors, and optional floating
  point.
- [ ] **Containers**
  One implementation per concept, all allocator-aware, all tested, including `HashMap`.
- [ ] **Text**
  UTF-8 strings and views, number parsing, a lexer and JSON.
- [ ] **Concurrency**
  Atomics, locks and lock-free queues that pass TSan stress tests.
- [ ] **Hosted layer**
  Files, streams, processes, time and threads behind `PRISM_HOSTED`.
- [ ] **Safety**
  Clean under ASan, UBSan and TSan. Fuzzed parsers. Strict warnings as errors.
- [ ] **Performance**
  A published, CI-regenerated benchmark table against libstdc++ and fmt.
- [ ] **CryptixOS**
  The kernel and userland build on Prism alone.
- [ ] **Packaging**
  CMake package config, pkg-config, C++ modules, and at least the AUR and Nix packages.
- [ ] **Docs**
  API reference, freestanding integration guide, platform hook contract and examples.

### Nice to have (allowed to slip past 1.0)

- [ ] Floating-point *parsing* fast path (Eisel-Lemire)
- [ ] `Function<Sig>` with small-buffer storage replacing `Delegate`
- [ ] `OrderedHashMap`, `Trie`, `BinaryHeap`
- [ ] Freestanding `<cmath>` subset with constexpr variants
- [ ] Windows and macOS backends for `Prism::OS`
- [ ] A POSIX/libc backend alongside raw Linux syscalls
- [ ] Coroutine support (`Task<T>`, `Generator<T>`) on top of `EventLoop`
- [ ] SIMD-accelerated `StringView` search and `Memory::` routines
