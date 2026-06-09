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
    void MedianOfThree(It first, It mid, It last, Compare comp)
    {
        if (comp(*mid, *first)) IteratorSwap(first, mid);
        if (comp(*last, *first)) IteratorSwap(first, last);
        if (comp(*last, *mid)) IteratorSwap(mid, last);
    }

    template <typename It, typename Compare>
    void InsertionSort(It first, It last, Compare comparator)
    {
        if (first == last) return;

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
        isize largest = root;
        isize left    = 2 * root + 1;
        isize right   = 2 * root + 2;

        if (left < size && comp(first[largest], first[left])) largest = left;
        if (right < size && comp(first[largest], first[right])) largest = right;
        if (largest != root)
        {
            IteratorSwap(first + root, first + largest);
            Heapify(first, size, largest, comp);
        }
    }

    template <typename It, typename Compare>
    void HeapSort(It first, It last, Compare comp)
    {
        isize size = last - first;
        if (size <= 1) return;

        for (isize i = size / 2 - 1; i >= 0; --i) Heapify(first, size, i, comp);

        for (isize i = size - 1; i > 0; --i)
        {
            IteratorSwap(first, first + i);
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

    /**
     * @brief Reorders elements such that all elements matching the predicate
     *        precede elements that do not.
     *
     * @tparam It Forward Iterator type.
     * @tparam Predicate Unary function returning bool.
     * @return It Iterator pointing to the first element of the second group.
     */
    template <typename It, typename Predicate>
    constexpr It Partition(It first, It last, Predicate pred)
    {
        first = FindFirstOf(first, last, pred);
        if (first == last) return first;

        for (It i = Next(first); i != last; ++i)
        {
            if (pred(*i))
            {
                IteratorSwap(i, first);
                ++first;
            }
        }

        return first;
    }

    template <typename It, typename Compare>
    It HoarePartition(It first, It last, Compare comp)
    {
        It mid = first + (last - first) / 2;
        MedianOfThree(first, mid, last - 1, comp);
        IteratorSwap(mid, last - 1);

        auto pivot = *(last - 1);
        It   i     = first - 1;
        It   j     = last;

        while (true)
        {
            do { ++i; } while (comp(*i, pivot));
            do { --j; } while (comp(pivot, *j));

            if (i >= j) return i;
            IteratorSwap(i, j);
        }
    }

    template <typename It, typename Compare>
    void QuickSort(It first, It last, Compare comp)
    {
        while (last - first > 1)
        {
            It pivot = HoarePartition(first, last, comp);
            QuickSort(pivot, last, comp);
            last = pivot;
        }
    }

    template <typename It, typename Compare>
    void DoIntroSort(It first, It last, isize depthLimit, Compare comp)
    {
        constexpr usize InsertionSortThreshold = 16;

        while (static_cast<usize>(last - first) > InsertionSortThreshold)
        {
            if (depthLimit == 0)
            {
                HeapSort(first, last, comp);
                return;
            }
            --depthLimit;

            It pivot = HoarePartition(first, last, comp);
            DoIntroSort(pivot, last, depthLimit, comp);
            last = pivot;
        }
    }
    template <typename It, typename Compare>
    void IntroSort(It first, It last, Compare comp)
    {
        if (first == last) return;

        isize depthLimit = 2 * static_cast<isize>(Math::Log2(last - first));
        DoIntroSort(first, last, depthLimit, comp);

        InsertionSort(first, last, comp);
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
