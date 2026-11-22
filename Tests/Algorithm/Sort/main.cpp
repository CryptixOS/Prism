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
import Prism.Debug.Log;
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


// Randomized checks over many sizes and input shapes: every algorithm must
// produce a sorted permutation of its input, MergeSort must also be stable.
namespace
{
    struct Rng
    {
        u64 State = 0x2545f4914f6cdd1dull;
        u64 Next()
        {
            State ^= State << 13;
            State ^= State >> 7;
            State ^= State << 17;
            return State;
        }
        u64 Below(u64 bound) { return bound ? Next() % bound : 0; }
    };

    struct Keyed
    {
        u32 Key;
        u32 Index;
    };

    constexpr usize PatternCount = 10;

    void        Fill(Vector<Keyed>& v, usize n, usize pattern, Rng& rng)
    {
        v.Clear();
        for (usize i = 0; i < n; ++i)
        {
            u64 key = 0;
            switch (pattern)
            {
                case 0: key = rng.Next(); break;             // random
                case 1: key = rng.Below(n ? n : 1); break;   // duplicates
                case 2: key = rng.Below(4); break;           // few unique
                case 3: key = 7; break;                      // all equal
                case 4: key = i; break;                      // sorted
                case 5: key = n - i; break;                  // reverse
                case 6: key = i < n / 2 ? i : n - i; break;  // organ pipe
                case 7: key = i % 17; break;                 // sawtooth
                case 8:                                      // nearly sorted
                    key = rng.Below(20) == 0 ? rng.Below(n ? n : 1) : i;
                    break;
                default: key = (i & 1) ? i : n - i; break;   // interleaved
            }
            v.PushBack({static_cast<u32>(key), static_cast<u32>(i)});
        }
    }

    using KeyedSorter = void (*)(Keyed*, Keyed*, bool (*)(const Keyed&, const Keyed&));
    bool KeyLess(const Keyed& a, const Keyed& b) { return a.Key < b.Key; }

    bool CheckSorted(const Vector<Keyed>& input, Vector<Keyed>& v, bool stable)
    {
        // Every index appears exactly once: a permutation of the input.
        Vector<u8> seen(v.Size(), 0);
        for (usize i = 0; i < v.Size(); ++i)
        {
            if (v[i].Index >= v.Size() || seen[v[i].Index]) return false;
            seen[v[i].Index] = 1;
            if (input[v[i].Index].Key != v[i].Key) return false;
        }
        for (usize i = 1; i < v.Size(); ++i)
        {
            if (v[i].Key < v[i - 1].Key) return false;
            if (stable && v[i].Key == v[i - 1].Key
                && v[i].Index < v[i - 1].Index)
                return false;
        }
        return true;
    }

    bool Stress(KeyedSorter sorter, bool stable, usize maxSize)
    {
        Rng           rng;
        Vector<Keyed> input, work;
        const usize   sizes[] = {0,   1,   2,    3,    4,    5,     7,     8,
                                 15,  16,  17,   23,   24,   25,    31,    32,
                                 33,  63,  64,   65,   100,  127,   128,   129,
                                 200, 255, 256,  257,  500,  1000,  1024,  4096,
                                 5000, 10007, 65536, 100000};
        for (usize n : sizes)
        {
            if (n > maxSize) continue;
            for (usize pattern = 0; pattern < PatternCount; ++pattern)
            {
                Fill(input, n, pattern, rng);
                work = input;
                sorter(work.Raw(), work.Raw() + work.Size(), KeyLess);
                if (!CheckSorted(input, work, stable))
                {
                    Println("sort failed: n = {}, pattern = {}", n, pattern);
                    return false;
                }
            }
        }
        return true;
    }
} // namespace

TEST_CASE(TestSortAlgorithm_Stress)
{
    EXPECT(Stress(Sort, false, ~usize(0)));
    EXPECT(Stress(IntroSort, false, ~usize(0)));
    EXPECT(Stress(QuickSort, false, ~usize(0)));
    EXPECT(Stress(HeapSort, false, ~usize(0)));
    EXPECT(Stress(MergeSort, true, ~usize(0)));
    EXPECT(Stress(InsertionSort, true, 5000));
}

TEST_CASE(TestSortAlgorithm_DefaultComparator)
{
    Vector<int> v = {5, 3, 9, 1, 1, 0, -4};
    Sort(v.begin(), v.end());
    EXPECT(IsSorted(v));
}
