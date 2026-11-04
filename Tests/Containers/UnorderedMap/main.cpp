/*
 * Created by v1tr10l7 on 28.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */

#include <cassert>
#include <iostream>
#include <string>

#include <Prism/Containers/UnorderedMap.hpp>
#include <Prism/Debug/Test.hpp>
#include <Prism/String/String.hpp>
using namespace Prism;

#define TestEq(condition) PrismTestEq(condition)

template <>
struct Prism::Hash<std::string>
{
    usize operator()(const std::string& key) const
    {
        return Hash<StringView>{}(StringView(key.data(), key.size()));
    }
};
struct TestHasher
{
    std::size_t operator()(const std::string& key) const
    {
        std::size_t hash = 0;
        for (char c : key) hash = hash * 31 + static_cast<std::size_t>(c);
        return hash;
    }
};

void Test_CoreFunctionality() {}
void Test_InsertOrAssign_And_Find()
{
    UnorderedMap<std::string, int> map;

    map.InsertOrAssign("apple", 10);
    map.InsertOrAssign("banana", 20);
    map.InsertOrAssign("cherry", 30);

    TestEq(map.Find("apple")->Value == 10);
    TestEq(map.Find("banana")->Value == 20);
    TestEq(map.Find("cherry")->Value == 30);
    TestEq(map.Find("nonexistent") == map.end());
}

void Test_InsertOrAssignOrAssign()
{
    UnorderedMap<std::string, int> map;

    map.InsertOrAssign("key", 1);
    map.InsertOrAssign("key", 42);
    TestEq(map.Find("key")->Value == 42);

    map.InsertOrAssign("key", 42);
    TestEq(map.Find("key")->Value == 42); // value must be updated
}

void Test_Erase_By_Key()
{
    UnorderedMap<std::string, int> map;

    map.InsertOrAssign("dog", 5);
    map.InsertOrAssign("cat", 6);

    TestEq(map.Find("dog") != map.end());
    TestEq(map.Find("cat") != map.end());

    TestEq(map.Erase("dog") != map.end());
    TestEq(map.Find("dog") == map.end());

    TestEq(map.Erase("nonexistent")
           == map.end()); // trying to erase nonexistent key
}

void Test_Iterator()
{
    UnorderedMap<std::string, int> map;

    map.InsertOrAssign("one", 1);
    map.InsertOrAssign("two", 2);
    map.InsertOrAssign("three", 3);

    int sum = 0;
    for (auto& [name, value] : map) sum += value;

    TestEq(sum == (1 + 2 + 3));
}

namespace
{
    int g_ValueAlive = 0;
    struct Tracked
    {
        int Value = 0;
        Tracked() { ++g_ValueAlive; }
        Tracked(int v)
            : Value(v)
        {
            ++g_ValueAlive;
        }
        Tracked(const Tracked& other)
            : Value(other.Value)
        {
            ++g_ValueAlive;
        }
        Tracked& operator=(const Tracked&) = default;
        ~Tracked() { --g_ValueAlive; }
    };

    // Everything lands in one bucket, so per-bucket chains get long.
    struct CollidingHasher
    {
        usize operator()(const int&) const { return 7; }
    };
} // namespace

// Regression: Insert(const KeyValuePair&) called itself forever.
void Test_InsertConstPair()
{
    using Map = UnorderedMap<int, int>;
    Map                     map;
    const Map::KeyValuePair entry(1, 10);
    map.Insert(entry);
    TestEq(map.Size() == 1 && map.At(1) == 10);

    const Map::KeyValuePair duplicate(1, 99);
    map.Insert(duplicate);
    TestEq(map.Size() == 1 && map.At(1) == 10);
}

// Regression: Clear() freed the bucket array, so begin() read out of bounds
// and later lookups divided by zero buckets.
void Test_ClearThenReuse()
{
    UnorderedMap<int, int> map;
    for (int i = 0; i < 20; ++i) map.InsertOrAssign(i, i);
    map.Clear();

    TestEq(map.Empty() && map.Size() == 0);
    TestEq(map.begin() == map.end());
    TestEq(map.Find(3) == map.end());
    TestEq(map.Erase(3) == map.end());

    map.InsertOrAssign(5, 50);
    TestEq(map.Size() == 1 && map.At(5) == 50);
}

// Regression: Erase() returned an iterator stuck at the end of the bucket
// rather than the next element (or end()).
void Test_EraseReturnsNext()
{
    UnorderedMap<int, int> map;
    for (int i = 0; i < 50; ++i) map.InsertOrAssign(i, i);

    int erased = 0;
    for (auto it = map.begin(); it != map.end();)
    {
        if (it->Key % 2 == 0)
        {
            it = map.Erase(it);
            ++erased;
        }
        else ++it;
    }
    TestEq(erased == 25 && map.Size() == 25);

    int visited = 0;
    for (auto& entry : map)
    {
        TestEq(entry.Key % 2 == 1);
        ++visited;
    }
    TestEq(visited == 25);

    // Erasing everything through the returned iterator must reach end().
    auto it = map.begin();
    while (it != map.end()) it = map.Erase(it);
    TestEq(map.Empty() && map.begin() == map.end());

    UnorderedMap<int, int> single;
    single.InsertOrAssign(1, 1);
    TestEq(single.Erase(1) == single.end());
}

