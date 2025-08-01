/*
 * Created by v1tr10l7 on 01.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Algorithm/Sort.hpp>
#include <Prism/Containers/Array.hpp>
#include <Prism/Containers/Vector.hpp>
#include <Prism/Utility/Compare.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>

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
        assert(IsSorted(v));
    }
    {
        Container v = {42};
        sorter(v.begin(), v.end(), Less<typename Container::ValueType>{});
        assert(IsSorted(v));
    }
    {
        Container v = {5, 4, 3, 2, 1};
        sorter(v.begin(), v.end(), Less<typename Container::ValueType>{});
        assert(IsSorted(v));
    }
    {
        Container v = {1, 2, 3, 4, 5};
        sorter(v.begin(), v.end(), Less<typename Container::ValueType>{});
        assert(IsSorted(v));
    }
    {
        Container v = {3, 1, 4, 1, 5, 9, 2};
        sorter(v.begin(), v.end(), Less<typename Container::ValueType>{});
        assert(IsSorted(v));
    }
    {
        Container v(100);
        for (usize i = 0; i < v.Size(); ++i) v[i] = 100 - i;

        sorter(v.begin(), v.end(), Less<typename Container::ValueType>{});
        assert(IsSorted(v));
    }
}

int main()
{
    TestSortAlgorithm(InsertionSort);
    TestSortAlgorithm(HeapSort);
    TestSortAlgorithm(MergeSort);
    TestSortAlgorithm(QuickSort);
    TestSortAlgorithm(IntroSort);
    TestSortAlgorithm(Sort);
    return 0;
}
