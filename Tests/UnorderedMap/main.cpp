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
using namespace Prism;

struct TestHasher
{
    std::size_t operator()(const std::string& key) const
    {
        std::size_t hash = 0;
        for (char c : key) hash = hash * 31 + static_cast<std::size_t>(c);
        return hash;
    }
};

void Test_Insert_And_Find()
{
    UnorderedMap<std::string, int> map;

    map.Insert("apple", 10);
    map.Insert("banana", 20);
    map.Insert("cherry", 30);

    assert(map.Find("apple")->value == 10);
    assert(map.Find("banana")->value == 20);
    assert(map.Find("cherry")->value == 30);
    assert(map.Find("nonexistent") == nullptr);
}

void Test_InsertOrAssign()
{
    UnorderedMap<std::string, int> map;

    map.Insert("key", 1);
    map.Insert("key", 42);
    assert(map.Find("key")->value == 1);

    map.Insert("key", 42);
    assert(map.Find("key")->value == 42); // value must be updated
}

void Test_Erase_By_Key()
{
    UnorderedMap<std::string, int> map;

    map.Insert("dog", 5);
    map.Insert("cat", 6);

    assert(map.Find("dog") != nullptr);
    assert(map.Find("cat") != nullptr);

    assert(map.Erase("dog"));
    assert(map.Find("dog") == nullptr);

    assert(!map.Erase("nonexistent")); // trying to erase nonexistent key
}

void Test_Iterator()
{
    UnorderedMap<std::string, int> map;

    map.Insert("one", 1);
    map.Insert("two", 2);
    map.Insert("three", 3);

    int sum = 0;
    for (auto& node : map) { sum += node.value; }

    assert(sum == (1 + 2 + 3));
}

int main()
{
    Test_Insert_And_Find();
    Test_InsertOrAssign();
    Test_Erase_By_Key();
    Test_Iterator();
    // Test_Rehash();

    std::cout << "All IntrusiveHashMap tests passed!\n";
    return 0;
}
