/*
 * Created by v1tr10l7 on 26.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Containers/Tuple.hpp>
#include <Prism/String/String.hpp>
#include <iostream>

using namespace Prism;
using namespace Prism::Literals;

TEST_CASE(tuple_get)
{
    Tuple tuple{10, 2.50, "Hello"_s};

    EXPECT_EQ(tuple.Get<0>(), 10);
    EXPECT_APPROXIMATE(tuple.Get<1>(), 2.50);
    EXPECT_EQ(tuple.Get<2>(), "Hello"_s);
}
