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
#include <Prism/Memory/WeakRef.hpp>
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

    void WeakRefFollowsLifetime()
    {
        g_Alive = 0;
        WeakRef<Object> weak;
        Check(weak.IsNull() && !weak && !weak.Promote());
        {
            auto strong = CreateRef<Object>(42);
            weak        = WeakRef<Object>(strong);
            Check(!weak.IsNull() && weak && weak->Value == 42);
            Check(strong.RefCount() == 1); // weak refs don't own

            auto promoted = weak.Promote();
            Check(promoted && promoted.RefCount() == 2);
        }
        Check(g_Alive == 0);
        Check(weak.IsNull() && !weak && weak.Raw() == nullptr);
        Check(!weak.Promote());
    }

    // Regression: weak refs were revoked by ~Weakable, which runs *after* the
    // derived destructor, so a WeakRef could reach a half-destroyed object.
    void WeakRefRevokedBeforeDestructor()
    {
        g_Alive               = 0;
        g_WasNullInDestructor = false;
        WeakRef<Object> weak;
        {
            auto strong              = CreateRef<Object>(1);
            weak                     = WeakRef<Object>(strong);
            g_ObservedFromDestructor = &weak;
        }
        g_ObservedFromDestructor = nullptr;
        Check(g_WasNullInDestructor);
        Check(weak.IsNull());
    }

    // Regression: copying a Weakable copied its link, so WeakRefs made from
    // the copy pointed at the original, and destroying the copy revoked the
    // original's weak refs.
    void CopiedWeakableGetsItsOwnLink()
    {
        Plain original;
        original.Value = 1;
        WeakRef<Plain> toOriginal(original);
        {
            Plain copy = original;
            copy.Value = 2;
            WeakRef<Plain> toCopy(copy);
            Check(toCopy->Value == 2);
            Check(toOriginal->Value == 1);
        }
        Check(!toOriginal.IsNull() && toOriginal->Value == 1);

        Plain assigned;
        assigned = original;
        Check(!toOriginal.IsNull());
    }

    void PlainObjectRevokesOnDestruction()
    {
        WeakRef<Plain> weak;
        {
            Plain stackObject;
            weak = WeakRef<Plain>(&stackObject);
            Check(weak && weak->Value == 5);
            WeakRef<Plain> copy = weak;
            Check(copy.Raw() == &stackObject);
        }
        Check(weak.IsNull());
        weak = nullptr;
        Check(weak.IsNull());
    }
} // namespace

int main()
{
    WeakRefFollowsLifetime();
    WeakRefRevokedBeforeDestructor();
    CopiedWeakableGetsItsOwnLink();
    PlainObjectRevokesOnDestruction();

    std::puts("All WeakRef tests passed!");
    return 0;
}