// Many inserts trigger several rehashes; every key must stay reachable.
void Test_RehashKeepsEverything()
{
    UnorderedMap<int, int> map(1);
    for (int i = 0; i < 1000; ++i) map.InsertOrAssign(i, i * 2);
    TestEq(map.Size() == 1000);
    TestEq(map.Capacity() > 1000 * 3 / 4);

    long long sum = 0, count = 0;
    for (auto& entry : map)
    {
        sum += entry.Value;
        ++count;
    }
    TestEq(count == 1000 && sum == 999LL * 1000);
    for (int i = 0; i < 1000; ++i) TestEq(map.At(i) == i * 2);

    map.Reserve(4096);
    TestEq(map.Size() == 1000 && map.Find(777)->Value == 1554);
}

void Test_Collisions()
{
    UnorderedMap<int, int, CollidingHasher> map;
    for (int i = 0; i < 10; ++i) map.InsertOrAssign(i, i);
    TestEq(map.Size() == 10);
    map.Erase(4);
    TestEq(map.Find(4) == map.end() && map.Find(5)->Value == 5);
    TestEq(map.Size() == 9);
}

// Regression: copying the map shallow-copied the bucket lists, so both maps
// shared (and double-deleted) the same nodes.
void Test_CopyAndMove()
{
    g_ValueAlive = 0;
    {
        UnorderedMap<int, Tracked> original;
        for (int i = 0; i < 10; ++i) original.InsertOrAssign(i, Tracked(i));

        UnorderedMap<int, Tracked> copy(original);
        TestEq(copy.Size() == 10);
        copy.At(3).Value = 300;
        TestEq(original.At(3).Value == 3);
        copy.Erase(4);
        TestEq(original.Contains(4) && !copy.Contains(4));

        UnorderedMap<int, Tracked> assigned;
        assigned.InsertOrAssign(42, Tracked(42));
        assigned = copy;
        TestEq(assigned.Size() == 9 && !assigned.Contains(42));
        TestEq(assigned.At(3).Value == 300);

        UnorderedMap<int, Tracked> moved(Move(assigned));
        TestEq(moved.Size() == 9 && assigned.Size() == 0);
        // A moved-from map must still be usable.
        TestEq(assigned.begin() == assigned.end());
        assigned.InsertOrAssign(1, Tracked(1));
        TestEq(assigned.Size() == 1 && assigned.At(1).Value == 1);
    }
    TestEq(g_ValueAlive == 0);
}

// Regression: const operator[] tried to insert into a const map.
void Test_ConstAccess()
{
    UnorderedMap<int, int> map;
    map[1] = 10;
    map[2];
    TestEq(map.Size() == 2 && map.At(2) == 0);

    const auto& view = map;
    TestEq(view[1] == 10 && view.At(1) == 10);
    TestEq(view.Contains(2) && !view.Contains(3));
    TestEq(view.Find(1)->Value == 10);

    int sum = 0;
    for (const auto& entry : view) sum += entry.Value;
    TestEq(sum == 10);
}

void Test_EmplaceKeepsKeysUnique()
{
    UnorderedMap<int, int> map;
    map.Emplace(1, 10);
    map.Emplace(1, 20);
    TestEq(map.Size() == 1 && map.At(1) == 10);

    map.TryEmplace(1, 30);
    TestEq(map.At(1) == 10);
    map.TryEmplace(2, 40);
    TestEq(map.Size() == 2 && map.At(2) == 40);
}

void Test_NoLeaksOnDestroy()
{
    g_ValueAlive = 0;
    {
        UnorderedMap<int, Tracked> map;
        for (int i = 0; i < 100; ++i) map.InsertOrAssign(i, Tracked(i));
        for (int i = 0; i < 50; ++i) map.Erase(i);
        TestEq(g_ValueAlive == 50);
    }
    TestEq(g_ValueAlive == 0);
}

int main()
{
    Test_InsertOrAssign_And_Find();
    Test_InsertOrAssignOrAssign();
    Test_Erase_By_Key();
    Test_Iterator();
    Test_InsertConstPair();
    Test_ClearThenReuse();
    Test_EraseReturnsNext();
    Test_RehashKeepsEverything();
    Test_Collisions();
    Test_CopyAndMove();
    Test_ConstAccess();
    Test_EmplaceKeepsKeysUnique();
    Test_NoLeaksOnDestroy();

    std::cout << "All IntrusiveHashMap tests passed!\n";
    return 0;
}
