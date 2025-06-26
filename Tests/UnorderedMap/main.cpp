/*
 * Created by v1tr10l7 on 28.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */

#include <cassert>
#include <format>
#include <iostream>
#include <set>

#include <Prism/Containers/UnorderedMap.hpp>
#include <Prism/String/String.hpp>
using namespace Prism;

#define TestEq(condition) assert(condition)

struct TestHasher
{
    std::size_t operator()(const std::string& key) const
    {
        std::size_t hash = 0;
        for (char c : key) hash = hash * 31 + static_cast<std::size_t>(c);
        return hash;
    }
};

void Test_CoreFunctionality()
{
    UnorderedMap<std::string, int> map;

    // Initially empty
    assert(map.Empty());
    assert(map.Size() == 0);

    // Insert values
    map.Insert("apple", 10);
    map.Insert("banana", 20);
    map.Insert("cherry", 30);
    map.Insert("apple", 10);
    map.Insert("banana", 20);
    map.Insert("cherry", 30);

    std::print(std::cout, "iterating\n");
    for (const auto& entry : map)
        std::print(std::cout, "{} => {}\n", entry.Key, entry.Value);
    std::print(std::cout, "done\n");

    assert(!map.Empty());
    assert(map.Size() == 3);
    assert(map.At("apple") == 10);
    assert(map.At("banana") == 20);
    assert(map.At("cherry") == 30);
    assert(map.Contains("banana"));
    assert(!map.Contains("date"));

    // Overwrite value
    map.Insert("apple", 42);
    assert(map.At("apple") == 42);
    assert(map.Size() == 3);

    // operator[]
    map["banana"] = 99;
    assert(map.At("banana") == 99);
    assert(map.Size() == 3);

    map["date"] = 123; // inserted
    assert(map.At("date") == 123);
    assert(map.Size() == 4);

    // Find
    auto it = map.Find("cherry");
    assert(it != map.end());
    assert(it->Key == "cherry");
    assert(it->Value == 30);

    auto miss = map.Find("unknown");
    assert(miss == map.end());

    // Erase
    map.Erase("banana");
    assert(!map.Contains("banana"));
    assert(map.Size() == 3);

    // Iteration
    usize keysFound = 0;
    for (auto& pair : map)
    {
        std::print(std::cout, "Entry: {} => {}\n", pair.Key, pair.Value);
        if (pair.Key == "apple")
        {
            assert(pair.Value == 42);
            keysFound++;
        }
        if (pair.Key == "cherry")
        {
            assert(pair.Value == 30);
            keysFound++;
        }
        if (pair.Key == "date")
        {
            assert(pair.Value == 123);
            keysFound++;
        }
    }
    assert(keysFound == 3);

    // Clear
    map.Clear();
    assert(map.Size() == 0);
    assert(map.Empty());
}

void Test_InsertOrAssign()
{
    UnorderedMap<std::string, int> map;

    map.Insert("key", 1);
    map.Insert("key", 42);
    TestEq(map.Find("key")->Value == 42);

    map.Insert("key", 42);
    TestEq(map.Find("key")->Value == 42); // value must be updated
}

void Test_Erase_By_Key()
{
    UnorderedMap<std::string, int> map;

    map.Insert("dog", 5);
    map.Insert("cat", 6);

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

    map.Insert("one", 1);
    map.Insert("two", 2);
    map.Insert("three", 3);

    int sum = 0;
    for (auto& node : map) sum += node.Value;

    TestEq(sum == (1 + 2 + 3));
}
void Test_Rehash()
{
    UnorderedMap<std::string, int> map(2); // force small initial capacity

    for (int i = 0; i < 20; ++i)
    {
        map.Insert(std::format("key{}", i), i * 10);
    }

    assert(map.Size() == 20);

    for (int i = 0; i < 20; ++i)
    {
        std::string key = std::format("key{}", i);
        assert(map.At(key) == i * 10);
    }

    // Force rehash
    map.Rehash(64);
    assert(map.Capacity() >= 64);

    // Data must be intact after rehash
    for (int i = 0; i < 20; ++i)
    {
        std::string key = std::format("key{}", i);
        assert(map.At(key) == i * 10);
    }
}

struct DumbHash
{
    constexpr usize operator()(const std::string& s) const
    {
        // Only use first character for hashing
        return s[0];
    }
};

