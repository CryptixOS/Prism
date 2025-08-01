/*
 * Created by v1tr10l7 on 01.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Concepts.hpp>
#include <Prism/Core/Iterator.hpp>
#include <Prism/Utility/Compare.hpp>
#include <Prism/Utility/Math.hpp>

namespace Prism
{
    template <typename It, typename Compare>
    void InsertionSort(It first, It last, Compare comparator)
    {
        for (It it = first + 1; it < last; ++it)
        {
            auto key     = Move(*it);
            It   current = it;
            while (current > first && comparator(key, *(current - 1)))
            {
                *current = Move(*(current - 1));
                --current;
            }
            *current = Move(key);
        }
    }

    template <typename It, typename Compare>
    void Heapify(It first, isize size, isize root, Compare comp)
    {
        isize largest   = root;
        isize left      = 2 * root + 1;
        isize right     = 2 * root + 2;

        It    largestIt = Next(first, largest);

        if (left < size)
        {
            It leftIt = Next(first, left);
            if (comp(*largestIt, *leftIt))
            {
                largest   = left;
                largestIt = leftIt;
            }
        }

        if (right < size)
        {
            It rightIt = Next(first, right);
            if (comp(*largestIt, *rightIt))
            {
                largest   = right;
                largestIt = rightIt;
            }
        }

        if (largest != root)
        {
            It rootIt = Next(first, root);
            IteratorSwap(rootIt, largestIt);
            Heapify(first, size, largest, comp);
        }
    }

    template <typename It, typename Compare>
    void HeapSort(It first, It last, Compare comp)
    {
        isize size = Distance(first, last);
        if (size <= 1) return;

        for (isize i = size / 2 - 1; i >= 0; --i) Heapify(first, size, i, comp);

        for (isize i = size - 1; i > 0; --i)
        {
            IteratorSwap(first, Next(first, i));
            Heapify(first, i, 0, comp);
        }
    }

    template <typename It, typename Compare>
    void Merge(It first, It mid, It last, Compare comp)
    {
        using ValueType = typename IteratorTraits<It>::ValueType;
        Vector<ValueType> left(first, mid);
        Vector<ValueType> right(mid, last);

        auto              l   = left.begin();
        auto              r   = right.begin();
        auto              out = first;

        while (l != left.end() && r != right.end())
        {
            if (comp(*r, *l)) *out++ = Move(*r++);
            else *out++ = Move(*l++);
        }

        while (l != left.end()) *out++ = Move(*l++);
        while (r != right.end()) *out++ = Move(*r++);
    }

    template <typename It, typename Compare>
    void MergeSort(It first, It last, Compare comp)
    {
        auto size = last - first;
        if (size <= 1) return;

        It mid = first + size / 2;
        MergeSort(first, mid, comp);
        MergeSort(mid, last, comp);
        Merge(first, mid, last, comp);
    }

    template <typename It, typename Compare>
    It Partition(It first, It last, Compare comp)
    {
        auto pivot = *(last - 1);
        auto i     = first;

        for (auto j = first; j < last - 1; ++j)
        {
            if (comp(*j, pivot))
            {
                Swap(*i, *j);
                ++i;
            }
        }

        Swap(*i, *(last - 1));
        return i;
    }

    template <typename It, typename Compare>
    void QuickSort(It first, It last, Compare comp)
    {
        if (last - first <= 1) return;

        It pivot = Partition(first, last, comp);
        QuickSort(first, pivot, comp);
        QuickSort(pivot + 1, last, comp);
    }

    template <typename It, typename Compare>
    void IntroSort(It first, It last, isize depthLimit, Compare comp)
    {
        constexpr usize InsertionSortThreshold = 16;
        usize           size                   = last - first;

        if (size <= InsertionSortThreshold)
        {
            InsertionSort(first, last, comp);
            return;
        }

        if (depthLimit == 0)
        {
            HeapSort(first, last, comp);
            return;
        }

        return QuickSort(first, last, comp);
    }

    template <typename It, typename Compare>
    void IntroSort(It first, It last, Compare comp)
    {
        if (first == last) return;

        isize depthLimit = 2 * static_cast<isize>(Math::Log2(last - first));
        IntroSort(first, last, depthLimit, comp);
    }

    template <typename It, typename Compare = Less<>>
    void Sort(It first, It last, Compare compare)
    {
        IntroSort(first, last, compare);
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::HeapSort;
using Prism::InsertionSort;
using Prism::IntroSort;
using Prism::MergeSort;
using Prism::QuickSort;
using Prism::Sort;
#endif
