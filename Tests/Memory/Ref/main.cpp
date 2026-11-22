/*
 * Created by v1tr10l7 on 09.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <Prism/Test/Test.hpp>
#include <cstdio>
#include <cstdlib>
import Prism.Test.Test;
import Prism.Memory.Ref;

// Unlike EXPECT(), this still fires when NDEBUG is defined (release builds).

// Hides self-assignment from -Wself-move / -Wself-assign.
template <typename T>
T& Alias(T& value)
{
    return value;
}

using namespace Prism;

struct Dummy : public Prism::RefCounted<Dummy>
{
    int value = 42;
};

TEST_CASE(TestRefBasic){
    auto ref1 = Prism::CreateRef<Dummy>();
    EXPECT(ref1->value == 42);
    EXPECT(ref1->RefCount() == 1);

    Prism::Ref<Dummy> ref2 = ref1;
    EXPECT(ref1->RefCount() == 2);
    EXPECT(ref2->RefCount() == 2);

    Prism::Ref<Dummy> ref3;
    ref3 = ref1;
    EXPECT(ref3->RefCount() == 3);

    ref2 = nullptr;
    EXPECT(ref1->RefCount() == 2);

    ref3.Reset();
    EXPECT(ref1->RefCount() == 1);
}

TEST_CASE(TestRefMove){
    auto ref1 = Prism::CreateRef<Dummy>();
    EXPECT(ref1->RefCount() == 1);

    Prism::Ref<Dummy> ref2 = Move(ref1);
    EXPECT_EQ(ref1.Raw(), nullptr);
    EXPECT(ref2->value == 42);
    EXPECT(ref2->RefCount() == 1);
}

TEST_CASE(TestRefDeleteOnZero){
    bool deleted = false;
    struct Track : Prism::RefCounted<Track>
    {
        bool* del;
        Track(bool* d)
            : del(d)
        {
        }
        ~Track() { *del = true; }
    };

    {
        Prism::Ref<Track> ref(AdoptRef, new Track(&deleted));
        EXPECT(!deleted);
    }
    EXPECT(deleted);
}

TEST_CASE(TestRefCreateHelper){
    auto ref = Prism::CreateRef<Dummy>();
    EXPECT(ref->value == 42);
    EXPECT_EQ(ref.RefCount(), 1);
}

namespace
{
    int  g_Alive         = 0;
    int  g_WillDestroyed = 0;

    struct Base : RefCounted<Base>
    {
        Base() { ++g_Alive; }
        virtual ~Base() { --g_Alive; }
        virtual int Kind() const { return 1; }
    };
    // Counter lives outside Derived: with a member, GCC -O3 speculatively
    // devirtualizes ~Derived into Base deletions and warns (-Warray-bounds).
    int* g_DerivedDestroyed = nullptr;
    struct Derived : Base
    {
        explicit Derived(int* flag) { g_DerivedDestroyed = flag; }
        ~Derived() override { ++*g_DerivedDestroyed; }
        int Kind() const override { return 2; }
    };

    struct WithHook : RefCounted<WithHook>
    {
        void WillBeDestroyed() { ++g_WillDestroyed; }
    };
} // namespace

// Regression: RefCounted started at 0 and Unref() never deleted, so
// CreateRef() produced RefCount()==0 and every object leaked.
TEST_CASE(TestRefLastReleaseDeletes){
    g_Alive = 0;
    {
        auto a = CreateRef<Base>();
        EXPECT(a.RefCount() == 1 && g_Alive == 1);
        {
            auto b = a;
            EXPECT_EQ(a.RefCount(), 2);
        }
        EXPECT(a.RefCount() == 1 && g_Alive == 1);
    }
    EXPECT_EQ(g_Alive, 0);
}

TEST_CASE(TestRefDerivedThroughBase){
    g_Alive              = 0;
    int derivedDestroyed = 0;
    {
        Ref<Base> base = CreateRef<Derived>(&derivedDestroyed);
        EXPECT(base->Kind() == 2 && base.RefCount() == 1);

        Ref<Derived> back = base.As<Derived>();
        EXPECT_EQ(back.RefCount(), 2);
        Ref<Base> moved = Move(back);
        EXPECT(!back && moved.RefCount() == 2);
    }
    // The virtual destructor has to run the derived destructor too.
    EXPECT(derivedDestroyed == 1 && g_Alive == 0);
}

TEST_CASE(TestRefWillBeDestroyed){
    g_WillDestroyed = 0;
    {
        auto ref = CreateRef<WithHook>();
        auto copy = ref;
    }
    EXPECT_EQ(g_WillDestroyed, 1);
}

// Regression: AssignIfNull() assigned even when the Ref already held an
// object.
TEST_CASE(TestRefAssignIfNull){
    g_Alive = 0;
    {
        Ref<Base> empty;
        auto      first = CreateRef<Base>();
        EXPECT(empty.AssignIfNull(Move(first)));
        EXPECT(empty && !first);

        auto second = CreateRef<Base>();
        Base* held  = empty.Raw();
        EXPECT(!empty.AssignIfNull(Move(second)));
        EXPECT(empty.Raw() == held && second); // untouched
        EXPECT(!empty.AssignIfNull(Move(empty)));
        EXPECT_EQ(empty.Raw(), held);
    }
    EXPECT_EQ(g_Alive, 0);
}

TEST_CASE(TestRefSwapAndAssign){
    g_Alive = 0;
    {
        auto a = CreateRef<Base>();
        auto b = CreateRef<Base>();
        Base* rawA = a.Raw();
        Base* rawB = b.Raw();

        Swap(a, b);
        EXPECT(a.Raw() == rawB && b.Raw() == rawA);

        a = Alias(a); // self-assignment keeps the object alive
        EXPECT(a.RefCount() == 1 && g_Alive == 2);
        a = Move(Alias(a));
        EXPECT(a.Raw() == rawB && g_Alive == 2);

        a = b;
        EXPECT(g_Alive == 1 && a.RefCount() == 2);
        a.Reset();
        b = nullptr;
        EXPECT_EQ(g_Alive, 0);
    }
}

TEST_CASE(TestRefFromThis){
    g_Alive = 0;
    {
        auto       ref = CreateRef<Base>();
        Ref<Base>  self = ref->RefFromThis();
        EXPECT(self == ref && ref.RefCount() == 2);
        const Base& constRef = *ref;
        Ref<const Base> constSelf = constRef.RefFromThis();
        EXPECT_EQ(ref.RefCount(), 3);
    }
    EXPECT_EQ(g_Alive, 0);
}

TEST_CASE(TestRefLeakAndAdopt){
    g_Alive = 0;
    auto  ref = CreateRef<Base>();
    Base* raw = ref.Leak();
    EXPECT(!ref && raw->RefCount() == 1 && g_Alive == 1);
    {
        Ref<Base> adopted(AdoptRef, raw);
        EXPECT_EQ(adopted.RefCount(), 1);
    }
    EXPECT_EQ(g_Alive, 0);
    EXPECT(!AdoptRefIfNotNonnull<Base>(nullptr));
}


