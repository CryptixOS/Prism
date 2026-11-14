/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
import Prism.Containers.Array;
import Prism.Containers.CircularQueue;
import Prism.Containers.Deque;
import Prism.Containers.DoublyLinkedList;
import Prism.Containers.IntervalTree;
import Prism.Containers.Queue;
import Prism.Containers.Set;
import Prism.Containers.Span;
import Prism.Containers.SplitBuffer;
import Prism.Containers.Stack;
import Prism.Containers.UnorderedMap;
import Prism.Containers.Vector;
import Prism.Memory.Scope;
import Prism.Test.Test;

using namespace Prism;

// The edge cases every container must get right: empty, one element,
// growing past its first allocation, and move-only element types (which
// also catch copies sneaking into growth or erase paths).

namespace
{
    // Counts live instances, so leaks and double destructions show up.
    struct Tracked
    {
        static inline int s_Live = 0;
        int               Value  = 0;

        Tracked(int value = 0)
            : Value(value)
        {
            ++s_Live;
        }
        Tracked(const Tracked& other)
            : Value(other.Value)
        {
            ++s_Live;
        }
        Tracked(Tracked&& other)
            : Value(other.Value)
        {
            other.Value = -1;
            ++s_Live;
        }
        Tracked& operator=(const Tracked&) = default;
        Tracked& operator=(Tracked&&)      = default;
        ~Tracked() { --s_Live; }
        bool operator==(const Tracked& other) const
        {
            return Value == other.Value;
        }
    };

    struct MoveOnly
    {
        Scope<int> Value;
        MoveOnly(int value)
            : Value(CreateScope<int>(value))
        {
        }
        MoveOnly(MoveOnly&&)                 = default;
        MoveOnly& operator=(MoveOnly&&)      = default;
        MoveOnly(const MoveOnly&)            = delete;
        MoveOnly& operator=(const MoveOnly&) = delete;
    };

    constexpr int kGrowth = 1000;
} // namespace

TEST_CASE(vector)
{
    {
        Vector<Tracked> v;
        EXPECT(v.Empty());
        EXPECT_EQ(v.Size(), 0u);
        EXPECT(v.begin() == v.end());
        v.PopBack(); // a no-op on an empty vector
        EXPECT(v.Empty());

        v.PushBack(Tracked(7));
        EXPECT_EQ(v.Size(), 1u);
        EXPECT_EQ(v.Front().Value, 7);
        EXPECT_EQ(v.Back().Value, 7);
        v.Erase(v.begin());
        EXPECT(v.Empty());

        for (int i = 0; i < kGrowth; ++i) v.EmplaceBack(i);
        EXPECT_EQ(v.Size(), usize(kGrowth));
        for (int i = 0; i < kGrowth; ++i) EXPECT_EQ(v[i].Value, i);
        v.Erase(v.begin() + 10, v.begin() + 20);
        EXPECT_EQ(v.Size(), usize(kGrowth - 10));
        EXPECT_EQ(v[10].Value, 20);
        v.ShrinkToFit();
        EXPECT_EQ(v.Capacity(), v.Size());
    }
    EXPECT_EQ(Tracked::s_Live, 0);

    Vector<MoveOnly> m;
    for (int i = 0; i < kGrowth; ++i) m.EmplaceBack(i);
    m.Erase(m.begin());
    EXPECT_EQ(*m.Front().Value, 1);
    EXPECT_EQ(*m.Back().Value, kGrowth - 1);
    Vector<MoveOnly> moved = Move(m);
    EXPECT_EQ(moved.Size(), usize(kGrowth - 1));
    EXPECT(m.Empty());
}

