/*
 * Created by v1tr10l7 on 17.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Utility/AtomicBuiltins.hpp>

using namespace Prism;

enum class Color : u32
{
    eRed   = Bit(0),
    eGreen = Bit(1),
    eBlue  = Bit(2)
};

// --- Generic test function ---
template <typename T>
void TestAtomicType()
{
    // --- AtomicLoad / Store ---
    T val = T(1);
    AtomicStore(val, T(2), MemoryOrder::eSeqCst);
    assert(AtomicLoad(val, MemoryOrder::eSeqCst) == T(2));

    T dest{};
    AtomicLoad(val, dest, MemoryOrder::eSeqCst);
    assert(dest == T(2));

    T src = T(3);
    AtomicStore(val, src, MemoryOrder::eSeqCst);
    assert(val == T(3));

    // --- Exchange ---
    T old = AtomicExchange(val, T(4), MemoryOrder::eSeqCst);
    assert(old == T(3));
    assert(val == T(4));

    // --- CompareExchange strong ---
    if constexpr (!IsSameV<T, bool>)
    {
        // expected mismatch
        T expected   = T(4);
        val          = T(5);
        bool success = AtomicCompareExchange(val, expected, T(6), false,
                                             MemoryOrder::eSeqCst,
                                             MemoryOrder::eRelaxed);
        assert(!success);
        assert(val == T(5));
        assert(expected == T(5));
    }
    else
    {
        // For bool, just test true/false transitions
        val           = false;
        T    expected = false;
        bool success  = AtomicCompareExchange(val, expected, true, false,
                                              MemoryOrder::eSeqCst,
                                              MemoryOrder::eRelaxed);
        assert(success);
        assert(val == true);

        expected = false;
        success  = AtomicCompareExchange(val, expected, false, false,
                                         MemoryOrder::eSeqCst,
                                         MemoryOrder::eRelaxed);
        assert(!success); // because expected != val
        assert(val == true);
        assert(expected == true);
    }
    // --- Arithmetic fetches (only for integral-like) ---
    if constexpr (!IsSameV<T, bool>)
    {
        val = T(1);
        AtomicAddFetch(val, T(2), MemoryOrder::eSeqCst);
        assert(static_cast<T>(val) == static_cast<T>(3));

        AtomicSubFetch(val, T(1), MemoryOrder::eSeqCst);
        assert(static_cast<T>(val) == static_cast<T>(2));
    }

    // --- Bitwise fetches (only for integral-like) ---
    if constexpr (!IsSameV<T, bool>)
    {
        val = T(1);
        AtomicOrFetch(val, T(4), MemoryOrder::eSeqCst);
        assert(static_cast<T>(val) == static_cast<T>(1 | 4));

        AtomicAndFetch(val, T(1), MemoryOrder::eSeqCst);
        assert(static_cast<T>(val) == static_cast<T>((1 | 4) & 1));

        val = T(1);
        AtomicXorFetch(val, T(2), MemoryOrder::eSeqCst);
        assert(static_cast<T>(val) == static_cast<T>(1 ^ 2));

        val = T(1);
        AtomicNandFetch(val, T(2), MemoryOrder::eSeqCst);
        assert(static_cast<T>(val) == static_cast<T>(~(1 & 2)));
    }

    // --- Fetch ops returning old value ---
    if constexpr (!IsSameV<T, bool>)
    {
        val      = T(1);
        T oldVal = AtomicFetchAdd(val, T(2), MemoryOrder::eSeqCst);
        assert(oldVal == T(1));
        assert(static_cast<T>(val) == static_cast<T>(3));

        oldVal = AtomicFetchSub(val, T(1), MemoryOrder::eSeqCst);
        assert(static_cast<T>(oldVal) == static_cast<T>(3));
        assert(static_cast<T>(val) == static_cast<T>(2));

        oldVal = AtomicFetchOr(val, T(4), MemoryOrder::eSeqCst);
        assert(static_cast<T>(oldVal) == static_cast<T>(2));
        assert(static_cast<T>(val) == static_cast<T>(2 | 4));

        oldVal = AtomicFetchAnd(val, T(2), MemoryOrder::eSeqCst);
        assert(static_cast<T>(oldVal) == static_cast<T>(2 | 4));
        assert(static_cast<T>(val) == static_cast<T>((2 | 4) & 2));
    }

    // --- TestAndSet / Clear ---
    {
        bool flag  = 0;
        bool first = AtomicTestAndSet(flag, MemoryOrder::eSeqCst);
        assert(first == false);
        bool second = AtomicTestAndSet(flag, MemoryOrder::eSeqCst);
        assert(second == true);

        bool bflag = true;
        AtomicClear(bflag, MemoryOrder::eSeqCst);
        assert(bflag == false);
    }

    // --- Fences (no observable effect here) ---
    AtomicThreadFence(MemoryOrder::eSeqCst);
    AtomicSignalFence(MemoryOrder::eAcquire);

    // --- LockFree check ---
    assert(AtomicAlwaysLockFree(sizeof(T), nullptr));
}

// --- Runner ---
int main()
{
    TestAtomicType<i8>();
    TestAtomicType<i16>();
    TestAtomicType<i32>();
    TestAtomicType<i64>();
    TestAtomicType<u8>();
    TestAtomicType<u16>();
    TestAtomicType<u32>();
    TestAtomicType<u64>();
    TestAtomicType<bool>();
    TestAtomicType<Color>();

    return 0;
}
