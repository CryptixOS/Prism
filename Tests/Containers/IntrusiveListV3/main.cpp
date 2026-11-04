/*
 * Created by v1tr10l7 on 04.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */

#include <cstdio>
#include <cstdlib>

#if PRISM_TEST_MODULES
// Headers (std, fmt, macros) must come before any import.
#include <Prism/Macros.hpp>
import Prism;
#else
#include <Prism/Containers/IntrusiveListV3.hpp>
#include <Prism/Containers/Vector.hpp>
#endif

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

// Hides self-assignment from -Wself-move / -Wself-assign.
template <typename T>
T& Alias(T& value)
{
    return value;
}

namespace
{
    // Scoped here, not at global scope: in the module build the old
    // IntrusiveList/IntrusiveListHook are also visible globally (through
    // PRISM_USE_NAMESPACE), and these must take precedence.
    using Prism::Vector;
    using Prism::V3::IntrusiveList;
    using Prism::V3::IntrusiveListHook;
    using Prism::V3::IntrusiveOwningListHook;
    using Prism::V3::IntrusiveRefListHook;

    template <typename List>
    bool Matches(const List& list, std::initializer_list<int> expected)
    {
        if (list.Size() != expected.size()) return false;

        auto it = expected.begin();
        for (const auto& node : list)
            if (it == expected.end() || node.Value != *it++) return false;
        if (it != expected.end()) return false;

        // Walk backwards too, so Previous links are checked as well.
        auto rit = expected.end();
        for (auto r = list.rbegin(); r != list.rend(); ++r)
            if (rit == expected.begin() || (*r).Value != *--rit) return false;

        return rit == expected.begin();
    }

    // ---------------------------------------------------------------- raw
    struct Node
    {
        int                     Value;
        IntrusiveListHook<Node> Hook;

        explicit Node(int v)
            : Value(v)
        {
        }
    };
    using List = IntrusiveList<Node>;

    void Raw_PushPopOrder()
    {
        List list;
        Node a(1), b(2), c(3);
        Check(list.Empty() && list.Size() == 0);
        Check(list.begin() == list.end());
        Check(list.rbegin() == list.rend());

        list.PushBack(&b);
        list.PushFront(&a);
        list.PushBack(&c);
        Check(Matches(list, {1, 2, 3}));
        Check(list.Head() == &a && list.Tail() == &c);

        Check(list.TakeFront() == &a);
        Check(list.TakeBack() == &c);
        Check(Matches(list, {2}));
        Check(!a.Hook.IsLinked() && !c.Hook.IsLinked());

        list.PopFront();
        Check(list.Empty() && list.Head() == nullptr && list.Tail() == nullptr);
        Check(list.TakeFront() == nullptr);
        list.PopBack(); // no-op on empty list
    }

    void Raw_HookUnlinkKeepsSize()
    {
        List list;
        Node a(1), b(2), c(3);
        list.PushBack(&a);
        list.PushBack(&b);
        list.PushBack(&c);

        b.Hook.Unlink();
        Check(list.Size() == 2);
        Check(Matches(list, {1, 3}));

        a.Hook.Unlink();
        c.Hook.Unlink();
        Check(list.Empty() && list.Size() == 0);
        Check(list.Head() == nullptr && list.Tail() == nullptr);

        // Unlinking an unlinked hook is a no-op.
        a.Hook.Unlink();
        Check(list.Size() == 0);
    }

    void Raw_EraseForeignNodeIsRejected()
    {
        List first, second;
        Node a(1), b(2);
        first.PushBack(&a);
        second.PushBack(&b);

        Check(!first.Erase(b));
        Check(!first.Erase(static_cast<Node*>(nullptr)));
        Check(first.Size() == 1 && second.Size() == 1);
        Check(second.Contains(b) && !first.Contains(b));

        Node unlinked(3);
        Check(!first.Erase(unlinked));
        Check(first.Size() == 1);
    }

    void Raw_EraseDuringIteration()
    {
        List list;
        Node nodes[] = {Node(1), Node(2), Node(3), Node(4), Node(5)};
        for (auto& n : nodes) list.PushBack(&n);

        for (auto it = list.begin(); it != list.end();)
        {
            if (it->Value % 2 == 0) it = list.Erase(it);
            else ++it;
        }
        Check(Matches(list, {1, 3, 5}));

        auto it = list.Erase(list.begin());
        Check(it != list.end() && it->Value == 3);
        it = list.Erase(list.IteratorTo(nodes[4]));
        Check(it == list.end());
        Check(Matches(list, {3}));
    }

    void Raw_InsertBeforeAfter()
    {
        List list;
        Node a(1), b(2), c(3), d(4), e(5);
        list.PushBack(&c);
        list.InsertBefore(list.IteratorTo(c), &a);
        list.InsertAfter(list.IteratorTo(a), &b);
        list.InsertBefore(list.end(), &e);
        list.InsertAfter(list.IteratorTo(c), &d);
        Check(Matches(list, {1, 2, 3, 4, 5}));
    }

