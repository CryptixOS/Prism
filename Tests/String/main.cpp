/*
 * Created by v1tr10l7 on 20.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/String/String.hpp>
#include <cassert>

using namespace Prism;
using namespace Prism::Literals;

void TestConstruction()
{
    String empty;
    assert(empty.Empty());
    assert(empty.Size() == 0);

    String repeated(5, 'x');
    assert(repeated == "xxxxx"_s);

    const char* raw = "hello";
    String      fromRaw(raw);
    assert(fromRaw == "hello"_s);

    String fromView = "world"_s;
    assert(fromView == "world"_s);
}

void TestIteration()
{
    String      s = "abc"_s;

    std::string built;
    for (char c : s) built += c;

    assert(built == "abc");
}

void TestAccessors()
{
    String s = "hello"_s;

    assert(s.At(0) == 'h');
    assert(s[1] == 'e');
    assert(s.Front() == 'h');
    assert(s.Back() == 'o');
}

void TestFind()
{
    String s = "hello world"_s;

    assert(s.Find("world") == 6);
    assert(s.Find('o') == 4);
    assert(s.RFind('o') == 7);
}

void TestComparisons()
{
    String a = "abc"_s;
    String b = "abc"_s;
    String c = "abd"_s;

    assert(a == b);
    assert(a != c);
    assert((a <=> c) == std::strong_ordering::less);
}

void TestSubstringAndResize()
{
    String s   = "abcdef"_s;

    String sub = s.Substr(1, 3);
    assert(sub == "bcd"_s);

    s.Resize(3);
    assert(s == "abc"_s);

    s.Resize(5, 'x');
    assert(s == "abcxx"_s);
}

void TestCopyAndSwap()
{
    String a = "abc"_s;
    String b = "xyz"_s;

    a.Swap(b);
    assert(a == "xyz"_s);
    assert(b == "abc"_s);

    a = b;
    assert(a == "abc"_s);
}

int main()
{
    TestConstruction();
    TestIteration();
    TestAccessors();
    TestFind();
    TestComparisons();
    // TODO: TestSubstringAndResize();
    TestCopyAndSwap();

    return 0;
}
