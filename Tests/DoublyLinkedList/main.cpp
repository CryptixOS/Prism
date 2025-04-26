/*
 * Created by v1tr10l7 on 17.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/DoublyLinkedList.hpp>

#include <cassert>
#include <cstdio>

using namespace Prism;

void DoublyLinkedList_TestPushPop()
{
    DoublyLinkedList<int> list;
    assert(list.Empty());
    assert(list.Size() == 0);

    list.PushFront(1);
    list.PushFront(2);
    list.PushBack(3);
    // list: 2 <-> 1 <-> 3

    assert(!list.Empty());
    assert(list.Size() == 3);
    assert(list.PopFrontElement() == 2);
    assert(list.PopBackElement() == 3);
    assert(list.PopFrontElement() == 1);
    assert(list.Empty());
}

int main()
{
    DoublyLinkedList_TestPushPop();

    return EXIT_SUCCESS;
};