    void Raw_ReverseAndBidirectional()
    {
        List list;
        Node a(1), b(2), c(3);
        list.PushBack(&a);
        list.PushBack(&b);
        list.PushBack(&c);

        auto it = list.end();
        --it;
        Check(it->Value == 3);
        --it;
        Check(it->Value == 2);
        it++;
        Check(it->Value == 3);

        int expected = 3;
        for (auto r = list.rbegin(); r != list.rend(); ++r)
            Check(r->Value == expected--);
        Check(expected == 0);
    }

    void Raw_MoveTransfersOwnership()
    {
        Node a(1), b(2), c(3);
        List source;
        source.PushBack(&a);
        source.PushBack(&b);

        List moved(Prism::Move(source));
        Check(source.Empty() && source.Size() == 0);
        Check(Matches(moved, {1, 2}));
        Check(moved.Contains(a) && !source.Contains(a));

        // Unlinking through the hook must update the *new* owner.
        a.Hook.Unlink();
        Check(Matches(moved, {2}));

        List assigned;
        assigned.PushBack(&c);
        assigned = Prism::Move(moved);
        Check(!c.Hook.IsLinked()); // old contents released
        Check(Matches(assigned, {2}));
        Check(moved.Empty());

        assigned = Prism::Move(Alias(assigned)); // self-move must not clear
        Check(Matches(assigned, {2}));
    }

    void Raw_SurvivesVectorRelocation()
    {
        // Vector moves elements on growth; every hook must follow its list.
        Vector<List> buckets;
        Node         nodes[] = {Node(1), Node(2), Node(3), Node(4)};
        buckets.Resize(1);
        buckets[0].PushBack(&nodes[0]);
        buckets[0].PushBack(&nodes[1]);
        for (int i = 0; i < 64; ++i) buckets.EmplaceBack();
        buckets[10].PushBack(&nodes[2]);
        buckets.Reserve(1024);

        Check(Matches(buckets[0], {1, 2}));
        nodes[0].Hook.Unlink();
        Check(Matches(buckets[0], {2}));
        Check(buckets[10].Contains(nodes[2]));
    }

    void Raw_DestroyedNodeUnlinksItself()
    {
        List list;
        Node a(1), c(3);
        list.PushBack(&a);
        {
            Node b(2);
            list.PushBack(&b);
            list.PushBack(&c);
        }
        Check(Matches(list, {1, 3}));
    }

    void Raw_ListDestructorUnlinksNodes()
    {
        Node a(1), b(2);
        {
            List list;
            list.PushBack(&a);
            list.PushBack(&b);
        }
        Check(!a.Hook.IsLinked() && !b.Hook.IsLinked());
        Check(a.Hook.Next() == nullptr && b.Hook.Previous() == nullptr);
    }

    void Raw_CopiedNodeStartsUnlinked()
    {
        List list;
        Node a(1);
        list.PushBack(&a);

        Node copy = a;
        Check(!copy.Hook.IsLinked());
        Check(list.Size() == 1);

        Node assigned(5);
        assigned = a;
        Check(!assigned.Hook.IsLinked() && assigned.Value == 1);
        list.PushBack(&copy);
        Check(Matches(list, {1, 1}));
    }

    void Raw_Splice()
    {
        List a, b;
        Node n1(1), n2(2), n3(3), n4(4);
        a.PushBack(&n1);
        a.PushBack(&n2);
        b.PushBack(&n3);
        b.PushBack(&n4);

        a.Splice(b);
        Check(b.Empty());
        Check(Matches(a, {1, 2, 3, 4}));
        n3.Hook.Unlink();
        Check(Matches(a, {1, 2, 4}));

        List empty;
        empty.Splice(a);
        Check(a.Empty() && Matches(empty, {1, 2, 4}));
    }

    void Raw_ConstIteration()
    {
        List list;
        Node a(1), b(2);
        list.PushBack(&a);
        list.PushBack(&b);

        const List& view = list;
        int         sum  = 0;
        for (const Node& n : view) sum += n.Value;
        Check(sum == 3);
        Check(view.cbegin() == List::ConstIterator(list.begin()));
    }

    // A node that lives in two lists at once through two hooks.
    struct MultiNode
    {
        int                          Value;
        IntrusiveListHook<MultiNode> Hook;
        IntrusiveListHook<MultiNode> ReadyHook;

        explicit MultiNode(int v)
            : Value(v)
        {
        }
    };

    void Raw_CustomHookMember()
    {
        IntrusiveList<MultiNode>                        all;
        IntrusiveList<MultiNode, &MultiNode::ReadyHook> ready;
        MultiNode                                       a(1), b(2), c(3);
        all.PushBack(&a);
        all.PushBack(&b);
        all.PushBack(&c);
        ready.PushBack(&c);
        ready.PushBack(&a);

        Check(Matches(all, {1, 2, 3}));
        Check(Matches(ready, {3, 1}));

        a.ReadyHook.Unlink();
        Check(Matches(all, {1, 2, 3}));
        Check(Matches(ready, {3}));
    }

    // ---------------------------------------------------------------- Ref
    int g_Alive = 0;

    struct RefNode : Prism::RefCounted<RefNode>
    {
        int                           Value;
        IntrusiveRefListHook<RefNode> Hook;