void Test_CustomHash()
{
    UnorderedMap<std::string, int, DumbHash> map;

    map.Insert("apple", 100);
    map.Insert("apricot", 200); // collision with "apple" in DumbHash

    assert(map.At("apple") == 100);
    assert(map.At("apricot") == 200);
    assert(map.Contains("apple"));
    assert(map.Contains("apricot"));

    // They should both coexist despite collisions
    map.Erase("apple");
    assert(!map.Contains("apple"));
    assert(map.At("apricot") == 200);
}
void Test_NodePointerStability()
{
    UnorderedMap<std::string, int> map;

    Vector<const void*>            nodeAddresses;

    for (int i = 0; i < 100; ++i)
    {
        std::string key = std::format("k{}", i);
        map.Insert(key, i);

        auto it = map.Find(key);
        assert(it != map.end());

        nodeAddresses.PushBack(static_cast<const void*>(&*it));
    }

    // Force rehash
    map.Rehash(256);

    // All node addresses should still be valid and unique
    std::set<const void*> seen;
    for (const void* ptr : nodeAddresses)
    {
        assert(ptr != nullptr);
        assert(!seen.contains(ptr));
        seen.insert(ptr);
    }

    assert(seen.size() == 100);
}
void Test_Stress()
{
    UnorderedMap<usize, usize> map;

    for (usize i = 0; i < 10000; ++i) map.Insert(i, i * 2);
    for (usize i = 0; i < 10000; ++i) assert(map.At(i) == i * 2);

    assert(map.Size() == 10000);
}

void Test_InsertFindEraseSequences()
{
    UnorderedMap<std::string, int> map;

    // Insert many keys with similar prefixes
    for (int i = 0; i < 50; ++i)
    {
        std::string key = std::format("item{}", i);
        map.Insert(key, i);
    }

    // Validate all inserts
    for (int i = 0; i < 50; ++i)
    {
        std::string key = std::format("item{}", i);
        assert(map.Contains(key));
        assert(map.At(key) == i);
    }

    // Erase every even item
    for (int i = 0; i < 50; i += 2)
    {
        std::string key = std::format("item{}", i);
        map.Erase(key);
    }

    // Ensure correct keys remain
    for (int i = 0; i < 50; ++i)
    {
        std::string key = std::format("item{}", i);
        if (i % 2 == 0) assert(!map.Contains(key));
        else
        {
            assert(map.Contains(key));
            assert(map.At(key) == i);
        }
    }

    // Re-insert erased items
    for (int i = 0; i < 50; i += 2)
    {
        std::string key = std::format("item{}", i);
        map.Insert(key, i * 100);
    }

    // Verify all keys again
    for (int i = 0; i < 50; ++i)
    {
        std::string key = std::format("item{}", i);
        assert(map.Contains(key));
        if (i % 2 == 0) assert(map.At(key) == i * 100);
        else assert(map.At(key) == i);
    }
}

void Test_BulkErase()
{
    UnorderedMap<std::string, int> map;

    // Bulk insert
    for (int i = 0; i < 100; ++i) map.Insert(std::format("k{}", i), i);

    assert(map.Size() == 100);

    // Erase in reverse order
    for (int i = 99; i >= 0; --i)
    {
        std::string key = std::format("k{}", i);
        assert(map.Contains(key));
        map.Erase(key);
        assert(!map.Contains(key));
    }

    assert(map.Empty());
}

void Test_NonTrivialTypes()
{
    struct Widget
    {
        std::string Name;
        int         Value;

        bool        operator==(const Widget& other) const
        {
            return Name == other.Name && Value == other.Value;
        }
    };

    struct WidgetHash
    {
        usize operator()(const Widget& w) const
        {
            return std::hash<std::string>{}(w.Name) ^ (w.Value * 17);
        }
    };

    UnorderedMap<Widget, std::string, WidgetHash> map;

    Widget                                        w1 = {"Engine", 10};
    Widget                                        w2 = {"Wheel", 20};

    map.Insert(w1, "First");
    map.Insert(w2, "Second");

    assert(map.At(w1) == "First");
    assert(map.At(w2) == "Second");

    map[w1] = "Updated";
    assert(map.At(w1) == "Updated");

    map.Erase(w2);
    assert(!map.Contains(w2));
}

int main()
{
    Test_CoreFunctionality();
    Test_InsertOrAssign();
    Test_Erase_By_Key();
    Test_Iterator();
    Test_Rehash();
    Test_CustomHash();
    Test_NodePointerStability();
    Test_Stress();

    Test_InsertFindEraseSequences();
    Test_BulkErase();
    Test_NonTrivialTypes();

    std::cout << "All IntrusiveHashMap tests passed!\n";
    return 0;
}
