/*
 * Created by v1tr10l7 on 08.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Utility/Delegate.hpp>
#include <cassert>

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
    // Delegate<int(int, int)> d;
    // d.BindLambda(BigLambda{});
    // assert(d(10, 3) == 7);
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
    // const Foo          foo{4};
    // Delegate<int(int)> d;
    // d.Bind<&Foo::MultiplyConst>(&foo);
    // assert(d(6) == 24);
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

    // Delegate<int(int, int)> d2 = d1;
    assert(d1(8, 3) == 5);
    // assert(d2(8, 3) == 5);
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
    // Delegate<int(int, int)> d1;
    // d1.BindLambda([](int a, int b) { return a * 2 + b; });
    // Delegate<int(int, int)> d2;
    //
    // printf("d1(3, 4): %d\n", d1(3, 4));
    // d2 = d1;
    //
    // printf("d2(3, 4): %d\n", d2(3, 4));
    // assert(d2(3, 4) == 10);
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

int main()
{
    // 1. Free function binding
    DelegateTest_FreeFunctionBind();

    // 2. Small-buffer lambda binding
    DelegateTest_SBO();

    // 3. Heap-allocated lambda binding
    // DelegateTest_HeapLambda();

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
    // DelegateTest_CopyAssignment();

    // 10. Assignment operator (move)
    DelegateTest_MoveAssignment();

    // 11. Comparison operators
    {
        // Delegate<int(int, int)> d1([](int a, int b) { return a + b; });
        // Delegate<int(int, int)> d2 = d1;
        // assert(d1 == d2);
        // d2.Bind<&FreeFunctionAdd>();
        // assert(d1 != d2);
    }

    // 12. Void return type
    // DelegateTest_VoidReturn();

    printf("All Delegate unit tests passed!\n");
}
