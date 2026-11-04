/*
 * Created by v1tr10l7 on 04.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/IntrusiveRefList.hpp>

#include <cstdio>
#include <cstdlib>

// Unlike assert(), this still fires when NDEBUG is defined (release builds).
#define Check(expr)                                                            \
    do {                                                                       \
        if (!(expr))                                                           \
        {                                                                      \
            std::fprintf(stderr, "%s:%d: Check failed: %s\n", __FILE__,       \
                         __LINE__, #expr);                                     \
            std::abort();                                                      \
        }                                                                      \
    } while (0)

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

    void PushIterateAndArrow()
    {
        List list;
        list.PushBack(CreateRef<Node>(1));
        list.PushBack(CreateRef<Node>(2));
        list.PushBack(CreateRef<Node>(3));
        Check(list.Size() == 3);

        // Regression: Iterator::operator-> returned T&, so `it->Value`
        // didn't compile.
        int sum = 0;
        for (auto it = list.begin(); it != list.end(); ++it) sum += it->Value;
        Check(sum == 6);

        int expected = 3;
        for (auto it = list.rbegin(); it != list.rend(); ++it)
            Check((*it)->Value == expected--);
        Check(expected == 0);
    }

    void UnlinkAndEraseKeepSize()
    {
        List list;
        auto a = CreateRef<Node>(1);
        auto b = CreateRef<Node>(2);
        auto c = CreateRef<Node>(3);
        list.PushBack(a);
        list.PushBack(b);
        list.PushBack(c);

        b->Hook.Unlink(b);
        Check(list.Size() == 2);

        List other;
        list.Erase(b); // not in `list` anymore
        Check(list.Size() == 2);
        other.PushBack(b);
        list.Erase(b); // belongs to `other`
        Check(list.Size() == 2 && other.Size() == 1);
        other.Clear();

        auto next = list.Erase(list.begin());
        Check(next.Current == c && list.Size() == 1);
        list.Clear();
        Check(list.Empty() && list.Size() == 0);
    }

    void MovePreservesSize()
    {
        List source;
        source.PushBack(CreateRef<Node>(1));
        source.PushBack(CreateRef<Node>(2));

        List moved(Move(source));
        Check(moved.Size() == 2 && source.Size() == 0);
        Check(moved.Head()->Hook.Owner == &moved);
    }

    void NodesFreedAfterClear()
    {
        g_Alive = 0;
        {
            List list;
            list.PushBack(CreateRef<Node>(1));
            list.PushBack(CreateRef<Node>(2));
            Check(g_Alive == 2);
            list.Clear();
            Check(g_Alive == 0);

            list.PushBack(CreateRef<Node>(3));
        }
        Check(g_Alive == 0);
    }
} // namespace

int main()
{
    PushIterateAndArrow();
    UnlinkAndEraseKeepSize();
    MovePreservesSize();
    NodesFreedAfterClear();

    std::puts("All IntrusiveRefList tests passed!");
    return 0;
}
