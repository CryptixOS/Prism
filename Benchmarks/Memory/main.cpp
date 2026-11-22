/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

// Prism memory primitives, smart pointers and allocators vs libc and
// <memory>. See Common/Bench.hpp for the flags.

#include "../Common/Bench.hpp"

#include <cstdlib>
#include <cstring>
#include <memory>
#include <sys/mman.h>
#include <vector>

import Prism.Memory.BumpAllocator;
import Prism.Memory.Memory;
import Prism.Memory.Pointer;
import Prism.Memory.Ref;
import Prism.Memory.Scope;
import Prism.Memory.SlabAllocator;
import Prism.Memory.SlabPool;
import Prism.Memory.WeakRef;

using Bench::Name;
using Bench::Opaque;
using Bench::Runner;
using Bench::u64;

static u64 Digest(const unsigned char* data, size_t size)
{
    u64 h = size;
    // Sparse but position-dependent: cheap next to the copy itself.
    for (size_t i = 0; i < size; i += 61) h = h * 31 + data[i];
    if (size) h = h * 31 + data[size - 1];
    return h;
}

// ============================================================================
// Memory::Copy / Fill / Move / Compare / ScanForCharacter vs libc

static const size_t kByteSizes[] = {8, 64, 256, 4096, 65536, 1 << 20};

static void RegisterPrimitives()
{
    for (size_t n : kByteSizes)
    {
        Bench::Add("Memory", Name("Copy", n), [n](Runner& r) {
            std::vector<unsigned char> src(n), dst(n);
            Bench::Rng                 rng(1);
            for (auto& b : src) b = static_cast<unsigned char>(rng.Next());
            r.Bytes(double(n));
            r.Compare(
                1,
                [&] {
                    Prism::Memory::Copy(dst.data(), Opaque(src.data()), Opaque(n));
                    return Digest(dst.data(), n);
                },
                [&] {
                    std::memcpy(dst.data(), Opaque(src.data()), Opaque(n));
                    return Digest(dst.data(), n);
                });
        });
        Bench::Add("Memory", Name("Fill", n), [n](Runner& r) {
            std::vector<unsigned char> dst(n);
            r.Bytes(double(n));
            r.Compare(
                1,
                [&] {
                    Prism::Memory::Fill(dst.data(), Opaque(0x5a), Opaque(n));
                    return Digest(dst.data(), n);
                },
                [&] {
                    std::memset(dst.data(), Opaque(0x5a), Opaque(n));
                    return Digest(dst.data(), n);
                });
        });
        // Overlapping, forward by 3 bytes.
        Bench::Add("Memory", Name("Move/overlap", n), [n](Runner& r) {
            std::vector<unsigned char> a(n + 3), b(n + 3);
            for (size_t i = 0; i < n + 3; ++i) a[i] = b[i] = static_cast<unsigned char>(i * 7);
            r.Bytes(double(n));
            r.Compare(
                1,
                [&] {
                    Prism::Memory::Move(a.data() + 3, Opaque(a.data()), Opaque(n));
                    Prism::Memory::Move(a.data(), Opaque(a.data() + 3), Opaque(n));
                    return Digest(a.data(), n + 3);
                },
                [&] {
                    std::memmove(b.data() + 3, Opaque(b.data()), Opaque(n));
                    std::memmove(b.data(), Opaque(b.data() + 3), Opaque(n));
                    return Digest(b.data(), n + 3);
                });
        });
        // Equal buffers: a full pass.
        Bench::Add("Memory", Name("Compare/equal", n), [n](Runner& r) {
            std::vector<unsigned char> a(n, 0x33), b(n, 0x33);
            r.Bytes(double(n));
            r.Compare(
                1,
                [&] {
                    return u64(Prism::Memory::Compare(Opaque(a.data()), Opaque(b.data()), n)
                               == 0);
                },
                [&] {
                    return u64(std::memcmp(Opaque(a.data()), Opaque(b.data()), n) == 0);
                });
        });
        // The byte is only at the end: a full pass.
        Bench::Add("Memory", Name("ScanForCharacter", n), [n](Runner& r) {
            std::vector<unsigned char> a(n, 'a');
            a[n - 1] = 'z';
            r.Bytes(double(n));
            r.Compare(
                1,
                [&] {
                    Prism::Pointer p = Prism::Memory::ScanForCharacter(Opaque(a.data()), 'z', n);
                    return u64(p.As<unsigned char>() - a.data());
                },
                [&] {
                    auto* p = static_cast<unsigned char*>(std::memchr(Opaque(a.data()), 'z', n));
                    return u64(p - a.data());
                });
        });
    }
}

