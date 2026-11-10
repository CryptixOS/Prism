/*
 * Created by v1tr10l7 on 19.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Containers/RedBlackTree.hpp>


using namespace Prism;

void RBTree_TestInsertFind()
{
    RedBlackTree<int, String> tree;

    String                    a = "apple";
    String                    b = "banana";
    String                    c = "cherry";

    tree.Insert(10, a);
    tree.Insert(20, b);
    tree.Insert(5, c);

    EXPECT_EQ(tree.GetSize(), 3);
    EXPECT(tree.Contains(10));
    EXPECT(tree.Contains(20));
    EXPECT(tree.Contains(5));
    EXPECT(!tree.Contains(42));

    EXPECT(tree.Contains(10) && tree.At(10) == "apple");
    EXPECT(tree.Contains(20) && tree.At(20) == "banana");
    EXPECT(tree.Contains(5) && tree.At(5) == "cherry");
}

void RBTree_TestErase()
{
    RedBlackTree<int, String> tree;
    String                    v = "value";

    tree.Insert(1, v);
    tree.Insert(2, v);
    tree.Insert(3, v);

    EXPECT_EQ(tree.GetSize(), 3);
    EXPECT(tree.Erase(2));
    EXPECT_EQ(tree.GetSize(), 2);
    EXPECT(!tree.Contains(2));
    EXPECT(tree.Contains(1));
    EXPECT(tree.Contains(3));
    EXPECT(!tree.Erase(42)); // erase nonexistent
}

void RBTree_TestClear()
{
    RedBlackTree<int, String> tree;
    String                    val = "v";
    tree.Insert(1, val);
    tree.Insert(2, val);
    tree.Insert(3, val);

    EXPECT(!tree.IsEmpty());
    tree.Clear();
    EXPECT(tree.IsEmpty());
    EXPECT_EQ(tree.GetSize(), 0);
}

void RBTree_TestIterator()
{
    RedBlackTree<int, String> tree;
    String                    v1 = "a", v2 = "b", v3 = "c";
    tree.Insert(2, v1);
    tree.Insert(1, v2);
    tree.Insert(3, v3);

    auto it = tree.begin();
    EXPECT(!it.IsLast());
    EXPECT(it->Key == 1 || it->Key == 2 || it->Key == 3);

    size_t count = 0;
    for (auto i = tree.begin(); i != tree.end(); ++i) ++count;
    EXPECT_EQ(count, 3);
}

void RBTree_TestInsertOverwrite()
{
    RedBlackTree<int, String> tree;
    String                    val1 = "first";
    String                    val2 = "second";

    tree.Insert(10, val1);
    tree.Insert(10, val2); // should overwrite

    EXPECT_EQ(tree.GetSize(), 1);
    EXPECT(tree.Contains(10) && tree.At(10) == "second");
}

void RBTree_TestDeleteOnlyRoot()
{
    RedBlackTree<int, String> tree;
    String                    val = "only";

    tree.Insert(42, val);
    EXPECT_EQ(tree.GetSize(), 1);
    EXPECT(tree.Erase(42));
    EXPECT(tree.IsEmpty());
}

void RBTree_TestSequentialInsert()
{
    RedBlackTree<int, String> tree;
    String                    dummy = "x";

    for (int i = 1; i <= 1000; ++i) tree.Insert(i, dummy);

    EXPECT_EQ(tree.GetSize(), 1000);
    for (int i = 1; i <= 1000; ++i) EXPECT(tree.Contains(i));
}

void RBTree_TestInsertDeleteConsistency()
{
    RedBlackTree<int, String> tree;
    String                    val = "v";

    for (int i = 0; i < 100; ++i) tree.Insert(i, val);

    for (int i = 0; i < 100; i += 2) tree.Erase(i);

    EXPECT_EQ(tree.GetSize(), 50);
    for (int i = 0; i < 100; ++i)
    {
        if (i % 2 == 0) EXPECT(!tree.Contains(i));
        else EXPECT(tree.Contains(i));
    }
}

void RBTree_TestNullptrBehavior()
{
    RedBlackTree<int, String> tree;
    String                    val = "value";
    tree.Insert(1, val);

    EXPECT(!tree.Contains(2));
}

TEST_CASE(RBTree_RunAllTests){
    RBTree_TestInsertFind();
    RBTree_TestErase();
    RBTree_TestClear();
    RBTree_TestIterator();
    RBTree_TestInsertOverwrite();
    RBTree_TestDeleteOnlyRoot();
    RBTree_TestSequentialInsert();
    RBTree_TestInsertDeleteConsistency();
    RBTree_TestNullptrBehavior();
    printf("All tests passed.\n");
}


