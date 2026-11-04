/*
 * Created by v1tr10l7 on 09.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */

#include <Prism/Memory/Ref.hpp>
#include <cassert>
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

void TestRefBasic()
{
    auto ref1 = Prism::CreateRef<Dummy>();
    assert(ref1->value == 42);
    assert(ref1->RefCount() == 1);

    Prism::Ref<Dummy> ref2 = ref1;
    assert(ref1->RefCount() == 2);
    assert(ref2->RefCount() == 2);

    Prism::Ref<Dummy> ref3;
    ref3 = ref1;
    assert(ref3->RefCount() == 3);

    ref2 = nullptr;
    assert(ref1->RefCount() == 2);

    ref3.Reset();
    assert(ref1->RefCount() == 1);
}

void TestRefMove()
{
    auto ref1 = Prism::CreateRef<Dummy>();
    assert(ref1->RefCount() == 1);

    Prism::Ref<Dummy> ref2 = Move(ref1);
    assert(ref1.Raw() == nullptr);
    assert(ref2->value == 42);
    assert(ref2->RefCount() == 1);
}

void TestRefDeleteOnZero()
{
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
        assert(!deleted);
    }
    assert(deleted);
}

void TestRefCreateHelper()
{
    auto ref = Prism::CreateRef<Dummy>();
    assert(ref->value == 42);
    assert(ref.RefCount() == 1);
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
void TestRefLastReleaseDeletes()
{
    g_Alive = 0;
    {
        auto a = CreateRef<Base>();
        Check(a.RefCount() == 1 && g_Alive == 1);
        {
            auto b = a;
            Check(a.RefCount() == 2);
        }
        Check(a.RefCount() == 1 && g_Alive == 1);
    }
    Check(g_Alive == 0);
}

void TestRefDerivedThroughBase()
{
    g_Alive              = 0;
    int derivedDestroyed = 0;
    {
        Ref<Base> base = CreateRef<Derived>(&derivedDestroyed);
        Check(base->Kind() == 2 && base.RefCount() == 1);

        Ref<Derived> back = base.As<Derived>();
        Check(back.RefCount() == 2);
        Ref<Base> moved = Move(back);
        Check(!back && moved.RefCount() == 2);
    }
    // The virtual destructor has to run the derived destructor too.
    Check(derivedDestroyed == 1 && g_Alive == 0);
}

void TestRefWillBeDestroyed()
{
    g_WillDestroyed = 0;
    {
        auto ref = CreateRef<WithHook>();
        auto copy = ref;
    }
    Check(g_WillDestroyed == 1);
}

// Regression: AssignIfNull() assigned even when the Ref already held an
// object.
void TestRefAssignIfNull()
{
    g_Alive = 0;
    {
        Ref<Base> empty;
        auto      first = CreateRef<Base>();
        Check(empty.AssignIfNull(Move(first)));
        Check(empty && !first);

        auto second = CreateRef<Base>();
        Base* held  = empty.Raw();
        Check(!empty.AssignIfNull(Move(second)));
        Check(empty.Raw() == held && second); // untouched
        Check(!empty.AssignIfNull(Move(empty)));
        Check(empty.Raw() == held);
    }
    Check(g_Alive == 0);
}

void TestRefSwapAndAssign()
{
    g_Alive = 0;
    {
        auto a = CreateRef<Base>();
        auto b = CreateRef<Base>();
        Base* rawA = a.Raw();
        Base* rawB = b.Raw();

        Swap(a, b);
        Check(a.Raw() == rawB && b.Raw() == rawA);

        a = Alias(a); // self-assignment keeps the object alive
        Check(a.RefCount() == 1 && g_Alive == 2);
        a = Move(Alias(a));
        Check(a.Raw() == rawB && g_Alive == 2);

        a = b;
        Check(g_Alive == 1 && a.RefCount() == 2);
        a.Reset();
        b = nullptr;
        Check(g_Alive == 0);
    }
}

void TestRefFromThis()
{
    g_Alive = 0;
    {
        auto       ref = CreateRef<Base>();
        Ref<Base>  self = ref->RefFromThis();
        Check(self == ref && ref.RefCount() == 2);
        const Base& constRef = *ref;
        Ref<const Base> constSelf = constRef.RefFromThis();
        Check(ref.RefCount() == 3);
    }
    Check(g_Alive == 0);
}

void TestRefLeakAndAdopt()
{
    g_Alive = 0;
    auto  ref = CreateRef<Base>();
    Base* raw = ref.Leak();
    Check(!ref && raw->RefCount() == 1 && g_Alive == 1);
    {
        Ref<Base> adopted(AdoptRef, raw);
        Check(adopted.RefCount() == 1);
    }
    Check(g_Alive == 0);
    Check(!AdoptRefIfNotNonnull<Base>(nullptr));
}

int main()
{
    TestRefBasic();
    TestRefMove();
    TestRefDeleteOnZero();
    TestRefCreateHelper();
    TestRefLastReleaseDeletes();
    TestRefDerivedThroughBase();
    TestRefWillBeDestroyed();
    TestRefAssignIfNull();
    TestRefSwapAndAssign();
    TestRefFromThis();
    TestRefLeakAndAdopt();

    std::puts("All Ref tests passed!");
}
