# Prism Roadmap

Prism aims to be a self-contained C++ standard library usable in **two modes**:

- **Freestanding**: kernels, bootloaders, firmware. No libc, no libstdc++, no exceptions, no RTTI.
- **Hosted**: ordinary userland programs on Linux (and later other OSes), with an OS layer for files, time, threads, and I/O.

The model is SerenityOS' AK and LibCore: one consistent vocabulary everywhere, with OS-specific functionality kept behind a thin, optional layer.

---

## Guiding principles

1. **Core is always freestanding.** Anything under `Core/`, `Containers/`, `String/`, `Memory/`, `Algorithm/`, `Utility/`, `Ranges/` and `Math/` must build with `-ffreestanding -nostdlib -fno-exceptions -fno-rtti`.
2. **Hosted features are opt-in.** They live in a separate module/target (`Prism::OS` or `Prism::Core-hosted`), enabled with `PRISM_HOSTED`.
3. **Fallible by default.** Every operation that allocates has a `Try*` variant returning `ErrorOr<T>`. Infallible wrappers `MUST()` the result.
4. **Allocator-aware.** Containers take an allocator, or a global allocator hook the platform supplies.
5. **Platform hooks, not platform code.** The library calls a small set of user-provided functions (`Prism::Platform::Panic`, `Allocate`, `Free`, `Log`, `Now`), and the kernel or hosted runtime implements them.

---

## Phase 0: Foundation and hygiene (now)

- [ ] Finish the **fmt → `Prism::Format` migration** and remove the fmt dependency entirely.
- [ ] Settle on **one** intrusive list (`IntrusiveList`, `IntrusiveListV3`, `IntrusiveRefList`): deprecate the others and keep a single API.
- [ ] Remove the duplicates: `Math/SimdVector` vs `Utility/SimdVector`, `Algorithm/Hash` vs `Utility/Hash`, `Core/Ranges` vs `Ranges/`.
- [ ] Write down the **platform hook contract** (`Platform.hpp`): the exact symbols a freestanding consumer must provide.
- [ ] Add a CMake option matrix: `PRISM_FREESTANDING`, `PRISM_HOSTED`, `PRISM_BUILD_TESTS`, `PRISM_MODULES`.
- [ ] CI: build both modes for x86_64 and aarch64 with GCC and Clang, and run the tests in hosted mode.
- [ ] Turn on `-Wall -Wextra -Werror` and add a sanitizer job (ASan/UBSan) for the hosted tests.

## Phase 1: Test coverage and correctness

- [ ] Unit tests for every container (`Vector`, `Deque`, `UnorderedMap`, `RedBlackTree`, `RadixTree`, `IntervalTree`, `RingBuffer`, …).
- [ ] Tests for `String`, `StringView`, `StringBuilder`, `StringUtils`, and the Format engine (compare the output against fmt 10.2 behaviour).
- [ ] Tests for `Optional`, `Variant`, `Expected`, `Tuple`, `Delegate`, `Ref`/`WeakRef`/`Scope`.
- [ ] Fuzz targets (libFuzzer, hosted): format strings, number parsing, `OptionParser`, `Path`, UTF-8 decoding.
- [ ] Benchmarks with `BENCHMARK_CASE` against libstdc++ equivalents.
- [ ] A test runner that can run **inside the kernel** (a freestanding `TestMain` that reports over a log sink).

## Phase 2: Fill the AK-equivalent gaps (freestanding)

### Core and types
- [ ] `ErrorOr<T>` / `Error` unified with `Expected`, plus `TRY()` everywhere.
- [ ] `Badge<T>`, `ScopeGuard` / `ArmedScopeGuard`, `TemporaryChange`.
- [ ] `DistinctNumeric<T, Tag>` (strong typedefs for PIDs, physical/virtual addresses).
- [ ] `Checked<T>` (overflow-checked arithmetic) and saturating math.
- [ ] `FixedPoint<N>` for kernel code that can't use floats.
- [ ] `NonnullRefPtr` / `NonnullOwnPtr` style non-null smart pointers.
- [ ] `Function<Sig>` with small-buffer storage (alongside or replacing `Delegate`).
- [ ] `Error` with an errno-compatible code set and string literals for freestanding builds.

### Containers
- [ ] `HashMap` / `HashTable` with open addressing (and decide whether it replaces `UnorderedMap`).
- [ ] `OrderedHashMap` (keeps insertion order).
- [ ] `FixedArray<T>` (runtime-sized, allocated once) and `InlineVector<T, N>` (SBO).
- [ ] `BinaryHeap` / `PriorityQueue`.
- [ ] `Trie` and `BitmapAllocator`-style range trackers.
- [ ] `CircularBuffer` and a lock-free SPSC/MPSC queue for kernel use.
- [ ] `SinglyLinkedList` and `IntrusiveRedBlackTree` improvements (augmentation hooks).

### Strings and text
- [ ] `FlyString` (interned strings), and `ByteString` vs UTF-8 `String` separation.
- [ ] `Utf8View`, `Utf16View`, `Utf32View` with validation.
- [ ] `GenericLexer` for parsing (cmdline, config files, ACPI/DT strings).
- [ ] Number parsing: `ToInt<T>`, `ToFloat` (from_chars-quality, locale-free).
- [ ] Floating-point formatting (shortest round-trip, e.g. Ryu or Dragonbox).
- [ ] `Base64`, `Hex`, `URL` encoding helpers.

