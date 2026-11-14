/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
import Prism.Test.Test;
import Prism.Containers.Bitmap;

using namespace Prism;

TEST_CASE(Bitmap_AllocateStartsClear){
    Bitmap bitmap;
    bitmap.Allocate(1000);
    EXPECT_EQ(bitmap.BitCount(), 1000);
    for (usize i = 0; i < 1000; ++i) EXPECT(!bitmap.At(i));
    bitmap.Free();
}

TEST_CASE(Bitmap_SetIndex){
    Bitmap bitmap;
    bitmap.Allocate(100);
    bitmap.SetIndex(0, true);
    bitmap.SetIndex(63, true);
    bitmap.SetIndex(99, true);
    EXPECT(bitmap.At(0) && bitmap.At(63) && bitmap.At(99));
    EXPECT(!bitmap.At(1) && !bitmap.At(62) && !bitmap.At(98));
    bitmap.SetIndex(63, false);
    EXPECT(!bitmap.At(63));
    bitmap.Free();
}

// Without an explicit end the search covers every bit.
TEST_CASE(Bitmap_FindFirstDefaultRange){
    Bitmap bitmap;
    bitmap.Allocate(200);
    EXPECT_EQ(bitmap.FindFirstSet(), -1);
    EXPECT_EQ(bitmap.FindFirstNotSet(), 0);

    bitmap.SetIndex(150, true);
    EXPECT_EQ(bitmap.FindFirstSet(), 150);
    EXPECT_EQ(bitmap.FindFirstSet(151), -1);
    EXPECT_EQ(bitmap.FindFirstSet(0, 150), -1);

    for (usize i = 0; i < 200; ++i) bitmap.SetIndex(i, true);
    EXPECT_EQ(bitmap.FindFirstNotSet(), -1);
    bitmap.SetIndex(199, false);
    EXPECT_EQ(bitmap.FindFirstNotSet(), 199);
    bitmap.Free();
}
