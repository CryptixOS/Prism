/*
 * Created by v1tr10l7 on 11.01.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */

#include <Prism/Algorithm/Find.hpp>
#include <Prism/Containers/Array.hpp>
#include <Prism/Containers/Vector.hpp>
#include <Prism/Debug/Assertions.hpp>

void TestFind()
{
    Array<int, 5> arr = {1, 2, 3, 4, 5};

    // Find by value
    auto          it  = Find(arr.begin(), arr.end(), 3);
    PrismAssert(it != arr.end() && *it == 3);

    it = Find(arr.begin(), arr.end(), 42);
    PrismAssert(it == arr.end());

    // FindIf
    it = FindIf(arr.begin(), arr.end(), [](int x) { return x % 2 == 0; });
    PrismAssert(it != arr.end() && *it == 2);

    // FindIfNot
    it = FindIfNot(arr.begin(), arr.end(), [](int x) { return x < 4; });
    PrismAssert(it != arr.end() && *it == 4);

    // FindLastIf
    it = FindLastIf(arr.begin(), arr.end(), [](int x) { return x % 2 == 1; });
    PrismAssert(it != arr.end() && *it == 5);
}

void TestFindIter()
{
    Vector<int> vec = {10, 20, 30};

    // FindIfIter
    auto        it
        = FindIfIter(vec.begin(), vec.end(), [](auto i) { return *i == 20; });
    PrismAssert(it != vec.end() && *it == 20);

    // FindValueIter
    int value = 30;
    it        = FindValueIter(vec.begin(), vec.end(), value,
                              [](auto i, int* v) { return *i == *v; });
    PrismAssert(it != vec.end() && *it == 30);

    // FindIteratorIter
    auto it2 = vec.begin();
    ++it2; // points to 20
    it = FindIteratorIter(
        vec.begin(), vec.end(), [](auto a, auto b) { return *a == *b; }, it2);
    PrismAssert(it != vec.end() && *it == 20);
}

void TestAnyAllNoneOf()
{
    Vector<int> vec = {1, 2, 3, 4};

    PrismAssert(AnyOf(vec.begin(), vec.end(), [](int x) { return x == 3; }));
    PrismAssert(!AnyOf(vec.begin(), vec.end(), [](int x) { return x == 42; }));

    PrismAssert(AllOf(vec.begin(), vec.end(), [](int x) { return x > 0; }));
    PrismAssert(!AllOf(vec.begin(), vec.end(), [](int x) { return x < 4; }));

    PrismAssert(NoneOf(vec.begin(), vec.end(), [](int x) { return x > 5; }));
    PrismAssert(!NoneOf(vec.begin(), vec.end(), [](int x) { return x == 2; }));

    // Comparator + value
    PrismAssert(AnyOf(vec.begin(), vec.end(), EqualTo<>{}, 4));
    PrismAssert(!AllOf(vec.begin(), vec.end(), Greater<>{}, 2));
}

void TestCount()
{
    Vector<int> vec = {1, 2, 2, 3, 3, 3};

    PrismAssert(Count(vec.begin(), vec.end(), 2) == 2);
    PrismAssert(
        CountIf(vec.begin(), vec.end(), [](int x) { return x % 2 == 1; }) == 4);
    PrismAssert(
        CountIfNot(vec.begin(), vec.end(), [](int x) { return x % 2 == 0; })
        == 4);

    // Iterator-aware
    PrismAssert(
        CountIfIter(vec.begin(), vec.end(), [](auto it) { return *it == 3; })
        == 3);
}

void TestRemoveEraseIf()
{
    Vector<int> vec = {1, 2, 3, 4, 5};

    auto        newEnd
        = RemoveIf(vec.begin(), vec.end(), [](int x) { return x % 2 == 0; });
    vec.erase(newEnd, vec.end());
    PrismAssert((vec == Vector<int>{1, 3, 5}));

    // Refill
    vec = {1, 2, 3, 4, 5};

    EraseIf(vec, [](int x) { return x > 3; });
    PrismAssert((vec == Vector<int>{1, 2, 3}));

    // Iterator-aware
    vec    = {1, 2, 3, 4, 5};
    newEnd = RemoveIfIter(vec.begin(), vec.end(),
                          [](auto it) { return *it % 2 == 1; });
    vec.erase(newEnd, vec.end());
    PrismAssert((vec == Vector<int>{2, 4}));
}

void TestFindFirstOf()
{
    Vector<int> vec     = {1, 2, 3, 4};
    Vector<int> needles = {0, 3, 5};

    auto        it
        = FindFirstOf(vec.begin(), vec.end(), needles.begin(), needles.end());
    PrismAssert(it != vec.end() && *it == 3);

    // Comparator
    it = FindFirstOf(vec.begin(), vec.end(), needles.begin(), needles.end(),
                     EqualTo<>{});
    PrismAssert(it != vec.end() && *it == 3);
}

void TestFindIfProj()
{
    Vector<KeyValuePair<int, int>> vec = {{1, 2}, {3, 4}, {5, 6}};
    auto                           it  = FindIfProj(
        vec.begin(), vec.end(), [](int x) { return x == 4; },
        [](auto& p) { return p.second; });
    PrismAssert(it != vec.end() && it->second == 4);

    it = FindIfProj(vec.begin(), vec.end(), EqualTo<>{}, 5,
                    [](auto& p) { return p.first; });
    PrismAssert(it != vec.end() && it->first == 5);
}

int main()
{
    TestFind();
    TestFindIter();
    TestAnyAllNoneOf();
    TestCount();
    TestRemoveEraseIf();
    TestFindFirstOf();
    TestFindIfProj();

    return 0;
}
