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

static_assert(IsTupleLikeV<Tuple<int, char>>);
static_assert(IsTupleLikeV<const Tuple<int>&>);
static_assert(TupleLikeType<Tuple<>>);

TEST_CASE(tuple_apply_mutates_lvalue)
{
    Tuple<int, long, char> tuple;
    usize                  i = 0;
    Apply(
        [&](auto&&... args)
        {
            (Invoke([&]<typename T>(T& arg) { arg = T(++i * 10); }, args),
             ...);
        },
        tuple);

    EXPECT_EQ(tuple.Get<0>(), 10);
    EXPECT_EQ(tuple.Get<1>(), 20l);
    EXPECT_EQ(tuple.Get<2>(), char(30));
}

static int Sum3(int a, int b, int c) { return a + b + c; }

TEST_CASE(tuple_apply_function_pointer)
{
    Tuple<int, int, int> tuple(1, 2, 3);
    EXPECT_EQ(Apply(Sum3, tuple), 6);
    EXPECT_EQ(Apply(&Sum3, AsConst(tuple)), 6);
    EXPECT_EQ(Apply([](int a, int b, int c) { return a * b * c; },
                    Tuple<int, int, int>(2, 3, 4)),
              24);
}

TEST_CASE(tuple_apply_rvalue_moves)
{
    Tuple<String> tuple("Hello"_s);
    String        moved = Apply([](String&& s) { return Move(s); }, Move(tuple));
    EXPECT_EQ(moved, "Hello"_s);
}

TEST_CASE(tuple_converting_constructor)
{
    Tuple<int, const char*> source(7, "str");
    Tuple<long, StringView> converted(source);
    EXPECT_EQ(converted.Get<0>(), 7l);
    EXPECT_EQ(converted.Get<1>(), "str"_sv);
}

TEST_CASE(tuple_swap)
{
    Tuple<int, String> a(1, "a"_s);
    Tuple<int, String> b(2, "b"_s);
    Swap(a, b);
    EXPECT_EQ(a.Get<0>(), 2);
    EXPECT_EQ(b.Get<1>(), "a"_s);
}
