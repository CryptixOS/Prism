/*
 * Created by v1tr10l7 on 04.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Memory/Scope.hpp>

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

    void DeletesOnDestruction()
    {
        g_Alive = 0;
        {
            auto scope = CreateScope<Tracked>(3);
            Check(scope && scope->Value == 3 && (*scope).Value == 3);
            Check(g_Alive == 1);
        }
        Check(g_Alive == 0);

        Scope<Tracked> empty;
        Check(!empty && empty == nullptr);
    }

    // Regression: move assignment overwrote the pointer without deleting the
    // old object (leak), and self-move nulled the Scope.
    void MoveAssignmentReleasesOld()
    {
        g_Alive = 0;
        {
            auto a = CreateScope<Tracked>(1);
            auto b = CreateScope<Tracked>(2);
            a      = Move(b);
            Check(g_Alive == 1 && a->Value == 2 && !b);

            a = Move(Alias(a));
            Check(a && a->Value == 2);

            Scope<Tracked> constructed(Move(a));
            Check(constructed && !a && g_Alive == 1);
        }
        Check(g_Alive == 0);
    }

    void ConvertingMove()
    {
        g_Alive = 0;
        {
            Scope<DerivedTracked> derived = CreateScope<DerivedTracked>(5);
            Scope<Tracked>        base(Move(derived));
            Check(base->Value == 5 && !derived);

            Scope<Tracked> assigned;
            assigned = CreateScope<DerivedTracked>(6);
            Check(assigned->Value == 6 && g_Alive == 2);
        }
        Check(g_Alive == 0);
    }

    // Regression: the deleter passed to the constructor was ignored (a fresh
    // Deleter{} was used on Reset), and GetDeleter() returned a dangling
    // reference to a temporary.
    void StatefulDeleterIsUsed()
    {
        g_Alive   = 0;
        int calls = 0;
        {
            Scope<Tracked, CountingDeleter> scope(new Tracked(1),
                                                  CountingDeleter{&calls});
            Check(scope.GetDeleter().Calls == &calls);

            scope.Reset(new Tracked(2));
            Check(calls == 1 && g_Alive == 1);

            Scope<Tracked, CountingDeleter> moved(Move(scope));
            Check(moved.GetDeleter().Calls == &calls);
        }
        Check(calls == 2 && g_Alive == 0);
    }

    void ReleaseAndReset()
    {
        g_Alive    = 0;
        auto scope = CreateScope<Tracked>(1);
        Tracked* raw = scope.Release();
        Check(!scope && g_Alive == 1);
        scope.Reset(raw);
        Check(scope.Raw() == raw);
        scope = nullptr;
        Check(g_Alive == 0);

        Scope<Tracked> other(new Tracked(9));
        Scope<Tracked> swapped;
        swapped.Swap(other);
        Check(!other && swapped->Value == 9);
    }

    void ArraysUseDeleteArray()
    {
        g_Alive = 0;
        {
            auto array = CreateScope<Tracked[]>(4);
            Check(g_Alive == 4);
            array[2].Value = 7;
            Check(array[2].Value == 7);

            Scope<Tracked[]> moved(Move(array));
            Check(!array && moved[2].Value == 7);

            auto other = CreateScope<Tracked[]>(2);
            moved      = Move(other);
            Check(g_Alive == 2);

            moved = Move(Alias(moved));
            Check(moved && g_Alive == 2);
        }
        Check(g_Alive == 0);
    }
} // namespace

int main()
{
    DeletesOnDestruction();
    MoveAssignmentReleasesOld();
    ConvertingMove();
    StatefulDeleterIsUsed();
    ReleaseAndReset();
    ArraysUseDeleteArray();

    std::puts("All Scope tests passed!");
    return 0;
}
