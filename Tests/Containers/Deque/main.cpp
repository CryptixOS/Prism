/*
 * Created by v1tr10l7 on 30.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Deque.hpp>

using namespace Prism;

#include <cassert>
#include <string>
#include <utility>

void TestBasicOperations()
{
    Deque<int> dq;
    assert(dq.Empty());

    dq.PushBack(1);
    dq.PushBack(2);
    dq.PushFront(0);

    assert(dq.Size() == 3);
    assert(dq[0] == 0);
    assert(dq[1] == 1);
    assert(dq[2] == 2);

    assert(dq.Front() == 0);
    assert(dq.Back() == 2);

    dq.PopFront();
    dq.PopBack();
    assert(dq.Size() == 1);
    assert(dq[0] == 1);
}

void TestPopElement()
{
    Deque<std::string> dq;
    dq.PushBack("a");
    dq.PushBack("b");
    dq.PushBack("c");

    assert(dq.PopFrontElement() == "a");
    assert(dq.PopBackElement() == "c");
    assert(dq.Size() == 1);
    assert(dq[0] == "b");
}

void TestIndexing()
{
    Deque<int> dq;
    for (int i = 0; i < 100; ++i) dq.PushBack(i);
    for (int i = 0; i < 100; ++i) assert(dq[i] == i);
}

void TestIterators()
{
    Deque<int> dq;
    for (int i = 0; i < 10; ++i) dq.PushBack(i);

    int val = 0;
    for (auto it = dq.begin(); it != dq.end(); ++it, ++val) assert(*it == val);

    val = 9;
    for (auto it = dq.rbegin(); it != dq.rend(); ++it, --val)
        assert(*it == val);

    auto it = dq.begin();
    it += 5;
    assert(*it == 5);
    assert(it[2] == 7);
    assert((it - dq.begin()) == 5);
}

void TestClear()
{
    Deque<int> dq;
    for (int i = 0; i < 50; ++i) dq.PushBack(i);
    dq.Clear();
    assert(dq.Empty());
    dq.PushBack(1);
    assert(dq.Size() == 1);
    assert(dq[0] == 1);
}

void TestMove()
{
    Deque<int> dq1;
    dq1.PushBack(42);
    dq1.PushBack(84);

    Deque<int> dq2 = std::move(dq1);
    assert(dq2.Size() == 2);
    assert(dq2[0] == 42);
    assert(dq2[1] == 84);

    dq1 = std::move(dq2);
    assert(dq1.Size() == 2);
    assert(dq1[0] == 42);
    assert(dq1[1] == 84);
}

void TestStressPushPop()
{
    Deque<int> dq;
    for (int i = 0; i < 10000; ++i) dq.PushBack(i);
    for (int i = 0; i < 10000; ++i) assert(dq[i] == i);

    for (int i = 0; i < 10000; ++i) dq.PopFront();
    assert(dq.Empty());
}

void TestInterleavedPushPop()
{
    Deque<int> dq;
    for (int i = 0; i < 1000; ++i)
    {
        dq.PushFront(i);
        dq.PushBack(i);
    }
    assert(dq.Size() == 2000);

    for (int i = 0; i < 1000; ++i)
    {
        dq.PopFront();
        dq.PopBack();
    }
    assert(dq.Empty());
}
void TestErase()
{
    Deque<int> dq;
    for (int i = 0; i < 5; ++i) dq.PushBack(i); // [0, 1, 2, 3, 4]

    auto it = dq.Erase(dq.begin()); // erase front
    assert(dq.Size() == 4);
    assert(*it == 1);

    it = dq.Erase(dq.begin() + 1); // erase "2"
    assert(dq.Size() == 3);
    assert(*it == 3);

    it = dq.Erase(dq.end() - 1); // erase "4"
    assert(dq.Size() == 2);
    assert(it == dq.end());

    int expected[] = {1, 3};
    int i          = 0;
    for (int val : dq) assert(val == expected[i++]);
}

int main()
{
    TestBasicOperations();
    TestPopElement();
    TestIndexing();
    TestIterators();
    TestClear();
    TestMove();
    TestStressPushPop();
    TestInterleavedPushPop();
    TestErase();
    return 0;
}
