/*
 * Created by v1tr10l7 on 01.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
#include <cstdio>
#include <cstring>
import Prism.Test.Test;
import Prism.Algorithm.Sort;
import Prism.Containers.Array;
import Prism.Containers.Vector;
import Prism.Utility.Compare;

using namespace Prism;

template <typename Container>
bool IsSorted(const Container& c)
{
    for (size_t i = 1; i < c.Size(); ++i)
        if (c[i] < c[i - 1]) return false;
    return true;
}

template <typename Container,
          typename Compare = Less<typename Container::ValueType>>
using Sorter = void (*)(typename Container::Iterator,
                        typename Container::Iterator, Compare);

template <typename Container = Vector<int>>
void TestSortAlgorithm(Sorter<Container> sorter)
{
    {
        Container v = {};
        sorter(v.begin(), v.end(), Less<typename Container::ValueType>{});
        EXPECT(IsSorted(v));
    }
    {
        Container v = {42};
        sorter(v.begin(), v.end(), Less<typename Container::ValueType>{});
        EXPECT(IsSorted(v));
    }
    {
        Container v = {5, 4, 3, 2, 1};
        sorter(v.begin(), v.end(), Less<typename Container::ValueType>{});
        EXPECT(IsSorted(v));
    }
    {
        Container v = {1, 2, 3, 4, 5};
        sorter(v.begin(), v.end(), Less<typename Container::ValueType>{});
        EXPECT(IsSorted(v));
    }
    {
        Container v = {3, 1, 4, 1, 5, 9, 2};
        sorter(v.begin(), v.end(), Less<typename Container::ValueType>{});
        EXPECT(IsSorted(v));
    }
    {
        Container v(100);
        for (usize i = 0; i < v.Size(); ++i) v[i] = 100 - i;

        sorter(v.begin(), v.end(), Less<typename Container::ValueType>{});
        EXPECT(IsSorted(v));
    }
}


TEST_CASE(TestSortAlgorithm_InsertionSort)
{
    TestSortAlgorithm(InsertionSort);
}

TEST_CASE(TestSortAlgorithm_HeapSort)
{
    TestSortAlgorithm(HeapSort);
}

TEST_CASE(TestSortAlgorithm_MergeSort)
{
    TestSortAlgorithm(MergeSort);
}

TEST_CASE(TestSortAlgorithm_QuickSort)
{
    TestSortAlgorithm(QuickSort);
}

TEST_CASE(TestSortAlgorithm_IntroSort)
{
    TestSortAlgorithm(IntroSort);
}

TEST_CASE(TestSortAlgorithm_Sort)
{
    TestSortAlgorithm(Sort);
}

