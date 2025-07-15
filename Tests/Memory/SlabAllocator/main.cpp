/*
 * Created by v1tr10l7 on 15.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Core/Error.hpp>
#include <Prism/Memory/SlabPool.hpp>

#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>

using namespace Prism;
struct DummyLock
{
    void Init() {}
    struct Guard
    {
        Guard() {}
    };
    Guard Lock() { return {}; }
};

struct DummyPageAlloc
{
    static inline usize AllocIndex = 0;

    static Pointer      CallocatePages(usize count)
    {
        void* memory = mmap(nullptr, count * PAGE_SIZE, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (memory == MAP_FAILED) return nullptr;

        memset(memory, 0, count * PAGE_SIZE);
        return Pointer(reinterpret_cast<uintptr_t>(memory));
    }

    static void FreePages(Pointer base, usize count)
    {
        int result
            = munmap(reinterpret_cast<void*>(base.Raw()), count * PAGE_SIZE);
        assert(result == 0);
    }
};

void Test_SlabAllocatorBasic()
{
    SlabAllocator<DummyPageAlloc, DummyLock> allocator;
    assert(allocator.Initialize(32));

    Pointer a = allocator.Allocate();
    Pointer b = allocator.Allocate();
    Pointer c = allocator.Allocate();

    assert(a && b && c);
    assert(a != b && b != c && a != c);

    assert(allocator.Used() == 96);

    allocator.Free(a);
    allocator.Free(b);
    allocator.Free(c);

    assert(allocator.Used() == 0);
}

void Test_SlabAllocatorReallocateAfterFree()
{
    SlabAllocator<DummyPageAlloc, DummyLock> allocator;
    assert(allocator.Initialize(64));

    Pointer a = allocator.Allocate();
    allocator.Free(a);
    Pointer b = allocator.Allocate();

    assert(b == a); // Free list reused the same block
}

void Test_SlabPoolBucketAllocation()
{
    constexpr usize                                  BucketCount = 6;
    SlabPool<BucketCount, DummyPageAlloc, DummyLock> pool;
    assert(pool.Initialize());

    Pointer p8   = pool.Allocate(8);
    Pointer p16  = pool.Allocate(16);
    Pointer p32  = pool.Allocate(32);
    Pointer p64  = pool.Allocate(64);
    Pointer p128 = pool.Allocate(128);
    Pointer p256 = pool.Allocate(256);

    assert(p8 && p16 && p32 && p64 && p128 && p256);
    assert(pool.Used() > 0);

    pool.Free(p8);
    pool.Free(p16);
    pool.Free(p32);
    pool.Free(p64);
    pool.Free(p128);
    pool.Free(p256);

    assert(pool.Used() == 0);
}

void Test_SlabPoolLargeAlloc()
{
    constexpr usize                                  BucketCount = 5;
    SlabPool<BucketCount, DummyPageAlloc, DummyLock> pool;
    assert(pool.Initialize());

    Pointer large = pool.Allocate(8192);
    assert(large);

    assert((large.Raw() & 0xFFF) == 0); // page-aligned
    pool.Free(large);

    assert(pool.Used() == 0);
}

void Test_SlabPoolZeroInitialized()
{
    constexpr usize                                  BucketCount = 5;
    SlabPool<BucketCount, DummyPageAlloc, DummyLock> pool;
    assert(pool.Initialize());

    Pointer p = pool.Callocate(16);
    assert(p);

    u8* data = reinterpret_cast<u8*>(p.Raw());
    for (usize i = 0; i < 16; i++)
        assert(data[i] == 0); // Already cleared in dummy policy

    pool.Free(p);
    assert(pool.Used() == 0);
}

void RunSlabAllocatorTests()
{
    DummyPageAlloc::AllocIndex = 0;
    printf("running Test_SlabAllocatorBasic()...\n");
    Test_SlabAllocatorBasic();
    printf("running Test_SlabAllocatorReallocateAfterFree()...\n");
    Test_SlabAllocatorReallocateAfterFree();
    printf("running Test_SlabPoolBucketAllocation()...\n");
    Test_SlabPoolBucketAllocation();
    printf("running Test_SlabPoolLargeAlloc()...\n");
    Test_SlabPoolLargeAlloc();
    printf("running Test_SlabPoolZeroInitialized()...\n");
    Test_SlabPoolZeroInitialized();
    printf("All slab allocator tests passed");
}

int main() { RunSlabAllocatorTests(); }
