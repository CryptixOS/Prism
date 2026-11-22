/*
 * Created by v1tr10l7 on 08.01.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
import Prism.Test.Test;
import Prism.Containers.BitSpan;

using namespace Prism;

TEST_CASE(TestSingleBitOps){
    BitSpan<128> bits;

    for (usize i = 0; i < 128; ++i) EXPECT_EQ(bits.Test(i), false);

    bits.Set(42);
    EXPECT_EQ(bits.Test(42), true);

    bits.Clear(42);
    EXPECT_EQ(bits.Test(42), false);
}

TEST_CASE(TestRangeOps){
    BitSpan<128> bits;

    EXPECT_EQ((bits.TestRange(10, 20)), true);

    bits.SetRange(10, 20);
    EXPECT_EQ((bits.TestRange(10, 20)), false);

    for (usize i = 10; i < 30; ++i) EXPECT_EQ(bits.Test(i), true);

    bits.ClearRange(10, 20);
    EXPECT_EQ((bits.TestRange(10, 20)), true);
}

TEST_CASE(TestFindFreeRange){
    BitSpan<64> bits;

    bits.SetRange(0, 16);
    bits.SetRange(32, 8);

    isize r1 = bits.FindFreeRange(8);
    EXPECT_EQ(r1, 16);

    bits.SetRange(16, 8);

    isize r2 = bits.FindFreeRange(8);
    EXPECT_EQ(r2, 24);

    bits.SetRange(24, 8);

    isize r3 = bits.FindFreeRange(8);
    EXPECT_EQ(r3, 40);
}

TEST_CASE(TestExhaustion){
    BitSpan<32> bits;

    bits.SetRange(0, 32);
    EXPECT_EQ(bits.FindFreeRange(1), -1);
}