### Memory
- [ ] Global allocator hook plus `kmalloc`-style adapters.
- [ ] `ArenaAllocator`, `PoolAllocator`, and a general-purpose freestanding heap (for example a TLSF or buddy allocator).
- [ ] `MemoryStream` / `FixedMemoryStream` with a unified `Stream` interface (Read/Write/Seek).
- [ ] `Endian<T>` wrapper types (`LittleEndian<u32>`, `BigEndian<u16>`) for parsing on-disk and network structures.
- [ ] `alignas`-aware `Uninitialized<T>` storage and `Box`-like owning pointers with custom deleters.

### Concurrency (freestanding primitives)
- [ ] `Atomic<T>` completeness (all memory orders, `wait`/`notify` via platform hooks).
- [ ] `SpinLock`, `RecursiveSpinLock`, `RWLock`, `SeqLock`, plus `Locker<T>` RAII.
- [ ] `Once` / call-once and lazy init (`Eternal` cleanup).

### Algorithms and math
- [ ] Complete `<algorithm>` coverage: `BinarySearch`, `LowerBound`, `Partition`, `Rotate`, `Unique`, `MinMax`, `AnyOf`, …
- [ ] Introsort, stable sort, and `QuickSelect`.
- [ ] Freestanding `<cmath>` subset: `sqrt`, `sin`/`cos`, `exp`/`log`, `pow`, `floor`/`ceil`, with constexpr variants.
- [ ] Hashes and checksums: CRC32/CRC32C, Adler32, xxHash, SipHash (for hash-flooding resistance), and SHA-1/SHA-256/MD5 for file systems and network protocols.
- [ ] PRNGs: xoshiro, PCG, and a `Random` hook for platform entropy.

### Ranges
- [ ] Views: `filter`, `transform`, `enumerate`, `zip`, `reverse`, `take`/`drop`, `split`, `chunk`.
- [ ] Range-based algorithm overloads.

### Debug
- [ ] `Stacktrace` symbolization hooks (kernel-provided symbol table, `addr2line` when hosted).
- [ ] `dbgln()` / `warnln()` with compile-time log levels and per-subsystem toggles.
- [ ] `VERIFY` / `TODO()` / `VERIFY_NOT_REACHED()` parity with AK.

## Phase 3: Hosted layer (`Prism::OS`)

Comparable to Serenity's LibCore and LibFileSystem. It must never leak into the freestanding headers.

- [ ] Platform backend for Linux (raw syscalls first, so libc stays optional) and later POSIX/libc.
- [ ] `File`, `Directory`, `DirIterator`, `MappedFile`, and `FileSystem::` helpers (copy, remove, stat).
- [ ] `Stream` implementations: `File` stream, buffered stream, stdin/stdout/stderr.
- [ ] `Process` (spawn, wait, environment), `Environment`, `ArgsParser` (an upgrade of `OptionParser`).
- [ ] `Thread`, `Mutex`, `ConditionVariable`, `Semaphore`, and thread-local storage.
- [ ] `Time`: `MonotonicTime`, `UnixDateTime`, `Duration`, plus date/time formatting.
- [ ] `EventLoop` with timers and fd notifiers (epoll on Linux).
- [ ] Sockets: TCP/UDP/Unix, building on `Network/` (`IPv4Address` → `IPv6Address`, `SocketAddress`, `MACAddress`).
- [ ] `ConfigFile` (INI) and a JSON parser/serializer (`JsonValue`, `JsonObject`, `JsonArray`).

## Phase 4: Kernel-oriented extras

Things CryptixOS needs that a generic library usually skips:

- [ ] `PhysicalAddress` / `VirtualAddress` / `PageRange` distinct types.
- [ ] `IntrusiveRedBlackTree`-based VM region trees and an `IntervalTree` for MMIO ranges.
- [ ] Per-CPU storage helpers and an IRQ-safe lock variant.
- [ ] `ELF`, `DeviceTree`, and `ACPI` table parsers, possibly as a separate `Prism::Formats` target.
- [ ] Compression: DEFLATE/zlib inflate, and LZ4 for initrd/modules.
- [ ] Freestanding UBSan runtime completion (all handlers) and a KASAN-style shadow memory helper.

## Phase 5: Packaging, docs, and ecosystem

- [ ] C++20 modules (`Prism.cppm`) kept in sync with the headers, and tested in CI.
- [ ] A CMake package config (`find_package(Prism)`) with exported targets: `Prism::Prism`, `Prism::OS`, `Prism::Test`.
- [ ] Meson and pkg-config support (the AUR package already exists; add Nix and vcpkg/Conan).
- [ ] API documentation (Doxygen or mdBook) with a "freestanding integration guide" page: which hooks to implement and the minimal linker setup.
- [ ] Example projects: a minimal Limine kernel, a UEFI app, and a hosted CLI tool.
- [ ] Versioning: semver releases, a changelog, and an ABI/API stability policy for `Core/`.

---

## Quick wins (good next tasks)

1. Pick one intrusive list and delete the rest.
2. Write `Platform.hpp`, the hook contract, and a hosted implementation of it.
3. Add `ScopeGuard`, `Badge`, `Checked<T>`, `DistinctNumeric`.
4. Write tests for `Vector`, `String`, and `Format`, and run them in CI with ASan.
5. Add `Endian<T>` wrapper types and `GenericLexer`. Both pay off immediately in kernel parsers.
6. Add `HashMap` with open addressing and benchmark it against `UnorderedMap`.
