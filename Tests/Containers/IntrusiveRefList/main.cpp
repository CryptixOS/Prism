/*
 * Created by v1tr10l7 on 04.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Containers/IntrusiveRefList.hpp>

#include <cstdio>
#include <cstdlib>

// Unlike EXPECT(), this still fires when NDEBUG is defined (release builds).

using namespace Prism;

namespace
{
    int g_Alive = 0;

    struct Node : RefCounted<Node>
    {
        int                        Value;
        IntrusiveRefListHook<Node> Hook;

        explicit Node(int v)
            : Value(v)
        {
            ++g_Alive;
        }
        ~Node() { --g_Alive; }
    };
    using List = IntrusiveRefList<Node>;

    TEST_CASE(PushIterateAndArrow){
        List list;
        list.PushBack(CreateRef<Node>(1));
        list.PushBack(CreateRef<Node>(2));
        list.PushBack(CreateRef<Node>(3));
        EXPECT_EQ(list.Size(), 3);

        // Regression: Iterator::operator-> returned T&, so `it->Value`
        // didn't compile.
        int sum = 0;
        for (auto it = list.begin(); it != list.end(); ++it) sum += it->Value;
        EXPECT_EQ(sum, 6);

        int expected = 3;
        for (auto it = list.rbegin(); it != list.rend(); ++it)
            EXPECT((*it)->Value == expected--);
        EXPECT_EQ(expected, 0);
    }

    TEST_CASE(UnlinkAndEraseKeepSize){
        List list;
        auto a = CreateRef<Node>(1);
        auto b = CreateRef<Node>(2);
        auto c = CreateRef<Node>(3);
        list.PushBack(a);
        list.PushBack(b);
        list.PushBack(c);

        b->Hook.Unlink(b);
        EXPECT_EQ(list.Size(), 2);

        List other;
        list.Erase(b); // not in `list` anymore
        EXPECT_EQ(list.Size(), 2);
        other.PushBack(b);
        list.Erase(b); // belongs to `other`
        EXPECT(list.Size() == 2 && other.Size() == 1);
        other.Clear();

        auto next = list.Erase(list.begin());
        EXPECT(next.Current == c && list.Size() == 1);
        list.Clear();
        EXPECT(list.Empty() && list.Size() == 0);
    }

    TEST_CASE(MovePreservesSize){
        List source;
        source.PushBack(CreateRef<Node>(1));
        source.PushBack(CreateRef<Node>(2));

        List moved(Move(source));
        EXPECT(moved.Size() == 2 && source.Size() == 0);
        EXPECT(moved.Head()->Hook.Owner == &moved);
    }

    TEST_CASE(NodesFreedAfterClear){
        g_Alive = 0;
        {
            List list;
            list.PushBack(CreateRef<Node>(1));
            list.PushBack(CreateRef<Node>(2));
            EXPECT_EQ(g_Alive, 2);
            list.Clear();
            EXPECT_EQ(g_Alive, 0);

            list.PushBack(CreateRef<Node>(3));
        }
        EXPECT_EQ(g_Alive, 0);
    }
} // namespace


