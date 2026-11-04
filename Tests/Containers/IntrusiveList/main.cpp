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
#include <Prism/Containers/Vector.hpp>
#include <cassert>
#include <cstdlib>
#include <iostream>

// Unlike assert(), this still fires when NDEBUG is defined (release builds).
#define Check(expr)                                                            \
    do {                                                                       \
        if (!(expr))                                                           \
        {                                                                      \
            std::cerr << __FILE__ << ":" << __LINE__                           \
                      << ": Check failed: " #expr "\n";                        \
            std::abort();                                                      \
        }                                                                      \
    } while (0)

// Hides self-assignment from -Wself-move / -Wself-assign.
template <typename T>
T& Alias(T& value)
{
    return value;
}

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

// Regression: IntrusiveListHook::Unlink() didn't update the owner's size.
void IntrusiveList_TestHookUnlinkUpdatesSize()
{
    Node::List list;
    Node       a(1), b(2), c(3);
    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    b.Hook.Unlink(&b);
    Check(list.Size() == 2);
    a.Hook.Unlink(&a);
    c.Hook.Unlink(&c);
    Check(list.Size() == 0 && list.Empty());
}

// Regression: Erase() decremented the size even when the node wasn't in
// this list, underflowing it.
void IntrusiveList_TestEraseForeignNode()
{
    Node::List first, second;
    Node       a(1), b(2), unlinked(3);
    first.PushBack(&a);
    second.PushBack(&b);

    first.Erase(&b);
    Check(first.Size() == 1 && second.Size() == 1);
    Check(b.Hook.IsLinked() && second.Head() == &b);

    first.Erase(&unlinked);
    Check(first.Size() == 1);
    Check(first.Erase(first.end()) == first.end());
}

// Regression: move constructor/assignment dropped m_Size.
void IntrusiveList_TestMovePreservesSizeAndOwner()
{
    Node       a(1), b(2);
    Node::List source;
    source.PushBack(&a);
    source.PushBack(&b);

    Node::List moved(Move(source));
    Check(moved.Size() == 2 && source.Size() == 0 && source.Empty());
    Check(a.Hook.Owner == &moved && b.Hook.Owner == &moved);

    Node::List assigned;
    assigned = Move(moved);
    Check(assigned.Size() == 2 && moved.Size() == 0);

    a.Hook.Unlink(&a);
    Check(assigned.Size() == 1 && assigned.Head() == &b);

    assigned = Move(Alias(assigned));
    Check(assigned.Size() == 1 && assigned.Head() == &b);
}

// Regression: lists stored in a Vector (as UnorderedMap does) must survive
// reallocation; relocation goes through the move constructor.
void IntrusiveList_TestVectorRelocation()
{
    Vector<Node::List> buckets;
    Node               a(1), b(2);
    buckets.Resize(1);
    buckets[0].PushBack(&a);
    buckets[0].PushBack(&b);
    buckets.Reserve(128);

    Check(buckets[0].Size() == 2);
    Check(a.Hook.Owner == &buckets[0]);
    a.Hook.Unlink(&a);
    Check(buckets[0].Size() == 1 && buckets[0].Head() == &b);
}

// Regression: rbegin()/rend() were wired to the wrong ends and the iterator
// couldn't be decremented, so reverse iteration didn't compile.
void IntrusiveList_TestReverseIteration()
{
    Node::List list;
    Node       a(1), b(2), c(3);
    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    int expected = 3;
    for (auto it = list.rbegin(); it != list.rend(); ++it)
        Check((*it).Value == expected--);
    Check(expected == 0);

    auto it = list.end();
    --it;
    Check(&*it == &c);
}

// Erase(iterator) must return the following node.
void IntrusiveList_TestEraseReturnsNext()
{
    Node::List list;
    Node       a(1), b(2), c(3);
    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    auto it = list.Erase(list.begin());
    Check(it.Current == &b && list.Size() == 2);
    it = list.Erase(Node::List::Iterator(&c, &list));
    Check(it == list.end() && list.Size() == 1);
    Check(list.Head() == &b && list.Tail() == &b);
}

int main()
{
    IntrusiveList_TestPushAndIteration();
    IntrusiveList_TestUnlinkMiddle();
    IntrusiveList_TestUnlinkHead();
    IntrusiveList_TestUnlinkTail();
    IntrusiveList_TestClear();
    IntrusiveList_TestHookUnlinkUpdatesSize();
    IntrusiveList_TestEraseForeignNode();
    IntrusiveList_TestMovePreservesSizeAndOwner();
    IntrusiveList_TestVectorRelocation();
    IntrusiveList_TestReverseIteration();
    IntrusiveList_TestEraseReturnsNext();

    std::cout << "All intrusive list tests passed!\n";
    return 0;
}
