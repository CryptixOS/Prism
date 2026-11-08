/*
 * Created by v1tr10l7 on 20.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Containers/Vector.hpp>
#include <string>

using namespace Prism;

TEST_CASE(Vector_TestDefaultConstructor){
    Vector<int> v;
    EXPECT_EQ(v.Size(), 0);
    EXPECT(v.Empty());
}

TEST_CASE(Vector_TestPushBackAndAt){
    Vector<int> v;
    int         a = 10, b = 20;
    v.PushBack(a);
    v.PushBack(b);
    EXPECT_EQ(v.Size(), 2);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v.At(1), 20);
}

TEST_CASE(Vector_TestFrontBack){
    Vector<int> v;
    int         a = 5, b = 15;
    v.PushBack(a);
    v.PushBack(b);
    EXPECT_EQ(v.Front(), 5);
    EXPECT_EQ(v.Back(), 15);
}

TEST_CASE(Vector_TestInitializerListConstructor){
    Vector<int> v{1, 2, 3};
    EXPECT_EQ(v.Size(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST_CASE(Vector_TestCopyConstructor){
    Vector<int> a{1, 2, 3};
    Vector<int> b = a;
    EXPECT_EQ(b.Size(), 3);
    EXPECT(b[0] == 1 && b[1] == 2 && b[2] == 3);
}

TEST_CASE(Vector_TestMoveConstructor){
    Vector<int> a{4, 5, 6};
    Vector<int> b = std::move(a);
    EXPECT_EQ(b.Size(), 3);
    EXPECT(b[0] == 4 && b[1] == 5 && b[2] == 6);
    EXPECT(a.Empty());
}

TEST_CASE(Vector_TestResizeSmaller){
    Vector<int> v{1, 2, 3, 4};
    v.Resize(2);
    EXPECT_EQ(v.Size(), 2);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
}

TEST_CASE(Vector_TestResizeLarger){
    Vector<int> v{1, 2};
    v.Resize(5, 42);
    EXPECT_EQ(v.Size(), 5);
    EXPECT_EQ(v[2], 42);
    EXPECT_EQ(v[4], 42);
}

TEST_CASE(Vector_TestPopBack){
    Vector<int> v{7, 8, 9};
    v.PopBack();
    EXPECT_EQ(v.Size(), 2);
    EXPECT_EQ(v.Back(), 8);
}

TEST_CASE(Vector_TestClear){
    Vector<int> v{1, 2, 3};
    v.Clear();
    EXPECT(v.Empty());
    EXPECT_EQ(v.Size(), 0);
}

TEST_CASE(Vector_TestErase){
    Vector<int> v{1, 2, 3};
    auto        it = v.begin() + 1;
    v.Erase(it);
    EXPECT_EQ(v.Size(), 2);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 3);
}

TEST_CASE(Vector_TestIterators){
    Vector<int> v{10, 20, 30};
    int         sum = 0;
    for (auto x : v) sum += x;
    EXPECT_EQ(sum, 60);

    Vector<int>::ReverseIterator rit = v.rbegin();
    EXPECT_EQ((*rit), 30);
    ++rit;
    EXPECT_EQ((*rit), 20);
}

TEST_CASE(Vector_TestRawPointer){
    Vector<int> v{1, 2};
    int*        ptr = v.Raw();
    EXPECT_EQ(ptr[0], 1);
    EXPECT_EQ(ptr[1], 2);
}

TEST_CASE(Vector_TestEmplaceBack){
    Vector<std::string> v;
    v.EmplaceBack("hello");
    v.EmplaceBack(5, 'a');
    EXPECT_EQ(v.Size(), 2);
    EXPECT_EQ(v[0], "hello");
    EXPECT_EQ(v[1], "aaaaa");
}
TEST_CASE(Vector_TestEmplace){
    Vector<std::string> v;

    // 1. Emplace into an empty vector (acts like end())
    auto                it1 = v.Emplace(v.cbegin(), "middle");
    EXPECT_EQ(v.Size(), 1);
    EXPECT_EQ((*it1), "middle");
    EXPECT_EQ(v[0], "middle");

    // 2. Emplace at the beginning
    auto it2 = v.Emplace(v.cbegin(), "start");
    EXPECT_EQ(v.Size(), 2);
    EXPECT_EQ((*it2), "start");
    EXPECT_EQ(v[0], "start");
    EXPECT_EQ(v[1], "middle");

    // 3. Emplace at the end
    auto it3 = v.Emplace(v.cend(), "end");
    EXPECT_EQ(v.Size(), 3);
    EXPECT_EQ((*it3), "end");
    EXPECT_EQ(v[2], "end");

    // 4. Emplace in the middle with constructor arguments forwarding
    auto it4 = v.Emplace(v.cbegin() + 1, 3, 'x');
    EXPECT_EQ(v.Size(), 4);
    EXPECT_EQ((*it4), "xxx");
    EXPECT_EQ(v[0], "start");
    EXPECT_EQ(v[1], "xxx");
    EXPECT_EQ(v[2], "middle");
    EXPECT_EQ(v[3], "end");

    // 5. Emplace triggering capacity reallocation
    // Shrink capacity down to current size to force a reallocation on next
    // insertion
    v.ShrinkToFit();
    size_t oldCapacity = v.Capacity();
    EXPECT_EQ(v.Size(), oldCapacity);

    auto it5 = v.Emplace(v.cbegin() + 2, "realloc");
    EXPECT_EQ(v.Size(), 5);
    EXPECT(v.Capacity() > oldCapacity);
    EXPECT_EQ((*it5), "realloc");
    EXPECT_EQ(v[2], "realloc");
    EXPECT_EQ(v[3], "middle");
}

TEST_CASE(Vector_TestCapacityAndReserve){
    Vector<int> v;
    v.Reserve(100);
    EXPECT(v.Capacity() >= 100);
}


