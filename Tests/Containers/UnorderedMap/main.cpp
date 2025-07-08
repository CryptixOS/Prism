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
using namespace Prism;

#define TestEq(condition) PrismTestEq(condition)

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

int main()
{
    Test_InsertOrAssign_And_Find();
    Test_InsertOrAssignOrAssign();
    Test_Erase_By_Key();
    Test_Iterator();
    //   Test_Rehash();

    std::cout << "All IntrusiveHashMap tests passed!\n";
    return 0;
}
