/*
 * Created by v1tr10l7 on 08.01.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/BitSpan.hpp>

using namespace Prism;

static void TestSingleBitOps()
{
    BitSpan<128> bits;

    for (usize i = 0; i < 128; ++i) assert(bits.Test(i) == false);

    bits.Set(42);
    assert(bits.Test(42) == true);

    bits.Clear(42);
    assert(bits.Test(42) == false);
}

static void TestRangeOps()
{
    BitSpan<128> bits;

    assert(bits.TestRange(10, 20) == true);

    bits.SetRange(10, 20);
    assert(bits.TestRange(10, 20) == false);

    for (usize i = 10; i < 30; ++i) assert(bits.Test(i) == true);

    bits.ClearRange(10, 20);
    assert(bits.TestRange(10, 20) == true);
}

static void TestFindFreeRange()
{
    BitSpan<64> bits;

    bits.SetRange(0, 16);
    bits.SetRange(32, 8);

    isize r1 = bits.FindFreeRange(8);
    assert(r1 == 16);

    bits.SetRange(16, 8);

    isize r2 = bits.FindFreeRange(8);
    assert(r2 == 24);

    bits.SetRange(24, 8);

    isize r3 = bits.FindFreeRange(8);
    assert(r3 == 40);
}

static void TestExhaustion()
{
    BitSpan<32> bits;

    bits.SetRange(0, 32);
    assert(bits.FindFreeRange(1) == -1);
}

int main()
{
    TestSingleBitOps();
    TestRangeOps();
    TestFindFreeRange();
    TestExhaustion();

    return EXIT_SUCCESS;
}
