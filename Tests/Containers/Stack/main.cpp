/*
 * Created by v1tr10l7 on 13.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Containers/Stack.hpp>
using namespace Prism;

using S = Stack<int>;

TEST_CASE(default_construction)
{
    S s;
    EXPECT(s.Empty());
    EXPECT_EQ(s.Size(), 0);
}

TEST_CASE(construction_from_container)
{
    Deque<int> d;
    d.PushBack(1);
    d.PushBack(2);
    S s(d);
    EXPECT(!s.Empty());
    EXPECT_EQ(s.Size(), 2);
    EXPECT_EQ(s.Front(), 1);
    EXPECT_EQ(s.Back(), 2);
}

TEST_CASE(move_construction_from_container)
{
    Deque<int> d;
    d.PushBack(42);
    S s(std::move(d));
    EXPECT(!s.Empty());
    EXPECT_EQ(s.Front(), 42);
}

TEST_CASE(range_construction)
{
    int arr[] = {1, 2, 3, 4};
    S   s(arr, arr + 4);
    EXPECT_EQ(s.Size(), 4);
    EXPECT_EQ(s.Front(), 1);
    EXPECT_EQ(s.Back(), 4);
}

TEST_CASE(push_pop)
{
    S s;
    s.Push(10);
    s.Push(20);
    s.Push(30);
    EXPECT_EQ(s.Size(), 3);
    EXPECT_EQ(s.Back(), 30);

    s.Pop();
    EXPECT_EQ(s.Size(), 2);
    EXPECT_EQ(s.Back(), 20);
}

TEST_CASE(push_rvalue)
{
    S   s;
    int x = 99;
    s.Push(std::move(x));
    EXPECT_EQ(s.Size(), 1);
    EXPECT_EQ(s.Back(), 99);
}

TEST_CASE(emplace)
{
    Stack<std::pair<int, int>> s;
    s.Emplace(1, 2);
    EXPECT_EQ(s.Size(), 1);
    EXPECT_EQ(s.Back().first, 1);
    EXPECT_EQ(s.Back().second, 2);
}

TEST_CASE(popelement)
{
    S s;
    s.Push(5);
    s.Push(6);
    int val = s.PopElement();
    EXPECT_EQ(val, 6);
    EXPECT_EQ(s.Size(), 1);
    EXPECT_EQ(s.Back(), 5);
}

TEST_CASE(copy_assignment)
{
    S s1;
    s1.Push(1);
    s1.Push(2);
    S s2;
    s2 = s1;
    EXPECT_EQ(s2.Size(), 2);
    EXPECT_EQ(s2.Front(), 1);
    EXPECT_EQ(s2.Back(), 2);
}

TEST_CASE(move_assignment)
{
    S s1;
    s1.Push(42);
    S s2;
    s2 = std::move(s1);
    EXPECT_EQ(s2.Size(), 1);
    EXPECT_EQ(s2.Back(), 42);
}

TEST_CASE(swap_member)
{
    S s1;
    s1.Push(1);
    S s2;
    s2.Push(2);
    s1.Swap(s2);
    EXPECT_EQ(s1.Back(), 2);
    EXPECT_EQ(s2.Back(), 1);
}

TEST_CASE(swap_free_function)
{
    S s1;
    s1.Push(10);
    S s2;
    s2.Push(20);
    Swap(s1, s2);
    EXPECT_EQ(s1.Back(), 20);
    EXPECT_EQ(s2.Back(), 10);
}

