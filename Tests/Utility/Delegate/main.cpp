/*
 * Created by v1tr10l7 on 08.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Memory/Scope.hpp>
#include <Prism/Utility/Delegate.hpp>
#include <cassert>
#include <cstdio>
#include <cstdlib>

// Unlike assert(), this still fires when NDEBUG is defined (release builds).
#define Check(expr)                                                            \
    do {                                                                       \
        if (!(expr))                                                           \
        {                                                                      \
            std::fprintf(stderr, "%s:%d: Check failed: %s\n", __FILE__,      \
                         __LINE__, #expr);                                     \
            std::abort();                                                      \
        }                                                                      \
    } while (0)

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

void DelegateTest_FreeFunctionBind()
{
    Delegate<int(int, int)> d;
    d.Bind<&FreeFunctionAdd>();
    assert(d(2, 3) == 5);
}
void DelegateTest_SBO()
{
    Delegate<int(int, int)> d([](int a, int b) { return a * b; });

    assert(d(4, 5) == 20);
}
void DelegateTest_HeapLambda()
{
    Delegate<int(int, int)> d;
    d.BindLambda(BigLambda{});
    Check(!d.IsInline());
    Check(d(10, 3) == 7);
}
void DelegateTest_MemberFunctionBind()
{
    Foo                foo{3};
    Delegate<int(int)> d;
    d.Bind<&Foo::Multiply>(&foo);
    assert(d(5) == 15);
}
void DelegateTest_ConstMemberFunctionBind()
{
    const Foo          foo{4};
    Delegate<int(int)> d;
    d.Bind<&Foo::MultiplyConst>(&foo);
    Check(d(6) == 24);
}
void DelegateTest_RebindAndReset()
{
    Delegate<int(int, int)> d;
    d.BindLambda([](int a, int b) { return a + b; });
    assert(d(2, 2) == 4);
    d.Bind<&FreeFunctionAdd>();
    assert(d(3, 7) == 10);
    d.Reset();
    assert(!d.IsBound());
}
void DelegateTest_CopyConstructor()
{
    Delegate<int(int, int)> d1;
    d1.BindLambda([](int a, int b) { return a - b; });

    Delegate<int(int, int)> d2 = d1;
    Check(d1(8, 3) == 5);
    Check(d2(8, 3) == 5);
}
void DelegateTest_MoveConstructor()
{
    Delegate<int(int, int)> d1([](int a, int b) { return a / b; });
    d1.BindLambda([](int a, int b) { return a / b; });
    Delegate<int(int, int)> d2 = std::move(d1);
    assert(d2(8, 2) == 4);
}
void DelegateTest_CopyAssignment()
{
    Delegate<int(int, int)> d1;
    d1.BindLambda([](int a, int b) { return a * 2 + b; });
    Delegate<int(int, int)> d2;
    d2 = d1;
    Check(d1(3, 4) == 10);
    Check(d2(3, 4) == 10);
}
void DelegateTest_MoveAssignment()
{
    Delegate<int(int, int)> d1([](int a, int b) { return a + 100 + b; });
    Delegate<int(int, int)> d2;
    d2 = Move(d1);
    assert(d2(1, 1) == 102);
}
void DelegateTest_VoidReturn()
{
    bool                               flag = false;
    Delegate<void(int, int, int, int)> d;
    d.BindLambda([&](int, int, int, int) { flag = true; });

    d(1, 1, 1, 1);
    assert(flag);
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
void DelegateTest_LifetimeBalanced()
{
    g_Alive  = 0;
    int hits = 0;
    {
        Delegate<int(int)> inlined(Counter{&hits});
        Check(inlined.IsInline());
        Check(g_Alive == 1);
        Check(inlined(2) == 2);

        Delegate<int(int)> heap(BigCounter{&hits});
        Check(!heap.IsInline());
        Check(g_Alive == 2);
        Check(heap(3) == 5);
    }
    Check(g_Alive == 0);
}

// Regression: copying memcpy'd the inline buffer, which breaks callables
// that aren't trivially copyable (two owners, one destroyed twice).
void DelegateTest_CopyNonTrivialCallable()
{
    g_Alive  = 0;
    int hits = 0;
    {
        Delegate<int(int)> a(Counter{&hits});
        Delegate<int(int)> b = a;
        Check(g_Alive == 2);
        Check(a(1) == 1 && b(1) == 2);

        Delegate<int(int)> heap(BigCounter{&hits});
        Delegate<int(int)> heapCopy = heap;
        Check(g_Alive == 4);
        heap.Reset();
        Check(heapCopy(1) == 3); // the copy has its own allocation
        Check(g_Alive == 3);

        b = heapCopy;
        Check(g_Alive == 3);
        b = Alias(b);
        Check(b(0) == 3 && g_Alive == 3);
    }
    Check(g_Alive == 0);
}

void DelegateTest_MoveTransfers()
{
    g_Alive  = 0;
    int hits = 0;
    {
        Delegate<int(int)> a(Counter{&hits});
        Delegate<int(int)> b(Move(a));
        Check(!a && b && g_Alive == 1);
        Check(b(4) == 4);

        Delegate<int(int)> heap(BigCounter{&hits});
        Delegate<int(int)> c;
        c = Move(heap);
        Check(!heap && c(1) == 5 && g_Alive == 2);

        c = Move(Alias(c));
        Check(c && c(0) == 5);
    }
    Check(g_Alive == 0);
}

void DelegateTest_RebindReleasesPrevious()
{
    g_Alive  = 0;
    int hits = 0;
    Delegate<int(int, int)> d;
    d.BindLambda([counter = Counter{&hits}](int a, int b) { return a + b; });
    Check(g_Alive == 1);
    d.Bind<&FreeFunctionAdd>();
    Check(g_Alive == 0 && d(1, 2) == 3);
    d.BindLambda([counter = BigCounter{&hits}](int a, int) { return a; });
    Check(g_Alive == 1);
    d = nullptr;
    Check(g_Alive == 0 && !d.IsBound());
}

// Regression: operator=(NullType) had no return statement, and
// operator=(function pointer) built a temporary instead of assigning.
void DelegateTest_AssignOperators()
{
    Delegate<int(int, int)> d;
    Delegate<int(int, int)>& self = (d = nullptr);
    Check(&self == &d);

    d = &FreeFunctionAdd;
    Check(d.IsBound() && d(20, 22) == 42);

    int (*fn)(int, int) = &FreeFunctionAdd;
    d                   = fn;
    Check(d(1, 1) == 2);

    d = [](int a, int b) { return a * b; };
    Check(d(6, 7) == 42);

    Delegate<int(int, int)> fromPointer(&FreeFunctionAdd);
    Check(fromPointer(2, 2) == 4);
}

// Regression: Swap() called itself recursively (member Swap hid the free
// function) and didn't handle callables stored inline.
void DelegateTest_Swap()
{
    g_Alive  = 0;
    int hits = 0;
    {
        Delegate<int(int)> a(Counter{&hits});
        Delegate<int(int)> b(BigCounter{&hits});
        hits = 10;
        a.Swap(b);
        Check(!a.IsInline() && b.IsInline());
        Check(a(1) == 11 && b(1) == 12);
        Check(g_Alive == 2);

        Delegate<int(int)> empty;
        empty.Swap(b);
        Check(empty && !b && g_Alive == 2);
    }
    Check(g_Alive == 0);
}

void DelegateTest_MemberOfBaseClass()
{
    Derived            derived;
    Delegate<int(int)> d;
    d.Bind<&Base::Add>(&derived);
    Check(d(1) == 101);

    auto fromMethod = Delegate<int(int)>::FromMethod<&Base::Add>(&derived);
    Check(fromMethod(2) == 102);
    auto fromFunction
        = Delegate<int(int, int)>::FromFunction<&FreeFunctionAdd>();
    Check(fromFunction(2, 3) == 5);
}

void DelegateTest_OverAlignedGoesToHeap()
{
    Delegate<int(int)> d(OverAligned{});
    Check(!d.IsInline());
    Check(d(0) == 11);
}

void DelegateTest_ConstInvoke()
{
    const Delegate<int(int, int)> d([](int a, int b) { return a - b; });
    Check(d(5, 3) == 2);
    Check(d.Invoke(9, 3) == 6);
}

void DelegateTest_MoveOnlyCallable()
{
    struct MoveOnly
    {
        Scope<int> Value;
        int        operator()() const { return *Value; }
    };
    Delegate<int()> d(MoveOnly{CreateScope<int>(5)});
    Check(d() == 5);
    Delegate<int()> moved = Move(d);
    Check(moved() == 5 && !d);
}

int main()
{
    // 1. Free function binding
    DelegateTest_FreeFunctionBind();

    // 2. Small-buffer lambda binding
    DelegateTest_SBO();

    // 3. Heap-allocated lambda binding
    DelegateTest_HeapLambda();

    // 4. Member function binding (non-const)
    DelegateTest_MemberFunctionBind();

    // 5. Member function binding (const)
    DelegateTest_ConstMemberFunctionBind();

    // 6. Rebinding & Reset
    DelegateTest_RebindAndReset();

    // 7. Copy constructor
    DelegateTest_CopyConstructor();

    // 8. Move constructor
    DelegateTest_MoveConstructor();

    // 9. Assignment operator (copy)
    DelegateTest_CopyAssignment();

    // 10. Assignment operator (move)
    DelegateTest_MoveAssignment();

    // 11. Comparison operators
    {
        Foo                foo{2};
        Delegate<int(int)> d1;
        d1.Bind<&Foo::Multiply>(&foo);
        Delegate<int(int)> d2 = d1;
        Check(d1 == d2);
        d2.Bind<&Foo::MultiplyConst>(&foo);
        Check(d1 != d2);

        Delegate<int(int, int)> f1, f2;
        f1.Bind<&FreeFunctionAdd>();
        f2.Bind<&FreeFunctionAdd>();
        Check(f1 == f2);
    }

    // 12. Void return type
    DelegateTest_VoidReturn();

    // 13. Lifetime / ownership regressions
    DelegateTest_LifetimeBalanced();
    DelegateTest_CopyNonTrivialCallable();
    DelegateTest_MoveTransfers();
    DelegateTest_RebindReleasesPrevious();
    DelegateTest_AssignOperators();
    DelegateTest_Swap();
    DelegateTest_MemberOfBaseClass();
    DelegateTest_OverAlignedGoesToHeap();
    DelegateTest_ConstInvoke();
    DelegateTest_MoveOnlyCallable();

    printf("All Delegate unit tests passed!\n");
}
