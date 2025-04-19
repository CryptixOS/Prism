/*
 * Created by v1tr10l7 on 19.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/RedBlackTree.hpp>

#include <cassert>
#include <string>

using namespace Prism;

void TestInsertAndFind()
{
    RedBlackTree<int, std::string> tree;

    std::string                    a = "apple";
    std::string                    b = "banana";
    std::string                    c = "cherry";

    tree.Insert(10, a);
    tree.Insert(20, b);
    tree.Insert(5, c);

    assert(tree.GetSize() == 3);
    assert(tree.Contains(10));
    assert(tree.Contains(20));
    assert(tree.Contains(5));
    assert(!tree.Contains(42));

    assert(tree.Contains(10) && tree.At(10) == "apple");
    assert(tree.Contains(20) && tree.At(20) == "banana");
    assert(tree.Contains(5) && tree.At(5) == "cherry");
}

void TestErase()
{
    RedBlackTree<int, std::string> tree;
    std::string                    v = "value";

    tree.Insert(1, v);
    tree.Insert(2, v);
    tree.Insert(3, v);

    assert(tree.GetSize() == 3);
    assert(tree.Erase(2));
    assert(tree.GetSize() == 2);
    assert(!tree.Contains(2));
    assert(tree.Contains(1));
    assert(tree.Contains(3));
    assert(!tree.Erase(42)); // erase nonexistent
}

void TestClear()
{
    RedBlackTree<int, std::string> tree;
    std::string                    val = "v";
    tree.Insert(1, val);
    tree.Insert(2, val);
    tree.Insert(3, val);

    assert(!tree.IsEmpty());
    tree.Clear();
    assert(tree.IsEmpty());
    assert(tree.GetSize() == 0);
}

void TestIterator()
{
    RedBlackTree<int, std::string> tree;
    std::string                    v1 = "a", v2 = "b", v3 = "c";
    tree.Insert(2, v1);
    tree.Insert(1, v2);
    tree.Insert(3, v3);

    auto it = tree.begin();
    assert(!it.IsLast());
    assert(it->Key == 1 || it->Key == 2 || it->Key == 3);

    size_t count = 0;
    for (auto i = tree.begin(); i != tree.end(); ++i) ++count;
    assert(count == 3);
}

void TestInsertOverwrite()
{
    RedBlackTree<int, std::string> tree;
    std::string                    val1 = "first";
    std::string                    val2 = "second";

    tree.Insert(10, val1);
    tree.Insert(10, val2); // should overwrite

    assert(tree.GetSize() == 1);
    assert(tree.Contains(10) && tree.At(10) == "second");
}

void TestDeleteOnlyRoot()
{
    RedBlackTree<int, std::string> tree;
    std::string                    val = "only";

    tree.Insert(42, val);
    assert(tree.GetSize() == 1);
    assert(tree.Erase(42));
    assert(tree.IsEmpty());
}

void TestSequentialInsert()
{
    RedBlackTree<int, std::string> tree;
    std::string                    dummy = "x";

    for (int i = 1; i <= 1000; ++i) tree.Insert(i, dummy);

    assert(tree.GetSize() == 1000);
    for (int i = 1; i <= 1000; ++i) assert(tree.Contains(i));
}

void TestInsertDeleteConsistency()
{
    RedBlackTree<int, std::string> tree;
    std::string                    val = "v";

    for (int i = 0; i < 100; ++i) tree.Insert(i, val);

    for (int i = 0; i < 100; i += 2) tree.Erase(i);

    assert(tree.GetSize() == 50);
    for (int i = 0; i < 100; ++i)
    {
        if (i % 2 == 0) assert(!tree.Contains(i));
        else assert(tree.Contains(i));
    }
}

void TestNullptrBehavior()
{
    RedBlackTree<int, std::string> tree;
    std::string                    val = "value";
    tree.Insert(1, val);

    assert(!tree.Contains(2));
}

void RunAllTests()
{
    TestInsertAndFind();
    TestErase();
    TestClear();
    TestIterator();
    TestInsertOverwrite();
    TestDeleteOnlyRoot();
    TestSequentialInsert();
    TestInsertDeleteConsistency();
    TestNullptrBehavior();
    printf("All tests passed.\n");
}

int main()
{
    RunAllTests();
    return 0;
}