TEST_CASE(array_and_span)
{
    Array<int, 0> empty{};
    EXPECT_EQ(empty.Size(), 0u);
    EXPECT(empty.begin() == empty.end());

    Array<int, 1> one = {5};
    EXPECT_EQ(one.Front(), 5);
    EXPECT_EQ(one.Back(), 5);

    Array<MoveOnly, 2> moveOnly = {MoveOnly(1), MoveOnly(2)};
    EXPECT_EQ(*moveOnly[1].Value, 2);

    Span<int> none;
    EXPECT(none.Empty());
    EXPECT_EQ(none.Size(), 0u);
    int       data[3] = {1, 2, 3};
    Span<int> span(data, 3);
    EXPECT_EQ(span.SubSpan(1).Size(), 2u);
    EXPECT_EQ(span.SubSpan(3).Size(), 0u);
    EXPECT_EQ(span.First(0).Size(), 0u);
    EXPECT_EQ(span.Last(1)[0], 3);
    EXPECT_EQ(span.SubSpan(1, 1)[0], 2);
    EXPECT_EQ((span.First<2>().Size()), 2u);
    EXPECT_EQ((span.Last<1>()[0]), 3);
    EXPECT_EQ((span.SubSpan<1>().Size()), 2u);
}

TEST_CASE(deque)
{
    {
        Deque<Tracked> d;
        EXPECT(d.Empty());
        EXPECT(d.begin() == d.end());
        d.PushBack(Tracked(1));
        EXPECT_EQ(d.Front().Value, 1);
        EXPECT_EQ(d.Back().Value, 1);
        d.PopFront();
        EXPECT(d.Empty());

        // Grow at both ends, past several blocks.
        for (int i = 0; i < kGrowth; ++i)
        {
            d.PushBack(Tracked(i));
            d.PushFront(Tracked(-i - 1));
        }
        EXPECT_EQ(d.Size(), usize(2 * kGrowth));
        EXPECT_EQ(d.Front().Value, -kGrowth);
        EXPECT_EQ(d.Back().Value, kGrowth - 1);
        EXPECT_EQ(d[kGrowth].Value, 0);
        d.Clear();
        EXPECT(d.Empty());
    }
}

TEST_CASE(lists_queues_stacks)
{
    DoublyLinkedList<int> list;
    EXPECT(list.Empty());
    list.PushBack(1);
    EXPECT_EQ(list.PopFrontElement(), 1);
    EXPECT(list.Empty());
    for (int i = 0; i < kGrowth; ++i) list.PushBack(i);
    EXPECT_EQ(list.Size(), usize(kGrowth));
    EXPECT_EQ(list.PopBackElement(), kGrowth - 1);

    Queue<int> queue;
    EXPECT(queue.Empty());
    queue.Push(1);
    EXPECT_EQ(queue.Front(), 1);
    EXPECT_EQ(queue.Back(), 1);
    queue.Pop();
    EXPECT(queue.Empty());
    for (int i = 0; i < kGrowth; ++i) queue.Push(i);
    EXPECT_EQ(queue.Front(), 0);
    EXPECT_EQ(queue.Size(), usize(kGrowth));

    Stack<int> stack;
    EXPECT(stack.Empty());
    for (int i = 0; i < kGrowth; ++i) stack.Push(i);
    EXPECT_EQ(stack.Size(), usize(kGrowth));
}

