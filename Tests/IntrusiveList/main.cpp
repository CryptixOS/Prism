/*
 * Created by v1tr10l7 on 01.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/IntrusiveList.hpp>
#include <Prism/String/StringView.hpp>

#include <cstdio>

using namespace Prism;

#include <Prism/Containers/IntrusiveList.hpp>
#include <cassert>
#include <iostream>

using namespace Prism;

struct Node
{
    i32                     Value;
    IntrusiveListHook<Node> Hook;

    using List = IntrusiveList<Node>;

    Node(i32 v)
        : Value(v)
    {
    }
};

void IntrusiveList_TestPushAndIteration()
{
    Node::List list;
    Node       a(1), b(2), c(3);

    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    int expected[] = {1, 2, 3};
    int idx        = 0;

    for (auto node : list)
    {
        assert(node.Value == expected[idx]);
        idx++;
    }

    assert(idx == 3);
    std::cout << "IntrusiveList_TestPushAndIteration passed!\n";
}

void IntrusiveList_TestUnlinkMiddle()
{
    Node::List list;
    Node       a(10), b(20), c(30);

    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    b.Hook.Unlink(&b); // Unlink middle

    assert(list.Head() == &a);
    assert(list.Tail() == &c);
    assert(a.Hook.Next == &c);
    assert(c.Hook.Previous == &a);
    assert(!b.Hook.IsLinked());

    int expected[] = {10, 30};
    int idx        = 0;

    for (auto node : list)
    {
        assert(node.Value == expected[idx]);
        idx++;
    }

    assert(idx == 2);
    std::cout << "IntrusiveList_TestUnlinkMiddle passed!\n";
}

void IntrusiveList_TestUnlinkHead()
{
    Node::List list;
    Node       a(100), b(200), c(300);

    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    a.Hook.Unlink(&a); // Unlink head

    assert(list.Head() == &b);
    assert(list.Tail() == &c);
    assert(b.Hook.Previous == nullptr);
    assert(!a.Hook.IsLinked());

    std::cout << "IntrusiveList_TestUnlinkHead passed!\n";
}

void IntrusiveList_TestUnlinkTail()
{
    Node::List list;
    Node       a(1000), b(2000), c(3000);

    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    c.Hook.Unlink(&c); // Unlink tail

    assert(list.Head() == &a);
    assert(list.Tail() == &b);
    assert(b.Hook.Next == nullptr);
    assert(!c.Hook.IsLinked());

    std::cout << "IntrusiveList_TestUnlinkTail passed!\n";
}

void IntrusiveList_TestClear()
{
    Node::List list;
    Node       a(1), b(2), c(3);

    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    list.Clear();

    assert(list.Empty());
    assert(list.Head() == nullptr);
    assert(list.Tail() == nullptr);
    assert(!a.Hook.IsLinked());
    assert(!b.Hook.IsLinked());
    assert(!c.Hook.IsLinked());

    std::cout << "IntrusiveList_TestClear passed!\n";
}

int main()
{
    IntrusiveList_TestPushAndIteration();
    IntrusiveList_TestUnlinkMiddle();
    IntrusiveList_TestUnlinkHead();
    IntrusiveList_TestUnlinkTail();
    IntrusiveList_TestClear();

    std::cout << "All intrusive list tests passed!\n";
    return 0;
}
