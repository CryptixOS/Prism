/*
 * Created by v1tr10l7 on 04.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Memory/Scope.hpp>

#include <cstdio>
#include <cstdlib>

// Unlike EXPECT(), this still fires when NDEBUG is defined (release builds).

// Hides self-assignment from -Wself-move / -Wself-assign.
template <typename T>
T& Alias(T& value)
{
    return value;
}

using namespace Prism;

namespace
{
    int g_Alive = 0;
    struct Tracked
    {
        int Value;
        explicit Tracked(int v = 0)
            : Value(v)
        {
            ++g_Alive;
        }
        virtual ~Tracked() { --g_Alive; }
    };
    struct DerivedTracked : Tracked
    {
        using Tracked::Tracked;
    };

    struct CountingDeleter
    {
        int* Calls = nullptr;
        void operator()(Tracked* ptr) const
        {
            ++*Calls;
            delete ptr;
        }
    };

    TEST_CASE(DeletesOnDestruction){
        g_Alive = 0;
        {
            auto scope = CreateScope<Tracked>(3);
            EXPECT(scope && scope->Value == 3 && (*scope).Value == 3);
            EXPECT_EQ(g_Alive, 1);
        }
        EXPECT_EQ(g_Alive, 0);

        Scope<Tracked> empty;
        EXPECT(!empty && empty == nullptr);
    }

    // Regression: move assignment overwrote the pointer without deleting the
    // old object (leak), and self-move nulled the Scope.
    TEST_CASE(MoveAssignmentReleasesOld){
        g_Alive = 0;
        {
            auto a = CreateScope<Tracked>(1);
            auto b = CreateScope<Tracked>(2);
            a      = Move(b);
            EXPECT(g_Alive == 1 && a->Value == 2 && !b);

            a = Move(Alias(a));
            EXPECT(a && a->Value == 2);

            Scope<Tracked> constructed(Move(a));
            EXPECT(constructed && !a && g_Alive == 1);
        }
        EXPECT_EQ(g_Alive, 0);
    }

    TEST_CASE(ConvertingMove){
        g_Alive = 0;
        {
            Scope<DerivedTracked> derived = CreateScope<DerivedTracked>(5);
            Scope<Tracked>        base(Move(derived));
            EXPECT(base->Value == 5 && !derived);

            Scope<Tracked> assigned;
            assigned = CreateScope<DerivedTracked>(6);
            EXPECT(assigned->Value == 6 && g_Alive == 2);
        }
        EXPECT_EQ(g_Alive, 0);
    }

    // Regression: the deleter passed to the constructor was ignored (a fresh
    // Deleter{} was used on Reset), and GetDeleter() returned a dangling
    // reference to a temporary.
    TEST_CASE(StatefulDeleterIsUsed){
        g_Alive   = 0;
        int calls = 0;
        {
            Scope<Tracked, CountingDeleter> scope(new Tracked(1),
                                                  CountingDeleter{&calls});
            EXPECT_EQ(scope.GetDeleter().Calls, (&calls));

            scope.Reset(new Tracked(2));
            EXPECT(calls == 1 && g_Alive == 1);

            Scope<Tracked, CountingDeleter> moved(Move(scope));
            EXPECT_EQ(moved.GetDeleter().Calls, (&calls));
        }
        EXPECT(calls == 2 && g_Alive == 0);
    }

    TEST_CASE(ReleaseAndReset){
        g_Alive    = 0;
        auto scope = CreateScope<Tracked>(1);
        Tracked* raw = scope.Release();
        EXPECT(!scope && g_Alive == 1);
        scope.Reset(raw);
        EXPECT_EQ(scope.Raw(), raw);
        scope = nullptr;
        EXPECT_EQ(g_Alive, 0);

        Scope<Tracked> other(new Tracked(9));
        Scope<Tracked> swapped;
        swapped.Swap(other);
        EXPECT(!other && swapped->Value == 9);
    }

    TEST_CASE(ArraysUseDeleteArray){
        g_Alive = 0;
        {
            auto array = CreateScope<Tracked[]>(4);
            EXPECT_EQ(g_Alive, 4);
            array[2].Value = 7;
            EXPECT_EQ(array[2].Value, 7);

            Scope<Tracked[]> moved(Move(array));
            EXPECT(!array && moved[2].Value == 7);

            auto other = CreateScope<Tracked[]>(2);
            moved      = Move(other);
            EXPECT_EQ(g_Alive, 2);

            moved = Move(Alias(moved));
            EXPECT(moved && g_Alive == 2);
        }
        EXPECT_EQ(g_Alive, 0);
    }
} // namespace


