/*
 * Created by v1tr10l7 on 04.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Containers/IntrusiveListV3.hpp>
#include <Prism/Containers/Vector.hpp>

#include <cstdio>
#include <cstdlib>

// Unlike EXPECT(), this still fires when NDEBUG is defined (release builds).

// Hides self-assignment from -Wself-move / -Wself-assign.
template <typename T>
T& Alias(T& value)
{
    return value;
}

using Prism::Vector;
using Prism::V3::IntrusiveList;
using Prism::V3::IntrusiveListHook;
using Prism::V3::IntrusiveOwningListHook;
using Prism::V3::IntrusiveRefListHook;

namespace
{
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

    TEST_CASE(Raw_PushPopOrder){
        List list;
        Node a(1), b(2), c(3);
        EXPECT(list.Empty() && list.Size() == 0);
        EXPECT_EQ(list.begin(), list.end());
        EXPECT_EQ(list.rbegin(), list.rend());

        list.PushBack(&b);
        list.PushFront(&a);
        list.PushBack(&c);
        EXPECT(Matches(list, {1, 2, 3}));
        EXPECT(list.Head() == &a && list.Tail() == &c);

        EXPECT_EQ(list.TakeFront(), (&a));
        EXPECT_EQ(list.TakeBack(), (&c));
        EXPECT(Matches(list, {2}));
        EXPECT(!a.Hook.IsLinked() && !c.Hook.IsLinked());

        list.PopFront();
        EXPECT(list.Empty() && list.Head() == nullptr && list.Tail() == nullptr);
        EXPECT_EQ(list.TakeFront(), nullptr);
        list.PopBack(); // no-op on empty list
    }

    TEST_CASE(Raw_HookUnlinkKeepsSize){
        List list;
        Node a(1), b(2), c(3);
        list.PushBack(&a);
        list.PushBack(&b);
        list.PushBack(&c);

        b.Hook.Unlink();
        EXPECT_EQ(list.Size(), 2);
        EXPECT(Matches(list, {1, 3}));

        a.Hook.Unlink();
        c.Hook.Unlink();
        EXPECT(list.Empty() && list.Size() == 0);
        EXPECT(list.Head() == nullptr && list.Tail() == nullptr);

        // Unlinking an unlinked hook is a no-op.
        a.Hook.Unlink();
        EXPECT_EQ(list.Size(), 0);
    }

    TEST_CASE(Raw_EraseForeignNodeIsRejected){
        List first, second;
        Node a(1), b(2);
        first.PushBack(&a);
        second.PushBack(&b);

        EXPECT(!first.Erase(b));
        EXPECT(!first.Erase(static_cast<Node*>(nullptr)));
        EXPECT(first.Size() == 1 && second.Size() == 1);
        EXPECT(second.Contains(b) && !first.Contains(b));

        Node unlinked(3);
        EXPECT(!first.Erase(unlinked));
        EXPECT_EQ(first.Size(), 1);
    }

    TEST_CASE(Raw_EraseDuringIteration){
        List list;
        Node nodes[] = {Node(1), Node(2), Node(3), Node(4), Node(5)};
        for (auto& n : nodes) list.PushBack(&n);

        for (auto it = list.begin(); it != list.end();)
        {
            if (it->Value % 2 == 0) it = list.Erase(it);
            else ++it;
        }
        EXPECT(Matches(list, {1, 3, 5}));

        auto it = list.Erase(list.begin());
        EXPECT(it != list.end() && it->Value == 3);
        it = list.Erase(list.IteratorTo(nodes[4]));
        EXPECT_EQ(it, list.end());
        EXPECT(Matches(list, {3}));
    }

    TEST_CASE(Raw_InsertBeforeAfter){
        List list;
        Node a(1), b(2), c(3), d(4), e(5);
        list.PushBack(&c);
        list.InsertBefore(list.IteratorTo(c), &a);
        list.InsertAfter(list.IteratorTo(a), &b);
        list.InsertBefore(list.end(), &e);
        list.InsertAfter(list.IteratorTo(c), &d);
        EXPECT(Matches(list, {1, 2, 3, 4, 5}));
    }

    TEST_CASE(Raw_ReverseAndBidirectional){
        List list;
        Node a(1), b(2), c(3);
        list.PushBack(&a);
        list.PushBack(&b);
        list.PushBack(&c);

        auto it = list.end();
        --it;
        EXPECT(it->Value == 3);
        --it;
        EXPECT(it->Value == 2);
        it++;
        EXPECT(it->Value == 3);

        int expected = 3;
        for (auto r = list.rbegin(); r != list.rend(); ++r)
            EXPECT(r->Value == expected--);
        EXPECT_EQ(expected, 0);
    }

    TEST_CASE(Raw_MoveTransfersOwnership){
        Node a(1), b(2), c(3);
        List source;
        source.PushBack(&a);
        source.PushBack(&b);

        List moved(Prism::Move(source));
        EXPECT(source.Empty() && source.Size() == 0);
        EXPECT(Matches(moved, {1, 2}));
        EXPECT(moved.Contains(a) && !source.Contains(a));

        // Unlinking through the hook must update the *new* owner.
        a.Hook.Unlink();
        EXPECT(Matches(moved, {2}));

        List assigned;
        assigned.PushBack(&c);
        assigned = Prism::Move(moved);
        EXPECT(!c.Hook.IsLinked()); // old contents released
        EXPECT(Matches(assigned, {2}));
        EXPECT(moved.Empty());

        assigned = Prism::Move(Alias(assigned)); // self-move must not clear
        EXPECT(Matches(assigned, {2}));
    }

    TEST_CASE(Raw_SurvivesVectorRelocation){
        // Vector moves elements on growth; every hook must follow its list.
        Vector<List> buckets;
        Node         nodes[] = {Node(1), Node(2), Node(3), Node(4)};
        buckets.Resize(1);
        buckets[0].PushBack(&nodes[0]);
        buckets[0].PushBack(&nodes[1]);
        for (int i = 0; i < 64; ++i) buckets.EmplaceBack();
        buckets[10].PushBack(&nodes[2]);
        buckets.Reserve(1024);

        EXPECT(Matches(buckets[0], {1, 2}));
        nodes[0].Hook.Unlink();
        EXPECT(Matches(buckets[0], {2}));
        EXPECT(buckets[10].Contains(nodes[2]));
    }

    TEST_CASE(Raw_DestroyedNodeUnlinksItself){
        List list;
        Node a(1), c(3);
        list.PushBack(&a);
        {
            Node b(2);
            list.PushBack(&b);
            list.PushBack(&c);
        }
        EXPECT(Matches(list, {1, 3}));
    }

    TEST_CASE(Raw_ListDestructorUnlinksNodes){
        Node a(1), b(2);
        {
            List list;
            list.PushBack(&a);
            list.PushBack(&b);
        }
        EXPECT(!a.Hook.IsLinked() && !b.Hook.IsLinked());
        EXPECT(a.Hook.Next() == nullptr && b.Hook.Previous() == nullptr);
    }

    TEST_CASE(Raw_CopiedNodeStartsUnlinked){
        List list;
        Node a(1);
        list.PushBack(&a);

        Node copy = a;
        EXPECT(!copy.Hook.IsLinked());
        EXPECT_EQ(list.Size(), 1);

        Node assigned(5);
        assigned = a;
        EXPECT(!assigned.Hook.IsLinked() && assigned.Value == 1);
        list.PushBack(&copy);
        EXPECT(Matches(list, {1, 1}));
    }

    TEST_CASE(Raw_Splice){
        List a, b;
        Node n1(1), n2(2), n3(3), n4(4);
        a.PushBack(&n1);
        a.PushBack(&n2);
        b.PushBack(&n3);
        b.PushBack(&n4);

        a.Splice(b);
        EXPECT(b.Empty());
        EXPECT(Matches(a, {1, 2, 3, 4}));
        n3.Hook.Unlink();
        EXPECT(Matches(a, {1, 2, 4}));

        List empty;
        empty.Splice(a);
        EXPECT(a.Empty() && Matches(empty, {1, 2, 4}));
    }

    TEST_CASE(Raw_ConstIteration){
        List list;
        Node a(1), b(2);
        list.PushBack(&a);
        list.PushBack(&b);

        const List& view = list;
        int         sum  = 0;
        for (const Node& n : view) sum += n.Value;
        EXPECT_EQ(sum, 3);
        EXPECT_EQ(view.cbegin(), List::ConstIterator(list.begin()));
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

    TEST_CASE(Raw_CustomHookMember){
        IntrusiveList<MultiNode>                        all;
        IntrusiveList<MultiNode, &MultiNode::ReadyHook> ready;
        MultiNode                                       a(1), b(2), c(3);
        all.PushBack(&a);
        all.PushBack(&b);
        all.PushBack(&c);
        ready.PushBack(&c);
        ready.PushBack(&a);

        EXPECT(Matches(all, {1, 2, 3}));
        EXPECT(Matches(ready, {3, 1}));

        a.ReadyHook.Unlink();
        EXPECT(Matches(all, {1, 2, 3}));
        EXPECT(Matches(ready, {3}));
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

    TEST_CASE(Ref_ListHoldsOneReference){
        g_Alive = 0;
        {
            RefList list;
            auto    a = Prism::CreateRef<RefNode>(1);
            auto    b = Prism::CreateRef<RefNode>(2);
            list.PushBack(a);
            list.PushBack(b);
            EXPECT(a.RefCount() == 2 && b.RefCount() == 2);

            // Traversal must not touch reference counts.
            for (auto& node : list) (void)node;
            EXPECT_EQ(a.RefCount(), 2);

            a = nullptr;
            b = nullptr;
            EXPECT_EQ(g_Alive, 2); // kept alive by the list
            EXPECT(Matches(list, {1, 2}));
        }
        EXPECT_EQ(g_Alive, 0); // list destructor released both
    }

    TEST_CASE(Ref_EraseReleases){
        g_Alive = 0;
        RefList list;
        list.PushBack(Prism::CreateRef<RefNode>(1));
        list.PushBack(Prism::CreateRef<RefNode>(2));
        list.PushBack(Prism::CreateRef<RefNode>(3));
        EXPECT_EQ(g_Alive, 3);

        list.Erase(list.begin());
        EXPECT(g_Alive == 2 && Matches(list, {2, 3}));

        list.Head()->Hook.Unlink(); // destroys the node, hook included
        EXPECT(g_Alive == 1 && Matches(list, {3}));

        list.Clear();
        EXPECT(g_Alive == 0 && list.Empty());
    }

    TEST_CASE(Ref_TakeReturnsOwnership){
        g_Alive = 0;
        RefList list;
        list.PushBack(Prism::CreateRef<RefNode>(7));

        Prism::Ref<RefNode> taken = list.TakeFront();
        EXPECT(taken && taken->Value == 7);
        EXPECT_EQ(taken.RefCount(), 1);
        EXPECT(list.Empty() && g_Alive == 1);
        EXPECT(!list.TakeFront());

        taken = nullptr;
        EXPECT_EQ(g_Alive, 0);
    }

    TEST_CASE(Ref_EmplaceAndMove){
        g_Alive = 0;
        {
            RefList list;
            list.EmplaceBack(1);
            list.EmplaceFront(0);
            EXPECT(Matches(list, {0, 1}));
            EXPECT(list.Head()->RefCount() == 1);

            RefList moved(Prism::Move(list));
            EXPECT(g_Alive == 2 && Matches(moved, {0, 1}));
            moved.Head()->Hook.Unlink();
            EXPECT(g_Alive == 1 && moved.Size() == 1);
        }
        EXPECT_EQ(g_Alive, 0);
    }

    TEST_CASE(Ref_RejectsDoubleInsert){
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
            EXPECT(second.Empty() && node.RefCount() == 2);
#endif
            EXPECT_EQ(first.Size(), 1);
        }
        EXPECT_EQ(g_Alive, 0);
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

    TEST_CASE(Scope_ListOwnsNodes){
        g_Alive = 0;
        {
            OwningList list;
            list.PushBack(Prism::CreateScope<OwnedNode>(1));
            list.EmplaceBack(2);
            list.EmplaceBack(3);
            EXPECT(g_Alive == 3 && Matches(list, {1, 2, 3}));

            list.PopFront();
            EXPECT_EQ(g_Alive, 2);

            Prism::Scope<OwnedNode> taken = list.TakeBack();
            EXPECT(taken && taken->Value == 3 && g_Alive == 2);
            taken.Reset();
            EXPECT_EQ(g_Alive, 1);
        }
        EXPECT_EQ(g_Alive, 0);
    }
} // namespace


