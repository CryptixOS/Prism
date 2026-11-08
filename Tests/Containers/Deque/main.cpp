/*
 * Created by v1tr10l7 on 30.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Containers/Deque.hpp>

using namespace Prism;

#include <string>
#include <utility>

TEST_CASE(TestBasicOperations){
    Deque<int> dq;
    EXPECT(dq.Empty());

    dq.PushBack(1);
    dq.PushBack(2);
    dq.PushFront(0);

    EXPECT_EQ(dq.Size(), 3);
    EXPECT_EQ(dq[0], 0);
    EXPECT_EQ(dq[1], 1);
    EXPECT_EQ(dq[2], 2);

    EXPECT_EQ(dq.Front(), 0);
    EXPECT_EQ(dq.Back(), 2);

    dq.PopFront();
    dq.PopBack();
    EXPECT_EQ(dq.Size(), 1);
    EXPECT_EQ(dq[0], 1);
}

TEST_CASE(TestPopElement){
    Deque<std::string> dq;
    dq.PushBack("a");
    dq.PushBack("b");
    dq.PushBack("c");

    EXPECT_EQ(dq.PopFrontElement(), "a");
    EXPECT_EQ(dq.PopBackElement(), "c");
    EXPECT_EQ(dq.Size(), 1);
    EXPECT_EQ(dq[0], "b");
}

TEST_CASE(TestIndexing){
    Deque<int> dq;
    for (int i = 0; i < 100; ++i) dq.PushBack(i);
    for (int i = 0; i < 100; ++i) EXPECT_EQ(dq[i], i);
}

TEST_CASE(TestIterators){
    Deque<int> dq;
    for (int i = 0; i < 10; ++i) dq.PushBack(i);

    int val = 0;
    for (auto it = dq.begin(); it != dq.end(); ++it, ++val) EXPECT_EQ((*it), val);

    val = 9;
    for (auto it = dq.rbegin(); it != dq.rend(); ++it, --val)
        EXPECT_EQ((*it), val);

    auto it = dq.begin();
    it += 5;
    EXPECT_EQ((*it), 5);
    EXPECT_EQ(it[2], 7);
    EXPECT_EQ((it - dq.begin()), 5);
}

TEST_CASE(TestClear){
    Deque<int> dq;
    for (int i = 0; i < 50; ++i) dq.PushBack(i);
    dq.Clear();
    EXPECT(dq.Empty());
    dq.PushBack(1);
    EXPECT_EQ(dq.Size(), 1);
    EXPECT_EQ(dq[0], 1);
}

TEST_CASE(TestMove){
    Deque<int> dq1;
    dq1.PushBack(42);
    dq1.PushBack(84);

    Deque<int> dq2 = std::move(dq1);
    EXPECT_EQ(dq2.Size(), 2);
    EXPECT_EQ(dq2[0], 42);
    EXPECT_EQ(dq2[1], 84);

    dq1 = std::move(dq2);
    EXPECT_EQ(dq1.Size(), 2);
    EXPECT_EQ(dq1[0], 42);
    EXPECT_EQ(dq1[1], 84);
}

TEST_CASE(TestStressPushPop){
    Deque<int> dq;
    for (int i = 0; i < 10000; ++i) dq.PushBack(i);
    for (int i = 0; i < 10000; ++i) EXPECT_EQ(dq[i], i);

    for (int i = 0; i < 10000; ++i) dq.PopFront();
    EXPECT(dq.Empty());
}

TEST_CASE(TestInterleavedPushPop){
    Deque<int> dq;
    for (int i = 0; i < 1000; ++i)
    {
        dq.PushFront(i);
        dq.PushBack(i);
    }
    EXPECT_EQ(dq.Size(), 2000);

    for (int i = 0; i < 1000; ++i)
    {
        dq.PopFront();
        dq.PopBack();
    }
    EXPECT(dq.Empty());
}
TEST_CASE(TestErase){
    Deque<int> dq;
    for (int i = 0; i < 5; ++i) dq.PushBack(i); // [0, 1, 2, 3, 4]

    auto it = dq.Erase(dq.begin()); // erase front
    EXPECT_EQ(dq.Size(), 4);
    EXPECT_EQ((*it), 1);

    it = dq.Erase(dq.begin() + 1); // erase "2"
    EXPECT_EQ(dq.Size(), 3);
    EXPECT_EQ((*it), 3);

    it = dq.Erase(dq.end() - 1); // erase "4"
    EXPECT_EQ(dq.Size(), 2);
    EXPECT_EQ(it, dq.end());

    int expected[] = {1, 3};
    int i          = 0;
    for (int val : dq) EXPECT_EQ(val, (expected[i++]));
}


