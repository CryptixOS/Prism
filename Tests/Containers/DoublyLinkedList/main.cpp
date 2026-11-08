/*
 * Created by v1tr10l7 on 17.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Containers/DoublyLinkedList.hpp>

#include <cstdio>

using namespace Prism;

TEST_CASE(DoublyLinkedList_TestPushPop){
    DoublyLinkedList<int> list;
    EXPECT(list.Empty());
    EXPECT_EQ(list.Size(), 0);

    list.PushFront(1);
    list.PushFront(2);
    list.PushBack(3);
    // list: 2 <-> 1 <-> 3

    EXPECT(!list.Empty());
    EXPECT_EQ(list.Size(), 3);
    EXPECT_EQ(list.PopFrontElement(), 2);
    EXPECT_EQ(list.PopBackElement(), 3);
    EXPECT_EQ(list.PopFrontElement(), 1);
    EXPECT(list.Empty());
}

;
