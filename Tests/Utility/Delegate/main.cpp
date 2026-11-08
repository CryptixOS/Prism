/*
 * Created by v1tr10l7 on 08.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Memory/Scope.hpp>
#include <Prism/Utility/Delegate.hpp>
#include <cstdio>
#include <cstdlib>

// Unlike EXPECT(), this still fires when NDEBUG is defined (release builds).

template <typename T>
T& Alias(T& value)
{
    return value;
}

using namespace Prism;

//
// 1. Free function binding
//
static int FreeFunctionAdd(int a, int b) { return a + b; }

//
// 2. Test object with member functions
//
struct Foo
{
    int factor = 10;
    int Multiply(int x) { return x * factor; }
    int MultiplyConst(int x) const { return x * factor; }
};

//
// 3. Heap-heavy lambda
//
struct BigLambda
{
    char padding[128]; // Force heap allocation in BindLambda
    int  operator()(int a, int b) const { return a - b; }
};

TEST_CASE(DelegateTest_FreeFunctionBind)
{
    Delegate<int(int, int)> d;
    d.Bind<&FreeFunctionAdd>();
    EXPECT_EQ((d(2, 3)), 5);
}
TEST_CASE(DelegateTest_SBO)
{
    Delegate<int(int, int)> d([](int a, int b) { return a * b; });

    EXPECT_EQ((d(4, 5)), 20);
}
TEST_CASE(DelegateTest_HeapLambda)
{
    Delegate<int(int, int)> d;
    d.BindLambda(BigLambda{});
    EXPECT(!d.IsInline());
    EXPECT_EQ((d(10, 3)), 7);
}
TEST_CASE(DelegateTest_MemberFunctionBind)
{
    Foo                foo{3};
    Delegate<int(int)> d;
    d.Bind<&Foo::Multiply>(&foo);
    EXPECT_EQ(d(5), 15);
}
TEST_CASE(DelegateTest_ConstMemberFunctionBind)
{
    const Foo          foo{4};
    Delegate<int(int)> d;
    d.Bind<&Foo::MultiplyConst>(&foo);
    EXPECT_EQ(d(6), 24);
}
TEST_CASE(DelegateTest_RebindAndReset)
{
    Delegate<int(int, int)> d;
    d.BindLambda([](int a, int b) { return a + b; });
    EXPECT_EQ((d(2, 2)), 4);
    d.Bind<&FreeFunctionAdd>();
    EXPECT_EQ((d(3, 7)), 10);
    d.Reset();
    EXPECT(!d.IsBound());
}
TEST_CASE(DelegateTest_CopyConstructor)
{
    Delegate<int(int, int)> d1;
    d1.BindLambda([](int a, int b) { return a - b; });

    Delegate<int(int, int)> d2 = d1;
    EXPECT_EQ((d1(8, 3)), 5);
    EXPECT_EQ((d2(8, 3)), 5);
}
TEST_CASE(DelegateTest_MoveConstructor)
{
    Delegate<int(int, int)> d1([](int a, int b) { return a / b; });
    d1.BindLambda([](int a, int b) { return a / b; });
    Delegate<int(int, int)> d2 = std::move(d1);
    EXPECT_EQ((d2(8, 2)), 4);
}
TEST_CASE(DelegateTest_CopyAssignment)
{
    Delegate<int(int, int)> d1;
    d1.BindLambda([](int a, int b) { return a * 2 + b; });
    Delegate<int(int, int)> d2;
    d2 = d1;
    EXPECT_EQ((d1(3, 4)), 10);
    EXPECT_EQ((d2(3, 4)), 10);
}
TEST_CASE(DelegateTest_MoveAssignment)
{
    Delegate<int(int, int)> d1([](int a, int b) { return a + 100 + b; });
    Delegate<int(int, int)> d2;
    d2 = Move(d1);
    EXPECT_EQ((d2(1, 1)), 102);
}
TEST_CASE(DelegateTest_VoidReturn)
{
    bool                               flag = false;
    Delegate<void(int, int, int, int)> d;
    d.BindLambda([&](int, int, int, int) { flag = true; });

    d(1, 1, 1, 1);
    EXPECT(flag);
}

namespace
{
    // Counts live instances to catch double destruction and leaks.
    int g_Alive = 0;
    struct Counter
    {
        int* Hits;
        explicit Counter(int* hits)
            : Hits(hits)
        {
            ++g_Alive;
        }
        Counter(const Counter& other)
            : Hits(other.Hits)
        {
            ++g_Alive;
        }
        Counter(Counter&& other) noexcept
            : Hits(other.Hits)
        {
            ++g_Alive;
        }
        ~Counter() { --g_Alive; }
        int operator()(int x) const { return *Hits += x; }
    };
    struct BigCounter : Counter
    {
        using Counter::Counter;
        char Padding[128] = {};
    };
    struct alignas(64) OverAligned
    {
        int Value = 11;
        int operator()(int) const { return Value; }
    };

    struct Base
    {
        int Offset = 100;
        int Add(int x) { return x + Offset; }
    };
    struct Derived : Base
    {
    };
} // namespace

// Regression: Assign() went through a temporary Functor whose destructor
// destroyed the freshly bound callable right away (use-after-free for heap
// lambdas, destructor run twice for inline ones).
TEST_CASE(DelegateTest_LifetimeBalanced)
{
    g_Alive  = 0;
    int hits = 0;
    {
        Delegate<int(int)> inlined(Counter{&hits});
        EXPECT(inlined.IsInline());
        EXPECT_EQ(g_Alive, 1);
        EXPECT_EQ(inlined(2), 2);

        Delegate<int(int)> heap(BigCounter{&hits});
        EXPECT(!heap.IsInline());
        EXPECT_EQ(g_Alive, 2);
        EXPECT_EQ(heap(3), 5);
    }
    EXPECT_EQ(g_Alive, 0);
}

// Regression: copying memcpy'd the inline buffer, which breaks callables
// that aren't trivially copyable (two owners, one destroyed twice).
TEST_CASE(DelegateTest_CopyNonTrivialCallable)
{
    g_Alive  = 0;
    int hits = 0;
    {
        Delegate<int(int)> a(Counter{&hits});
        Delegate<int(int)> b = a;
        EXPECT_EQ(g_Alive, 2);
        EXPECT(a(1) == 1 && b(1) == 2);

        Delegate<int(int)> heap(BigCounter{&hits});
        Delegate<int(int)> heapCopy = heap;
        EXPECT_EQ(g_Alive, 4);
        heap.Reset();
        EXPECT_EQ(heapCopy(1), 3); // the copy has its own allocation
        EXPECT_EQ(g_Alive, 3);

        b = heapCopy;
        EXPECT_EQ(g_Alive, 3);
        b = Alias(b);
        EXPECT(b(0) == 3 && g_Alive == 3);
    }
    EXPECT_EQ(g_Alive, 0);
}

TEST_CASE(DelegateTest_MoveTransfers)
{
    g_Alive  = 0;
    int hits = 0;
    {
        Delegate<int(int)> a(Counter{&hits});
        Delegate<int(int)> b(Move(a));
        EXPECT(!a && b && g_Alive == 1);
        EXPECT_EQ(b(4), 4);

        Delegate<int(int)> heap(BigCounter{&hits});
        Delegate<int(int)> c;
        c = Move(heap);
        EXPECT(!heap && c(1) == 5 && g_Alive == 2);

        c = Move(Alias(c));
        EXPECT(c && c(0) == 5);
    }
    EXPECT_EQ(g_Alive, 0);
}

TEST_CASE(DelegateTest_RebindReleasesPrevious)
{
    g_Alive  = 0;
    int hits = 0;
    Delegate<int(int, int)> d;
    d.BindLambda([counter = Counter{&hits}](int a, int b) { return a + b; });
    EXPECT_EQ(g_Alive, 1);
    d.Bind<&FreeFunctionAdd>();
    EXPECT(g_Alive == 0 && d(1, 2) == 3);
    d.BindLambda([counter = BigCounter{&hits}](int a, int) { return a; });
    EXPECT_EQ(g_Alive, 1);
    d = nullptr;
    EXPECT(g_Alive == 0 && !d.IsBound());
}

// Regression: operator=(NullType) had no return statement, and
// operator=(function pointer) built a temporary instead of assigning.
TEST_CASE(DelegateTest_AssignOperators)
{
    Delegate<int(int, int)> d;
    Delegate<int(int, int)>& self = (d = nullptr);
    EXPECT_EQ((&self), (&d));

    d = &FreeFunctionAdd;
    EXPECT(d.IsBound() && d(20, 22) == 42);

    int (*fn)(int, int) = &FreeFunctionAdd;
    d                   = fn;
    EXPECT_EQ((d(1, 1)), 2);

    d = [](int a, int b) { return a * b; };
    EXPECT_EQ((d(6, 7)), 42);

    Delegate<int(int, int)> fromPointer(&FreeFunctionAdd);
    EXPECT_EQ((fromPointer(2, 2)), 4);
}

// Regression: Swap() called itself recursively (member Swap hid the free
// function) and didn't handle callables stored inline.
TEST_CASE(DelegateTest_Swap)
{
    g_Alive  = 0;
    int hits = 0;
    {
        Delegate<int(int)> a(Counter{&hits});
        Delegate<int(int)> b(BigCounter{&hits});
        hits = 10;
        a.Swap(b);
        EXPECT(!a.IsInline() && b.IsInline());
        EXPECT(a(1) == 11 && b(1) == 12);
        EXPECT_EQ(g_Alive, 2);

        Delegate<int(int)> empty;
        empty.Swap(b);
        EXPECT(empty && !b && g_Alive == 2);
    }
    EXPECT_EQ(g_Alive, 0);
}

TEST_CASE(DelegateTest_MemberOfBaseClass)
{
    Derived            derived;
    Delegate<int(int)> d;
    d.Bind<&Base::Add>(&derived);
    EXPECT_EQ(d(1), 101);

    auto fromMethod = Delegate<int(int)>::FromMethod<&Base::Add>(&derived);
    EXPECT_EQ(fromMethod(2), 102);
    auto fromFunction
        = Delegate<int(int, int)>::FromFunction<&FreeFunctionAdd>();
    EXPECT_EQ((fromFunction(2, 3)), 5);
}

TEST_CASE(DelegateTest_OverAlignedGoesToHeap)
{
    Delegate<int(int)> d(OverAligned{});
    EXPECT(!d.IsInline());
    EXPECT_EQ(d(0), 11);
}

TEST_CASE(DelegateTest_ConstInvoke)
{
    const Delegate<int(int, int)> d([](int a, int b) { return a - b; });
    EXPECT_EQ((d(5, 3)), 2);
    EXPECT_EQ((d.Invoke(9, 3)), 6);
}

TEST_CASE(DelegateTest_MoveOnlyCallable)
{
    struct MoveOnly
    {
        Scope<int> Value;
        int        operator()() const { return *Value; }
    };
    Delegate<int()> d(MoveOnly{CreateScope<int>(5)});
    EXPECT_EQ(d(), 5);
    Delegate<int()> moved = Move(d);
    EXPECT(moved() == 5 && !d);
}

TEST_CASE(DelegateTest_Comparison)
{
    Foo                foo{2};
    Delegate<int(int)> d1;
    d1.Bind<&Foo::Multiply>(&foo);
    Delegate<int(int)> d2 = d1;
    EXPECT_EQ(d1, d2);
    d2.Bind<&Foo::MultiplyConst>(&foo);
    EXPECT_NE(d1, d2);

    Delegate<int(int, int)> f1, f2;
    f1.Bind<&FreeFunctionAdd>();
    f2.Bind<&FreeFunctionAdd>();
    EXPECT_EQ(f1, f2);
}

