/*
 * Created by v1tr10l7 on 11.01.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */

#include <Prism/Test/Test.hpp>
#include <Prism/Algorithm/Find.hpp>
#include <Prism/Containers/Array.hpp>
#include <Prism/Containers/Vector.hpp>
#include <Prism/Debug/Assertions.hpp>

TEST_CASE(TestFind){
    Array<int, 5> arr = {1, 2, 3, 4, 5};

    // Find by value
    auto          it  = Find(arr.begin(), arr.end(), 3);
    EXPECT(it != arr.end() && *it == 3);

    it = Find(arr.begin(), arr.end(), 42);
    EXPECT_EQ(it, arr.end());

    // FindIf
    it = FindIf(arr.begin(), arr.end(), [](int x) { return x % 2 == 0; });
    EXPECT(it != arr.end() && *it == 2);

    // FindIfNot
    it = FindIfNot(arr.begin(), arr.end(), [](int x) { return x < 4; });
    EXPECT(it != arr.end() && *it == 4);

    // FindLastIf
    it = FindLastIf(arr.begin(), arr.end(), [](int x) { return x % 2 == 1; });
    EXPECT(it != arr.end() && *it == 5);
}

TEST_CASE(TestFindIter){
    Vector<int> vec = {10, 20, 30};

    // FindIfIter
    auto        it
        = FindIfIter(vec.begin(), vec.end(), [](auto i) { return *i == 20; });
    EXPECT(it != vec.end() && *it == 20);

    // FindValueIter
    int value = 30;
    it        = FindValueIter(vec.begin(), vec.end(), value,
                              [](auto i, int* v) { return *i == *v; });
    EXPECT(it != vec.end() && *it == 30);

    // FindIteratorIter
    auto it2 = vec.begin();
    ++it2; // points to 20
    it = FindIteratorIter(
        vec.begin(), vec.end(), [](auto a, auto b) { return *a == *b; }, it2);
    EXPECT(it != vec.end() && *it == 20);
}

TEST_CASE(TestAnyAllNoneOf){
    Vector<int> vec = {1, 2, 3, 4};

    EXPECT(AnyOf(vec.begin(), vec.end(), [](int x) { return x == 3; }));
    EXPECT(!AnyOf(vec.begin(), vec.end(), [](int x) { return x == 42; }));

    EXPECT(AllOf(vec.begin(), vec.end(), [](int x) { return x > 0; }));
    EXPECT(!AllOf(vec.begin(), vec.end(), [](int x) { return x < 4; }));

    EXPECT(NoneOf(vec.begin(), vec.end(), [](int x) { return x > 5; }));
    EXPECT(!NoneOf(vec.begin(), vec.end(), [](int x) { return x == 2; }));

    // Comparator + value
    EXPECT(AnyOf(vec.begin(), vec.end(), EqualTo<>{}, 4));
    EXPECT(!AllOf(vec.begin(), vec.end(), Greater<>{}, 2));
}

TEST_CASE(TestCount){
    Vector<int> vec = {1, 2, 2, 3, 3, 3};

    EXPECT_EQ((Count(vec.begin(), vec.end(), 2)), 2);
    EXPECT_EQ((CountIf(vec.begin(), vec.end(), [](int x) { return x % 2 == 1; })), 4);
    EXPECT_EQ((CountIfNot(vec.begin(), vec.end(), [](int x) { return x % 2 == 0; })), 4);

    // Iterator-aware
    EXPECT_EQ((CountIfIter(vec.begin(), vec.end(), [](auto it) { return *it == 3; })), 3);
}

TEST_CASE(TestRemoveEraseIf){
    Vector<int> vec = {1, 2, 3, 4, 5};

    auto        newEnd
        = RemoveIf(vec.begin(), vec.end(), [](int x) { return x % 2 == 0; });
    vec.erase(newEnd, vec.end());
    EXPECT((vec == Vector<int>{1, 3, 5}));

    // Refill
    vec = {1, 2, 3, 4, 5};

    EraseIf(vec, [](int x) { return x > 3; });
    EXPECT((vec == Vector<int>{1, 2, 3}));

    // Iterator-aware
    vec    = {1, 2, 3, 4, 5};
    newEnd = RemoveIfIter(vec.begin(), vec.end(),
                          [](auto it) { return *it % 2 == 1; });
    vec.erase(newEnd, vec.end());
    EXPECT((vec == Vector<int>{2, 4}));
}

TEST_CASE(TestFindFirstOf){
    Vector<int> vec     = {1, 2, 3, 4};
    Vector<int> needles = {0, 3, 5};

    auto        it
        = FindFirstOf(vec.begin(), vec.end(), needles.begin(), needles.end());
    EXPECT(it != vec.end() && *it == 3);

    // Comparator
    it = FindFirstOf(vec.begin(), vec.end(), needles.begin(), needles.end(),
                     EqualTo<>{});
    EXPECT(it != vec.end() && *it == 3);
}

TEST_CASE(TestFindIfProj){
    Vector<KeyValuePair<int, int>> vec = {{1, 2}, {3, 4}, {5, 6}};
    auto                           it  = FindIfProj(
        vec.begin(), vec.end(), [](int x) { return x == 4; },
        [](auto& p) { return p.second; });
    EXPECT(it != vec.end() && it->second == 4);

    it = FindIfProj(vec.begin(), vec.end(), EqualTo<>{}, 5,
                    [](auto& p) { return p.first; });
    EXPECT(it != vec.end() && it->first == 5);
}


