/*
 * Created by v1tr10l7 on 04.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Memory/WeakRef.hpp>

#include <cstdio>
#include <cstdlib>

// Unlike EXPECT(), this still fires when NDEBUG is defined (release builds).

using namespace Prism;

namespace
{
    int g_Alive = 0;

    struct Object;
    WeakRef<Object>* g_ObservedFromDestructor = nullptr;
    bool             g_WasNullInDestructor    = false;

    struct Object : RefCounted<Object>, Weakable<Object>
    {
        int Value;
        explicit Object(int v = 0)
            : Value(v)
        {
            ++g_Alive;
        }
        Object(const Object& other)
            : RefCounted<Object>(),
              Weakable<Object>(other),
              Value(other.Value)
        {
            ++g_Alive;
        }
        ~Object()
        {
            if (g_ObservedFromDestructor)
                g_WasNullInDestructor = g_ObservedFromDestructor->IsNull();
            --g_Alive;
        }
    };

    struct Plain : Weakable<Plain>
    {
        int Value = 5;
    };

    TEST_CASE(WeakRefFollowsLifetime){
        g_Alive = 0;
        WeakRef<Object> weak;
        EXPECT(weak.IsNull() && !weak && !weak.Promote());
        {
            auto strong = CreateRef<Object>(42);
            weak        = WeakRef<Object>(strong);
            EXPECT(!weak.IsNull() && weak && weak->Value == 42);
            EXPECT_EQ(strong.RefCount(), 1); // weak refs don't own

            auto promoted = weak.Promote();
            EXPECT(promoted && promoted.RefCount() == 2);
        }
        EXPECT_EQ(g_Alive, 0);
        EXPECT(weak.IsNull() && !weak && weak.Raw() == nullptr);
        EXPECT(!weak.Promote());
    }

    // Regression: weak refs were revoked by ~Weakable, which runs *after* the
    // derived destructor, so a WeakRef could reach a half-destroyed object.
    TEST_CASE(WeakRefRevokedBeforeDestructor){
        g_Alive               = 0;
        g_WasNullInDestructor = false;
        WeakRef<Object> weak;
        {
            auto strong              = CreateRef<Object>(1);
            weak                     = WeakRef<Object>(strong);
            g_ObservedFromDestructor = &weak;
        }
        g_ObservedFromDestructor = nullptr;
        EXPECT(g_WasNullInDestructor);
        EXPECT(weak.IsNull());
    }

    // Regression: copying a Weakable copied its link, so WeakRefs made from
    // the copy pointed at the original, and destroying the copy revoked the
    // original's weak refs.
    TEST_CASE(CopiedWeakableGetsItsOwnLink){
        Plain original;
        original.Value = 1;
        WeakRef<Plain> toOriginal(original);
        {
            Plain copy = original;
            copy.Value = 2;
            WeakRef<Plain> toCopy(copy);
            EXPECT(toCopy->Value == 2);
            EXPECT(toOriginal->Value == 1);
        }
        EXPECT(!toOriginal.IsNull() && toOriginal->Value == 1);

        Plain assigned;
        assigned = original;
        EXPECT(!toOriginal.IsNull());
    }

    TEST_CASE(PlainObjectRevokesOnDestruction){
        WeakRef<Plain> weak;
        {
            Plain stackObject;
            weak = WeakRef<Plain>(&stackObject);
            EXPECT(weak && weak->Value == 5);
            WeakRef<Plain> copy = weak;
            EXPECT_EQ(copy.Raw(), (&stackObject));
        }
        EXPECT(weak.IsNull());
        weak = nullptr;
        EXPECT(weak.IsNull());
    }
} // namespace


