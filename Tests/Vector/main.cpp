/*
 * Created by v1tr10l7 on 20.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Vector.hpp>
#include <cassert>
#include <string>

using namespace Prism;

void TestDefaultConstructor()
{
    Vector<int> v;
    assert(v.Size() == 0);
    assert(v.Empty());
}

void TestPushBackAndAt()
{
    Vector<int> v;
    int         a = 10, b = 20;
    v.PushBack(a);
    v.PushBack(b);
    assert(v.Size() == 2);
    assert(v[0] == 10);
    assert(v.At(1) == 20);
}

void TestFrontBack()
{
    Vector<int> v;
    int         a = 5, b = 15;
    v.PushBack(a);
    v.PushBack(b);
    assert(v.Front() == 5);
    assert(v.Back() == 15);
}

void TestInitializerListConstructor()
{
    Vector<int> v{1, 2, 3};
    assert(v.Size() == 3);
    assert(v[0] == 1);
    assert(v[1] == 2);
    assert(v[2] == 3);
}

void TestCopyConstructor()
{
    Vector<int> a{1, 2, 3};
    Vector<int> b = a;
    assert(b.Size() == 3);
    assert(b[0] == 1 && b[1] == 2 && b[2] == 3);
}

void TestMoveConstructor()
{
    Vector<int> a{4, 5, 6};
    Vector<int> b = std::move(a);
    assert(b.Size() == 3);
    assert(b[0] == 4 && b[1] == 5 && b[2] == 6);
    assert(a.Empty());
}

void TestResizeSmaller()
{
    Vector<int> v{1, 2, 3, 4};
    v.Resize(2);
    assert(v.Size() == 2);
    assert(v[0] == 1);
    assert(v[1] == 2);
}

void TestResizeLarger()
{
    Vector<int> v{1, 2};
    v.Resize(5, 42);
    assert(v.Size() == 5);
    assert(v[2] == 42);
    assert(v[4] == 42);
}

void TestPopBack()
{
    Vector<int> v{7, 8, 9};
    v.PopBack();
    assert(v.Size() == 2);
    assert(v.Back() == 8);
}

void TestClear()
{
    Vector<int> v{1, 2, 3};
    v.Clear();
    assert(v.Empty());
    assert(v.Size() == 0);
}

void TestErase()
{
    Vector<int> v{1, 2, 3};
    auto        it = v.begin() + 1;
    v.Erase(it);
    assert(v.Size() == 2);
    assert(v[0] == 1);
    assert(v[1] == 3);
}

void TestIterators()
{
    Vector<int> v{10, 20, 30};
    int         sum = 0;
    for (auto x : v) sum += x;
    assert(sum == 60);

    Vector<int>::ReverseIterator rit = v.rbegin();
    assert(*rit == 30);
    ++rit;
    assert(*rit == 20);
}

void TestRawPointer()
{
    Vector<int> v{1, 2};
    int*        ptr = v.Raw();
    assert(ptr[0] == 1);
    assert(ptr[1] == 2);
}

void TestEmplaceBack()
{
    Vector<std::string> v;
    v.EmplaceBack("hello");
    v.EmplaceBack(5, 'a');
    assert(v.Size() == 2);
    assert(v[0] == "hello");
    assert(v[1] == "aaaaa");
}

void TestCapacityAndReserve()
{
    Vector<int> v;
    v.Reserve(100);
    assert(v.Capacity() >= 100);
}

int main()
{
    TestDefaultConstructor();
    TestPushBackAndAt();
    TestFrontBack();
    TestInitializerListConstructor();
    TestCopyConstructor();
    TestMoveConstructor();
    TestResizeSmaller();
    TestResizeLarger();
    TestPopBack();
    TestClear();
    TestErase();
    TestIterators();
    TestRawPointer();
    TestEmplaceBack();
    TestCapacityAndReserve();
    return 0;
}