TEST_CASE(circular_queue)
{
    {
        CircularQueue<Tracked, 4> q;
        EXPECT(q.Empty());
        EXPECT(q.begin() == q.end());

        q.Push(Tracked(1));
        EXPECT_EQ(q.Front().Value, 1);
        EXPECT_EQ(q.Back().Value, 1);
        EXPECT_EQ(q.Pop().Value, 1);
        EXPECT(q.Empty());

        // Pushing onto a full queue drops the oldest element.
        for (int i = 0; i < 10; ++i) q.Push(Tracked(i));
        EXPECT(q.Full());
        EXPECT_EQ(q.Size(), 4u);
        EXPECT_EQ(q.Front().Value, 6);
        EXPECT_EQ(q.Back().Value, 9);
        int expected = 6;
        for (const Tracked& t : q) EXPECT_EQ(t.Value, expected++);

        EXPECT_EQ(q.PopBack().Value, 9);
        EXPECT_EQ(q.PopFront().Value, 6);
        EXPECT_EQ(q.Size(), 2u);

        CircularQueue<Tracked, 4> copy = q;
        EXPECT_EQ(copy.Size(), 2u);
        EXPECT_EQ(copy[1].Value, 8);
    }
    EXPECT_EQ(Tracked::s_Live, 0);

    CircularQueue<MoveOnly, 3> m;
    for (int i = 0; i < 5; ++i) m.Emplace(i);
    EXPECT_EQ(*m.Front().Value, 2);
    MoveOnly popped = m.Pop();
    EXPECT_EQ(*popped.Value, 2);
}

TEST_CASE(split_buffer)
{
    SplitBuffer<int> empty(0);
    EXPECT_EQ(empty.Size(), 0u);
    // Growing from no capacity at all.
    empty.Insert(0, 1);
    EXPECT_EQ(empty.Size(), 1u);
    EXPECT_EQ(empty[0], 1);
    empty.Erase(0);
    EXPECT_EQ(empty.Size(), 0u);

    SplitBuffer<int> buffer(2);
    for (int i = 0; i < kGrowth; ++i) buffer.Insert(buffer.Size(), i);
    buffer.Insert(0, -1);
    buffer.Insert(500, 12345);
    EXPECT_EQ(buffer.Size(), usize(kGrowth + 2));
    EXPECT_EQ(buffer[0], -1);
    EXPECT_EQ(buffer[1], 0);
    EXPECT_EQ(buffer[500], 12345);
    EXPECT_EQ(buffer[usize(kGrowth + 1)], kGrowth - 1);
}

TEST_CASE(set_and_map)
{
    Set<int> set;
    EXPECT_EQ(set.Size(), 0u);
    EXPECT(!set.Contains(1));
    EXPECT(set.Find(1) == set.end());
    set.Insert(1);
    EXPECT(set.Contains(1));
    EXPECT_EQ(set.Size(), 1u);
    EXPECT(!set.Insert(1).Value); // already there
    for (int i = 0; i < kGrowth; ++i) set.Insert(i);
    EXPECT_EQ(set.Size(), usize(kGrowth));
    int previous = -1;
    for (int value : set)
    {
        EXPECT(value > previous);
        previous = value;
    }

    UnorderedMap<int, MoveOnly> map;
    EXPECT(map.Empty());
    EXPECT(!map.Contains(1));
    map.Emplace(1, MoveOnly(10));
    EXPECT_EQ(*map.At(1).Value, 10);
    for (int i = 2; i < kGrowth; ++i) map.Emplace(i, MoveOnly(i * 10));
    EXPECT_EQ(map.Size(), usize(kGrowth - 1));
    EXPECT_EQ(*map.At(kGrowth - 1).Value, (kGrowth - 1) * 10);
    map.Erase(1);
    EXPECT(!map.Contains(1));
}

TEST_CASE(interval_tree)
{
    IntervalTree<int, int> tree;
    int                    hits = 0;
    tree.ForOverlaps(0, 100, [&](auto&&...) { ++hits; });
    EXPECT_EQ(hits, 0);

    tree.InsertInterval(10, 20, 1);
    tree.ForOverlaps(15, 16, [&](auto&&...) { ++hits; });
    EXPECT_EQ(hits, 1);

    for (int i = 0; i < kGrowth; ++i) tree.InsertInterval(i * 100, i * 100 + 50, i);
    hits = 0;
    tree.ForOverlaps(1020, 1030, [&](auto&&...) { ++hits; });
    EXPECT_EQ(hits, 1);
    hits = 0;
    tree.ForOverlaps(1060, 1090, [&](auto&&...) { ++hits; });
    EXPECT_EQ(hits, 0);
}