        explicit RefNode(int v)
            : Value(v)
        {
            ++g_Alive;
        }
        ~RefNode() { --g_Alive; }
    };
    using RefList = IntrusiveList<RefNode>;

    void Ref_ListHoldsOneReference()
    {
        g_Alive = 0;
        {
            RefList list;
            auto    a = Prism::CreateRef<RefNode>(1);
            auto    b = Prism::CreateRef<RefNode>(2);
            list.PushBack(a);
            list.PushBack(b);
            Check(a.RefCount() == 2 && b.RefCount() == 2);

            // Traversal must not touch reference counts.
            for (auto& node : list) (void)node;
            Check(a.RefCount() == 2);

            a = nullptr;
            b = nullptr;
            Check(g_Alive == 2); // kept alive by the list
            Check(Matches(list, {1, 2}));
        }
        Check(g_Alive == 0); // list destructor released both
    }

    void Ref_EraseReleases()
    {
        g_Alive = 0;
        RefList list;
        list.PushBack(Prism::CreateRef<RefNode>(1));
        list.PushBack(Prism::CreateRef<RefNode>(2));
        list.PushBack(Prism::CreateRef<RefNode>(3));
        Check(g_Alive == 3);

        list.Erase(list.begin());
        Check(g_Alive == 2 && Matches(list, {2, 3}));

        list.Head()->Hook.Unlink(); // destroys the node, hook included
        Check(g_Alive == 1 && Matches(list, {3}));

        list.Clear();
        Check(g_Alive == 0 && list.Empty());
    }

    void Ref_TakeReturnsOwnership()
    {
        g_Alive = 0;
        RefList list;
        list.PushBack(Prism::CreateRef<RefNode>(7));

        Prism::Ref<RefNode> taken = list.TakeFront();
        Check(taken && taken->Value == 7);
        Check(taken.RefCount() == 1);
        Check(list.Empty() && g_Alive == 1);
        Check(!list.TakeFront());

        taken = nullptr;
        Check(g_Alive == 0);
    }

    void Ref_EmplaceAndMove()
    {
        g_Alive = 0;
        {
            RefList list;
            list.EmplaceBack(1);
            list.EmplaceFront(0);
            Check(Matches(list, {0, 1}));
            Check(list.Head()->RefCount() == 1);

            RefList moved(Prism::Move(list));
            Check(g_Alive == 2 && Matches(moved, {0, 1}));
            moved.Head()->Hook.Unlink();
            Check(g_Alive == 1 && moved.Size() == 1);
        }
        Check(g_Alive == 0);
    }

    void Ref_RejectsDoubleInsert()
    {
        g_Alive = 0;
        {
            RefList first, second;
            auto    node = Prism::CreateRef<RefNode>(1);
            first.PushBack(node);
            // Pushing a node that's already linked must not take another
            // reference (it would leak) and must not relink it. This path
            // also asserts in debug builds, so only exercise it with NDEBUG.
#ifdef NDEBUG
            second.PushBack(node);
            Check(second.Empty() && node.RefCount() == 2);
#endif
            Check(first.Size() == 1);
        }
        Check(g_Alive == 0);
    }

    // -------------------------------------------------------------- Scope
    struct OwnedNode
    {
        int                                Value;
        IntrusiveOwningListHook<OwnedNode> Hook;

        explicit OwnedNode(int v)
            : Value(v)
        {
            ++g_Alive;
        }
        ~OwnedNode() { --g_Alive; }
    };
    using OwningList = IntrusiveList<OwnedNode>;

    void Scope_ListOwnsNodes()
    {
        g_Alive = 0;
        {
            OwningList list;
            list.PushBack(Prism::CreateScope<OwnedNode>(1));
            list.EmplaceBack(2);
            list.EmplaceBack(3);
            Check(g_Alive == 3 && Matches(list, {1, 2, 3}));

            list.PopFront();
            Check(g_Alive == 2);

            Prism::Scope<OwnedNode> taken = list.TakeBack();
            Check(taken && taken->Value == 3 && g_Alive == 2);
            taken.Reset();
            Check(g_Alive == 1);
        }
        Check(g_Alive == 0);
    }
} // namespace

int main()
{
    Raw_PushPopOrder();
    Raw_HookUnlinkKeepsSize();
    Raw_EraseForeignNodeIsRejected();
    Raw_EraseDuringIteration();
    Raw_InsertBeforeAfter();
    Raw_ReverseAndBidirectional();
    Raw_MoveTransfersOwnership();
    Raw_SurvivesVectorRelocation();
    Raw_DestroyedNodeUnlinksItself();
    Raw_ListDestructorUnlinksNodes();
    Raw_CopiedNodeStartsUnlinked();
    Raw_Splice();
    Raw_ConstIteration();
    Raw_CustomHookMember();

    Ref_ListHoldsOneReference();
    Ref_EraseReleases();
    Ref_TakeReturnsOwnership();
    Ref_EmplaceAndMove();
    Ref_RejectsDoubleInsert();

    Scope_ListOwnsNodes();

    std::puts("All IntrusiveListV3 tests passed!");
    return 0;
}
