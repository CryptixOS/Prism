/*
 * Created by v1tr10l7 on 01.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>

#include <cstdio>
import Prism.Test.Test;
import Prism.String.StringView;

using namespace Prism;

TEST_CASE(string_view_basics)
{
    StringView view = "HelloWorld";
    EXPECT_EQ(view.Size(), 10u);
    EXPECT_EQ(view, "HelloWorld");
    EXPECT_EQ(view.Substr(5), "World");
}

// The whole view can be removed: the asserts used to reject n == Size()
TEST_CASE(string_view_remove_all)
{
    StringView prefix = "Hello";
    prefix.RemovePrefix(prefix.Size());
    EXPECT(prefix.Empty());

    StringView suffix = "Hello";
    suffix.RemoveSuffix(suffix.Size());
    EXPECT(suffix.Empty());

    StringView part = "Hello";
    part.RemovePrefix(1);
    part.RemoveSuffix(1);
    EXPECT_EQ(part, "ell");
}
