/*
 * Created by v1tr10l7 on 01.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Containers/IntrusiveList.hpp>
#include <Prism/String/StringView.hpp>

#include <cstdio>

using namespace Prism;

#include <Prism/Containers/IntrusiveList.hpp>
#include <Prism/Containers/Vector.hpp>
#include <cstdlib>

// Unlike EXPECT(), this still fires when NDEBUG is defined (release builds).

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

TEST_CASE(IntrusiveList_TestPushAndIteration){
    Node::List list;
    Node       a(1), b(2), c(3);

    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    int expected[] = {1, 2, 3};
    int idx        = 0;

    for (auto node : list)
    {
        EXPECT_EQ(node.Value, expected[idx]);
        idx++;
    }

    EXPECT_EQ(idx, 3);
    Println("IntrusiveList_TestPushAndIteration passed!");
}

TEST_CASE(IntrusiveList_TestUnlinkMiddle){
    Node::List list;
    Node       a(10), b(20), c(30);

    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    b.Hook.Unlink(&b); // Unlink middle

    EXPECT_EQ(list.Head(), (&a));
    EXPECT_EQ(list.Tail(), (&c));
    EXPECT_EQ(a.Hook.Next, (&c));
    EXPECT_EQ(c.Hook.Previous, (&a));
    EXPECT(!b.Hook.IsLinked());

    int expected[] = {10, 30};
    int idx        = 0;

    for (auto node : list)
    {
        EXPECT_EQ(node.Value, expected[idx]);
        idx++;
    }

    EXPECT_EQ(idx, 2);
    Println("IntrusiveList_TestUnlinkMiddle passed!");
}

TEST_CASE(IntrusiveList_TestUnlinkHead){
    Node::List list;
    Node       a(100), b(200), c(300);

    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    a.Hook.Unlink(&a); // Unlink head

    EXPECT_EQ(list.Head(), (&b));
    EXPECT_EQ(list.Tail(), (&c));
    EXPECT_EQ(b.Hook.Previous, nullptr);
    EXPECT(!a.Hook.IsLinked());

    Println("IntrusiveList_TestUnlinkHead passed!");
}

TEST_CASE(IntrusiveList_TestUnlinkTail){
    Node::List list;
    Node       a(1000), b(2000), c(3000);

    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    c.Hook.Unlink(&c); // Unlink tail

    EXPECT_EQ(list.Head(), (&a));
    EXPECT_EQ(list.Tail(), (&b));
    EXPECT_EQ(b.Hook.Next, nullptr);
    EXPECT(!c.Hook.IsLinked());

    Println("IntrusiveList_TestUnlinkTail passed!");
}

TEST_CASE(IntrusiveList_TestClear){
    Node::List list;
    Node       a(1), b(2), c(3);

    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    list.Clear();

    EXPECT(list.Empty());
    EXPECT_EQ(list.Head(), nullptr);
    EXPECT_EQ(list.Tail(), nullptr);
    EXPECT(!a.Hook.IsLinked());
    EXPECT(!b.Hook.IsLinked());
    EXPECT(!c.Hook.IsLinked());

    Println("IntrusiveList_TestClear passed!");
}

// Regression: IntrusiveListHook::Unlink() didn't update the owner's size.
TEST_CASE(IntrusiveList_TestHookUnlinkUpdatesSize){
    Node::List list;
    Node       a(1), b(2), c(3);
    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    b.Hook.Unlink(&b);
    EXPECT_EQ(list.Size(), 2);
    a.Hook.Unlink(&a);
    c.Hook.Unlink(&c);
    EXPECT(list.Size() == 0 && list.Empty());
}

// Regression: Erase() decremented the size even when the node wasn't in
// this list, underflowing it.
TEST_CASE(IntrusiveList_TestEraseForeignNode){
    Node::List first, second;
    Node       a(1), b(2), unlinked(3);
    first.PushBack(&a);
    second.PushBack(&b);

    first.Erase(&b);
    EXPECT(first.Size() == 1 && second.Size() == 1);
    EXPECT(b.Hook.IsLinked() && second.Head() == &b);

    first.Erase(&unlinked);
    EXPECT_EQ(first.Size(), 1);
    EXPECT_EQ(first.Erase(first.end()), first.end());
}

// Regression: move constructor/assignment dropped m_Size.
TEST_CASE(IntrusiveList_TestMovePreservesSizeAndOwner){
    Node       a(1), b(2);
    Node::List source;
    source.PushBack(&a);
    source.PushBack(&b);

    Node::List moved(Move(source));
    EXPECT(moved.Size() == 2 && source.Size() == 0 && source.Empty());
    EXPECT(a.Hook.Owner == &moved && b.Hook.Owner == &moved);

    Node::List assigned;
    assigned = Move(moved);
    EXPECT(assigned.Size() == 2 && moved.Size() == 0);

    a.Hook.Unlink(&a);
    EXPECT(assigned.Size() == 1 && assigned.Head() == &b);

    assigned = Move(Alias(assigned));
    EXPECT(assigned.Size() == 1 && assigned.Head() == &b);
}

// Regression: lists stored in a Vector (as UnorderedMap does) must survive
// reallocation; relocation goes through the move constructor.
TEST_CASE(IntrusiveList_TestVectorRelocation){
    Vector<Node::List> buckets;
    Node               a(1), b(2);
    buckets.Resize(1);
    buckets[0].PushBack(&a);
    buckets[0].PushBack(&b);
    buckets.Reserve(128);

    EXPECT_EQ(buckets[0].Size(), 2);
    EXPECT_EQ(a.Hook.Owner, (&buckets[0]));
    a.Hook.Unlink(&a);
    EXPECT(buckets[0].Size() == 1 && buckets[0].Head() == &b);
}

// Regression: rbegin()/rend() were wired to the wrong ends and the iterator
// couldn't be decremented, so reverse iteration didn't compile.
TEST_CASE(IntrusiveList_TestReverseIteration){
    Node::List list;
    Node       a(1), b(2), c(3);
    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    int expected = 3;
    for (auto it = list.rbegin(); it != list.rend(); ++it)
        EXPECT_EQ(((*it).Value), expected--);
    EXPECT_EQ(expected, 0);

    auto it = list.end();
    --it;
    EXPECT_EQ((&*it), (&c));
}

// Erase(iterator) must return the following node.
TEST_CASE(IntrusiveList_TestEraseReturnsNext){
    Node::List list;
    Node       a(1), b(2), c(3);
    list.PushBack(&a);
    list.PushBack(&b);
    list.PushBack(&c);

    auto it = list.Erase(list.begin());
    EXPECT(it.Current == &b && list.Size() == 2);
    it = list.Erase(Node::List::Iterator(&c, &list));
    EXPECT(it == list.end() && list.Size() == 1);
    EXPECT(list.Head() == &b && list.Tail() == &b);
}


