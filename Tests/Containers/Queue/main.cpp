/*
 * Created by v1tr10l7 on 13.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Containers/Queue.hpp>
using namespace Prism;

using Q = Queue<int>;

TEST_CASE(default_construction)
{
    Q q;
    EXPECT(q.Empty());
    EXPECT_EQ(q.Size(), 0);
}

TEST_CASE(construction_from_container)
{
    Deque<int> d;
    d.PushBack(1);
    d.PushBack(2);
    Q q(d);
    EXPECT(!q.Empty());
    EXPECT_EQ(q.Size(), 2);
    EXPECT_EQ(q.Front(), 1);
    EXPECT_EQ(q.Back(), 2);
}

TEST_CASE(move_construction_from_container)
{
    Deque<int> d;
    d.PushBack(42);
    Q q(std::move(d));
    EXPECT(!q.Empty());
    EXPECT_EQ(q.Front(), 42);
}

TEST_CASE(range_construction)
{
    int arr[] = {1, 2, 3, 4};
    Q   q(arr, arr + 4);
    EXPECT_EQ(q.Size(), 4);
    EXPECT_EQ(q.Front(), 1);
    EXPECT_EQ(q.Back(), 4);
}

TEST_CASE(push_pop)
{
    Q q;
    q.Push(10);
    q.Push(20);
    q.Push(30);
    EXPECT_EQ(q.Size(), 3);
    EXPECT_EQ(q.Front(), 10);
    EXPECT_EQ(q.Back(), 30);

    q.Pop();
    EXPECT_EQ(q.Size(), 2);
    EXPECT_EQ(q.Front(), 20);
}

TEST_CASE(push_rvalue)
{
    Q   q;
    int x = 99;
    q.Push(std::move(x));
    EXPECT_EQ(q.Size(), 1);
    EXPECT_EQ(q.Front(), 99);
}

TEST_CASE(emplace)
{
    Queue<std::pair<int, int>> q;
    q.Emplace(1, 2);
    EXPECT_EQ(q.Size(), 1);
    EXPECT_EQ(q.Front().first, 1);
    EXPECT_EQ(q.Front().second, 2);
}

TEST_CASE(popelement)
{
    Q q;
    q.Push(5);
    q.Push(6);
    int val = q.PopElement();
    EXPECT_EQ(val, 5);
    EXPECT_EQ(q.Size(), 1);
    EXPECT_EQ(q.Front(), 6);
}

TEST_CASE(copy_assignment)
{
    Q q1;
    q1.Push(1);
    q1.Push(2);
    Q q2;
    q2 = q1;
    EXPECT_EQ(q2.Size(), 2);
    EXPECT_EQ(q2.Front(), 1);
    EXPECT_EQ(q2.Back(), 2);
}

TEST_CASE(move_assignment)
{
    Q q1;
    q1.Push(42);
    Q q2;
    q2 = std::move(q1);
    EXPECT_EQ(q2.Size(), 1);
    EXPECT_EQ(q2.Front(), 42);
}

TEST_CASE(swap_member)
{
    Q q1;
    q1.Push(1);
    Q q2;
    q2.Push(2);
    q1.Swap(q2);
    EXPECT_EQ(q1.Front(), 2);
    EXPECT_EQ(q2.Front(), 1);
}

TEST_CASE(swap_free_function)
{
    Q q1;
    q1.Push(10);
    Q q2;
    q2.Push(20);
    Swap(q1, q2);
    EXPECT_EQ(q1.Front(), 20);
    EXPECT_EQ(q2.Front(), 10);
}

