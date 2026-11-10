/*
 * Created by v1tr10l7 on 17.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
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
    EXPECT_EQ((AtomicLoad(val, MemoryOrder::eSeqCst)), T(2));

    T dest{};
    AtomicLoad(val, dest, MemoryOrder::eSeqCst);
    EXPECT_EQ(dest, T(2));

    T src = T(3);
    AtomicStore(val, src, MemoryOrder::eSeqCst);
    EXPECT_EQ(val, T(3));

    // --- Exchange ---
    T old = AtomicExchange(val, T(4), MemoryOrder::eSeqCst);
    EXPECT_EQ(old, T(3));
    EXPECT_EQ(val, T(4));

    // --- CompareExchange strong ---
    if constexpr (!IsSameV<T, bool>)
    {
        // expected mismatch
        T expected   = T(4);
        val          = T(5);
        bool success = AtomicCompareExchange(val, expected, T(6), false,
                                             MemoryOrder::eSeqCst,
                                             MemoryOrder::eRelaxed);
        EXPECT(!success);
        EXPECT_EQ(val, T(5));
        EXPECT_EQ(expected, T(5));
    }
    else
    {
        // For bool, just test true/false transitions
        val           = false;
        T    expected = false;
        bool success  = AtomicCompareExchange(val, expected, true, false,
                                              MemoryOrder::eSeqCst,
                                              MemoryOrder::eRelaxed);
        EXPECT(success);
        EXPECT_EQ(val, true);

        expected = false;
        success  = AtomicCompareExchange(val, expected, false, false,
                                         MemoryOrder::eSeqCst,
                                         MemoryOrder::eRelaxed);
        EXPECT(!success); // because expected != val
        EXPECT_EQ(val, true);
        EXPECT_EQ(expected, true);
    }
    // --- Arithmetic fetches (only for integral-like) ---
    if constexpr (!IsSameV<T, bool>)
    {
        val = T(1);
        AtomicAddFetch(val, T(2), MemoryOrder::eSeqCst);
        EXPECT(static_cast<T>(val) == static_cast<T>(3));

        AtomicSubFetch(val, T(1), MemoryOrder::eSeqCst);
        EXPECT(static_cast<T>(val) == static_cast<T>(2));
    }

    // --- Bitwise fetches (only for integral-like) ---
    if constexpr (!IsSameV<T, bool>)
    {
        val = T(1);
        AtomicOrFetch(val, T(4), MemoryOrder::eSeqCst);
        EXPECT(static_cast<T>(val) == static_cast<T>(1 | 4));

        AtomicAndFetch(val, T(1), MemoryOrder::eSeqCst);
        EXPECT(static_cast<T>(val) == static_cast<T>((1 | 4) & 1));

        val = T(1);
        AtomicXorFetch(val, T(2), MemoryOrder::eSeqCst);
        EXPECT(static_cast<T>(val) == static_cast<T>(1 ^ 2));

        val = T(1);
        AtomicNandFetch(val, T(2), MemoryOrder::eSeqCst);
        EXPECT(static_cast<T>(val) == static_cast<T>(~(1 & 2)));
    }

    // --- Fetch ops returning old value ---
    if constexpr (!IsSameV<T, bool>)
    {
        val      = T(1);
        T oldVal = AtomicFetchAdd(val, T(2), MemoryOrder::eSeqCst);
        EXPECT_EQ(oldVal, T(1));
        EXPECT(static_cast<T>(val) == static_cast<T>(3));

        oldVal = AtomicFetchSub(val, T(1), MemoryOrder::eSeqCst);
        EXPECT(static_cast<T>(oldVal) == static_cast<T>(3));
        EXPECT(static_cast<T>(val) == static_cast<T>(2));

        oldVal = AtomicFetchOr(val, T(4), MemoryOrder::eSeqCst);
        EXPECT(static_cast<T>(oldVal) == static_cast<T>(2));
        EXPECT(static_cast<T>(val) == static_cast<T>(2 | 4));

        oldVal = AtomicFetchAnd(val, T(2), MemoryOrder::eSeqCst);
        EXPECT(static_cast<T>(oldVal) == static_cast<T>(2 | 4));
        EXPECT(static_cast<T>(val) == static_cast<T>((2 | 4) & 2));
    }

    // --- TestAndSet / Clear ---
    {
        bool flag  = 0;
        bool first = AtomicTestAndSet(flag, MemoryOrder::eSeqCst);
        EXPECT_EQ(first, false);
        bool second = AtomicTestAndSet(flag, MemoryOrder::eSeqCst);
        EXPECT_EQ(second, true);

        bool bflag = true;
        AtomicClear(bflag, MemoryOrder::eSeqCst);
        EXPECT_EQ(bflag, false);
    }

    // --- Fences (no observable effect here) ---
    AtomicThreadFence(MemoryOrder::eSeqCst);
    AtomicSignalFence(MemoryOrder::eAcquire);

    // --- LockFree check ---
    EXPECT(AtomicAlwaysLockFree(sizeof(T), nullptr));
}

// --- Runner ---

TEST_CASE(TestAtomicType_i8)
{
    TestAtomicType<i8>();
}

TEST_CASE(TestAtomicType_i16)
{
    TestAtomicType<i16>();
}

TEST_CASE(TestAtomicType_i32)
{
    TestAtomicType<i32>();
}

TEST_CASE(TestAtomicType_i64)
{
    TestAtomicType<i64>();
}

TEST_CASE(TestAtomicType_u8)
{
    TestAtomicType<u8>();
}

TEST_CASE(TestAtomicType_u16)
{
    TestAtomicType<u16>();
}

TEST_CASE(TestAtomicType_u32)
{
    TestAtomicType<u32>();
}

TEST_CASE(TestAtomicType_u64)
{
    TestAtomicType<u64>();
}

TEST_CASE(TestAtomicType_bool)
{
    TestAtomicType<bool>();
}

TEST_CASE(TestAtomicType_Color)
{
    TestAtomicType<Color>();
}

