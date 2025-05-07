/*
 * Created by v1tr10l7 on 30.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Deque.hpp>

using namespace Prism;

void test_push_pop()
{
    Deque<int> d;
    d.PushBack(1);
    d.PushBack(2);
    d.PushBack(3);
    d.PushFront(0);
    d.PushFront(-1);

    assert(d.Size() == 5);
    assert(d[0] == -1);
    assert(d[1] == 0);
    assert(d[2] == 1);
    assert(d[3] == 2);
    assert(d[4] == 3);

    d.PopFront();
    assert(d[0] == 0);
    d.PopBack();
    assert(d[d.Size() - 1] == 2);
}

void test_grow()
{
    Deque<int> d;
    for (int i = 0; i < 500; ++i) d.PushBack(i);
    for (int i = 0; i < 500; ++i) assert(d[i] == i);
}

int main()
{
    test_push_pop();
    test_grow();

    return 0;
}