// ============================================================================
// Ref / Scope / WeakRef vs shared_ptr / unique_ptr / weak_ptr

struct Object : Prism::RefCounted<Object>, Prism::Weakable<Object>
{
    u64 Value = 7;
};
struct StdObject : std::enable_shared_from_this<StdObject>
{
    u64 Value = 7;
};
struct Plain
{
    u64 Value = 7;
};

static void RegisterSmartPointers()
{
    constexpr size_t n = 1000;
    Bench::Add("Ref", "Create", [](Runner& r) {
        r.Compare(
            n,
            [] {
                u64 sum = 0;
                for (size_t i = 0; i < Opaque(n); ++i)
                    sum += Prism::CreateRef<Object>()->Value;
                return sum;
            },
            [] {
                u64 sum = 0;
                for (size_t i = 0; i < Opaque(n); ++i)
                    sum += std::make_shared<StdObject>()->Value;
                return sum;
            });
    });
    // Copy + destroy: one increment and one decrement each.
    Bench::Add("Ref", "CopyDestroy", [](Runner& r) {
        auto pr = Prism::CreateRef<Object>();
        auto sr = std::make_shared<StdObject>();
        r.Compare(
            n,
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < Opaque(n); ++i)
                {
                    Prism::Ref<Object> copy = Opaque(&pr)[0];
                    sum += copy->Value;
                }
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < Opaque(n); ++i)
                {
                    std::shared_ptr<StdObject> copy = Opaque(&sr)[0];
                    sum += copy->Value;
                }
                return sum;
            });
    });
    Bench::Add("Ref", "Deref", [](Runner& r) {
        std::vector<Prism::Ref<Object>>         pr;
        std::vector<std::shared_ptr<StdObject>> sr;
        for (size_t i = 0; i < n; ++i)
        {
            pr.push_back(Prism::CreateRef<Object>());
            sr.push_back(std::make_shared<StdObject>());
        }
        r.Compare(
            n,
            [&] {
                u64 sum = 0;
                for (auto& p : pr) sum += p->Value;
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (auto& p : sr) sum += p->Value;
                return sum;
            });
    });
    Bench::Add("WeakRef", "Promote", [](Runner& r) {
        auto                     pr = Prism::CreateRef<Object>();
        auto                     sr = std::make_shared<StdObject>();
        Prism::WeakRef<Object>   pw(pr);
        std::weak_ptr<StdObject> sw(sr);
        r.Compare(
            n,
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < Opaque(n); ++i) sum += Opaque(&pw)[0].Promote()->Value;
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < Opaque(n); ++i) sum += Opaque(&sw)[0].lock()->Value;
                return sum;
            });
    });
    Bench::Add("WeakRef", "Create", [](Runner& r) {
        auto pr = Prism::CreateRef<Object>();
        auto sr = std::make_shared<StdObject>();
        r.Compare(
            n,
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < Opaque(n); ++i)
                {
                    Prism::WeakRef<Object> weak(Opaque(&pr)[0]);
                    sum += !weak.IsNull();
                }
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < Opaque(n); ++i)
                {
                    std::weak_ptr<StdObject> weak(Opaque(&sr)[0]);
                    sum += !weak.expired();
                }
                return sum;
            });
    });
    Bench::Add("Scope", "Create", [](Runner& r) {
        r.Compare(
            n,
            [] {
                u64 sum = 0;
                for (size_t i = 0; i < Opaque(n); ++i)
                    sum += Prism::CreateScope<Plain>()->Value;
                return sum;
            },
            [] {
                u64 sum = 0;
                for (size_t i = 0; i < Opaque(n); ++i) sum += std::make_unique<Plain>()->Value;
                return sum;
            });
    });
    Bench::Add("Scope", "MoveChain", [](Runner& r) {
        r.Compare(
            n,
            [] {
                auto s   = Prism::CreateScope<Plain>();
                u64  sum = 0;
                for (size_t i = 0; i < Opaque(n); ++i)
                {
                    Prism::Scope<Plain> next = Prism::Move(s);
                    sum += next->Value;
                    s = Prism::Move(next);
                }
                return sum;
            },
            [] {
                auto s   = std::make_unique<Plain>();
                u64  sum = 0;
                for (size_t i = 0; i < Opaque(n); ++i)
                {
                    std::unique_ptr<Plain> next = std::move(s);
                    sum += next->Value;
                    s = std::move(next);
                }
                return sum;
            });
    });
}

