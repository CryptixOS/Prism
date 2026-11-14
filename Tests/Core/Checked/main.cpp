/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
import Prism.Core.Checked;
import Prism.Test.Test;

using namespace Prism;

TEST_CASE(checked_operations)
{
    u8 out = 0;
    EXPECT(CheckedAdd<u8>(200, 55, out) && out == 255);
    EXPECT(!CheckedAdd<u8>(200, 56, out));
    EXPECT(CheckedSub<u8>(5, 5, out) && out == 0);
    EXPECT(!CheckedSub<u8>(5, 6, out));
    EXPECT(CheckedMul<u8>(15, 17, out) && out == 255);
    EXPECT(!CheckedMul<u8>(16, 16, out));

    i32 s = 0;
    EXPECT(!CheckedAdd<i32>(NumericLimits<i32>::Max(), 1, s));
    EXPECT(!CheckedSub<i32>(NumericLimits<i32>::Min(), 1, s));
    EXPECT(!CheckedMul<i32>(NumericLimits<i32>::Min(), -1, s));
}

TEST_CASE(saturating_operations)
{
    EXPECT_EQ(SaturatingAdd<u8>(250, 10), 255);
    EXPECT_EQ(SaturatingSub<u8>(3, 10), 0);
    EXPECT_EQ(SaturatingMul<u8>(100, 3), 255);
    EXPECT_EQ(SaturatingAdd<i8>(120, 10), 127);
    EXPECT_EQ(SaturatingAdd<i8>(-120, -10), -128);
    EXPECT_EQ(SaturatingSub<i8>(-120, 10), -128);
    EXPECT_EQ(SaturatingSub<i8>(120, -10), 127);
    EXPECT_EQ(SaturatingMul<i8>(-100, 2), -128);
    EXPECT_EQ(SaturatingMul<i8>(-100, -2), 127);
    EXPECT_EQ(SaturatingAdd<i32>(1, 2), 3);
}

TEST_CASE(checked_type)
{
    Checked<usize> size = 10;
    size *= 4;
    size += 2;
    EXPECT(!size.HasOverflow());
    EXPECT_EQ(size.Value(), 42u);

    Checked<usize> huge = NumericLimits<usize>::Max() / 2 + 1;
    huge *= 2;
    EXPECT(huge.HasOverflow());
    // Once overflowed, it stays overflowed.
    huge -= NumericLimits<usize>::Max();
    EXPECT(huge.HasOverflow());
    EXPECT_EQ(huge.ValueOr(7), 7u);

    Checked<i32> div = NumericLimits<i32>::Min();
    div /= -1;
    EXPECT(div.HasOverflow());
    Checked<i32> zero = 5;
    zero /= 0;
    EXPECT(zero.HasOverflow());

    EXPECT((Checked<u8>(200) + Checked<u8>(100)).HasOverflow());
    EXPECT((Checked<u8>(2) * 3 - 1) == Checked<u8>(5));

    static_assert(Checked<u8>(255).ValueOr(0) == 255);
    static_assert((Checked<u8>(255) + 1).HasOverflow());
}

TEST_CASE(allocation_sizes)
{
    EXPECT_EQ(AllocationSize<u64>(10), 80u);
    EXPECT_EQ(AllocationSize<u8>(0), 0u);
    EXPECT_EQ(GrowCapacity(0, 1), 1u);
    EXPECT_EQ(GrowCapacity(8, 9), 16u);
    EXPECT_EQ(GrowCapacity(8, 100), 100u);
    EXPECT_EQ(GrowCapacity(0, 0, 8), 8u);
    // Doubling saturates instead of wrapping around to something small.
    EXPECT_EQ(GrowCapacity(NumericLimits<usize>::Max() / 2 + 1, 1),
              NumericLimits<usize>::Max());

    EXPECT_CHECK_FAILURE("AllocationSize overflows", [] {
        (void)AllocationSize<u64>(NumericLimits<usize>::Max() / 4);
    });
}
