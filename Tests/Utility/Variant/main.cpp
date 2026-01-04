/*
 * Created by v1tr10l7 on 09.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Debug/Log.hpp>
#include <Prism/Utility/Variant.hpp>
using namespace Prism;

// Test types
struct A
{
    i32  x = 0;
    bool operator==(const A& o) const { return x == o.x; }
};

struct B
{
    String s;
    bool   operator==(const B& o) const { return s == o.s; }
};

struct NonTrivial
{
    NonTrivial(i32 v)
        : val(v)
    {
    }
    NonTrivial(const NonTrivial& o)
        : val(o.val)
    {
    }
    NonTrivial(NonTrivial&& o) noexcept
        : val(o.val)
    {
        o.val = 0;
    }
    NonTrivial& operator=(const NonTrivial& o)
    {
        val = o.val;
        return *this;
    }
    NonTrivial& operator=(NonTrivial&& o) noexcept
    {
        val   = o.val;
        o.val = 0;
        return *this;
    }
    i32  val;
    bool operator==(const NonTrivial& o) const { return val == o.val; }
};

void TestConstructionAndHas()
{
    Prism::Variant<i32, f64, A> v1{42};
    assert(v1.Has<i32>());
    assert(!v1.Has<f64>());
    assert(!v1.Has<A>());
    assert(v1.Get<i32>() == 42);

    Prism::Variant<i32, f64, A> v2{A{123}};
    assert(v2.Has<A>());
    assert(v2.Get<A>().x == 123);
}

void TestSetAndGet()
{
    Prism::Variant<i32, String> v{123};
    assert(v.Has<i32>());
    v.Set(String("hello"));
    assert(v.Has<String>());
    assert(v.Get<String>() == "hello"_sv);

    v.Set(456);
    assert(v.Has<i32>());
    assert(v.Get<i32>() == 456);
}

void TestCopyAndMove()
{
    Prism::Variant<NonTrivial, i32> v1{NonTrivial{10}};
    Prism::Variant<NonTrivial, i32> v2{v1}; // copy ctor
    assert(v2.Has<NonTrivial>());
    assert(v2.Get<NonTrivial>().val == 10);

    Prism::Variant<NonTrivial, i32> v3{Move(v1)}; // move ctor
    assert(v3.Has<NonTrivial>());
    assert(v3.Get<NonTrivial>().val == 10);
    // v1 still contains valid but moved-from state
    assert(v1.Has<NonTrivial>());

    v2 = v3; // copy assign
    assert(v2.Has<NonTrivial>());
    assert(v2.Get<NonTrivial>().val == 10);

    v3 = std::move(v2); // move assign
    assert(v3.Has<NonTrivial>());
    assert(v3.Get<NonTrivial>().val == 10);
}

void TestEquality()
{
    Prism::Variant<i32, A> v1{42};
    Prism::Variant<i32, A> v2{42};
    Prism::Variant<i32, A> v3{A{1}};
    Prism::Variant<i32, A> v4{A{2}};

    assert(v1 == v2);
    assert(!(v1 == v3));
    assert(!(v3 == v4));
}

void TestVisit()
{
    Prism::Variant<i32, String> v{123};

    bool                        visitedInt    = false;
    bool                        visitedString = false;

    v.Visit([&](i32 i) { visitedInt = (i == 123); },
            [&](const String&) { visitedString = true; });
    assert(visitedInt);
    assert(!visitedString);

    v.Set(String("test"));
    v.Visit([&](i32) { visitedInt = true; },
            [&](const String& s) { visitedString = (s == "test"); });
    assert(visitedString);
}

i32 main()
{
    TestConstructionAndHas();
    TestSetAndGet();
    TestCopyAndMove();
    TestEquality();
    TestVisit();

    PrismInfo("All Variant tests passed.\n");
    return 0;
}