// ============================================================================
// Allocators vs malloc/free

constexpr size_t kPageSize = 0x1000;

struct NoLock
{
    void Init() {}
    struct Guard
    {
        Guard() {}
    };
    Guard Lock() { return {}; }
};

struct MmapPages
{
    static Prism::Pointer AllocatePages(size_t count)
    {
        void* memory = mmap(nullptr, count * kPageSize, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (memory == MAP_FAILED) return nullptr;
        return Prism::Pointer(reinterpret_cast<std::uintptr_t>(memory));
    }
    // Fresh anonymous mappings are already zeroed.
    static Prism::Pointer CallocatePages(size_t count) { return AllocatePages(count); }
    static void           FreePages(Prism::Pointer base, size_t count)
    {
        munmap(reinterpret_cast<void*>(base.Raw()), count * kPageSize);
    }
};

static void RegisterAllocators()
{
    constexpr size_t n = 4096;
    for (size_t size : {32, 256})
    {
        // Allocate n objects then free them all, in allocation order.
        Bench::Add("SlabAllocator", Name("AllocFree", size), [size](Runner& r) {
            auto slab = std::make_unique<Prism::SlabAllocator<MmapPages, NoLock>>();
            if (!slab->Initialize(size)) return;
            std::vector<void*> ptrs(n);
            r.Compare(
                n,
                [&] {
                    for (auto& p : ptrs) p = slab->Allocate().As<void>();
                    u64 nonNull = 0;
                    for (auto p : ptrs) nonNull += p != nullptr;
                    for (auto p : ptrs) slab->Free(p);
                    return nonNull;
                },
                [&] {
                    for (auto& p : ptrs) p = std::malloc(Opaque(size));
                    u64 nonNull = 0;
                    for (auto p : ptrs) nonNull += p != nullptr;
                    for (auto p : ptrs) std::free(p);
                    return nonNull;
                });
        });
    }
    // Mixed sizes through the bucketed pool.
    Bench::Add("SlabPool", "AllocFree/mixed-8..512", [](Runner& r) {
        auto pool = std::make_unique<Prism::SlabPool<7, MmapPages, NoLock>>();
        if (!pool->Initialize()) return;
        std::vector<size_t> sizes(n);
        Bench::Rng          rng(3);
        for (auto& s : sizes) s = 8 + rng.Below(505);
        std::vector<void*> ptrs(n);
        r.Compare(
            n,
            [&] {
                for (size_t i = 0; i < n; ++i) ptrs[i] = pool->Allocate(sizes[i]).As<void>();
                u64 nonNull = 0;
                for (auto p : ptrs) nonNull += p != nullptr;
                for (auto p : ptrs) pool->Free(p);
                return nonNull;
            },
            [&] {
                for (size_t i = 0; i < n; ++i) ptrs[i] = std::malloc(sizes[i]);
                u64 nonNull = 0;
                for (auto p : ptrs) nonNull += p != nullptr;
                for (auto p : ptrs) std::free(p);
                return nonNull;
            });
    });
    // A bump allocator never frees individually: set up, allocate n, tear
    // down. Against malloc n + free n.
    Bench::Add("BumpAllocator", "AllocReset/64", [](Runner& r) {
        r.Compare(
            n,
            [] {
                Prism::BumpAllocator<MmapPages, NoLock> bump;
                if (!bump.Initialize()) return u64(0);
                u64 nonNull = 0;
                for (size_t i = 0; i < n; ++i) nonNull += bool(bump.Allocate(Opaque(64)));
                bump.Shutdown();
                return nonNull;
            },
            [] {
                static void* ptrs[n];
                u64          nonNull = 0;
                for (size_t i = 0; i < n; ++i) nonNull += (ptrs[i] = std::malloc(Opaque(64))) != nullptr;
                for (auto p : ptrs) std::free(p);
                return nonNull;
            });
    });
}

int main(int argc, char** argv)
{
    RegisterPrimitives();
    RegisterSmartPointers();
    RegisterAllocators();
    return Bench::Main(argc, argv, "Memory");
}
