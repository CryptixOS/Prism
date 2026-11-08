/*
 * Created by v1tr10l7 on 01.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Test/Test.hpp>
#include <Prism/String/StringView.hpp>

#include <cstdio>

using namespace Prism;

TEST_CASE(string_view_basics)
{
    StringView view = "HelloWorld";
    EXPECT_EQ(view.Size(), 10u);
    EXPECT_EQ(view, "HelloWorld");
    EXPECT_EQ(view.Substr(5), "World");
}
