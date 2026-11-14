/*
 * Created by v1tr10l7 on 01.08.2025.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>
export module Prism.Algorithm.Sort;

export import Prism.Core.Concepts;
export import Prism.Core.Iterator;
export import Prism.Core.TypeTraits;
export import Prism.Containers.Array;
export import Prism.Containers.Vector;
export import Prism.Utility.Compare;
export import Prism.Utility.Math;

// Sort is a pattern-defeating quicksort (the pdqsort design by Orson
// Peters): introsort that recognizes sorted, reversed and duplicate-heavy
// input in linear time, partitions branchlessly (BlockQuicksort) for cheap
// trivially copyable types, breaks up adversarial patterns, and falls back
// to heap sort only when it keeps getting bad pivots. On top of pdqsort:
// a leading-run check (sorted/reversed input plus a short tail), flipping
// mostly descending input, pivots from evenly spaced samples (adjacent ones
// are fooled by interleaved sequences), and a partial insertion sort budget
// that scales with the range.
//
// Benchmarks/Sort compares it against std::sort.
//
// None of the in-place sorts allocate, and every recursion goes into the
// smaller half, so stack depth stays O(log n) (they run in the kernel).

export extern "C++"
{
    namespace Prism
    {
        namespace Details
        {
            // Below this many elements insertion sort wins.
            inline constexpr isize SortInsertionThreshold = 24;
            // Above this many elements the pivot is a median of medians.
            inline constexpr isize SortNintherThreshold   = 128;
            // A partial insertion sort may move about one element per element
            // (at least this many) before it gives up on the range being
            // nearly sorted; failing then costs about as much as a partition.
            inline constexpr usize SortPartialInsertionLimit = 8;
            inline constexpr usize SortBlockSize             = 64;
            // Branchless partitioning has a fixed setup cost that small
            // ranges don't earn back.
            inline constexpr isize SortBranchlessThreshold   = 128;
            // A branchless partition with fewer than size / this runs of
            // misplaced elements hands its subtree to the branching one.
            inline constexpr usize SortPredictableRunsRatio  = 16;
            // From this size on, sample whether the input is mostly
            // descending and flip it first if so.
            inline constexpr isize SortDescentSampleThreshold = 256;

            template <typename It>
            using SortValueType = RemoveCvRefType<decltype(*DeclVal<It>())>;

            // Branchless partitioning pays off when elements are cheap to
            // copy around: arithmetic types, pointers and small PODs.
            template <typename T>
            inline constexpr bool SortUseBranchless
                = IsTriviallyCopyableV<T> && sizeof(T) <= 2 * sizeof(void*);

            constexpr isize SortLog2(usize n)
            {
                isize log = 0;
                while (n >>= 1) ++log;
                return log;
            }

            // Swap for the sorts: a member Swap()/swap() when the type has
            // one (std::string, Ref, ...), three plain moves otherwise.
            template <typename T>
            constexpr void SortSwap(T& a, T& b)
            {
                if constexpr (requires { a.Swap(b); })
                {
                    if (&a != &b) a.Swap(b);
                }
                else if constexpr (requires { a.swap(b); })
                {
                    if (&a != &b) a.swap(b);
                }
                else if constexpr (IsTriviallyCopyableV<T>)
                {
                    T tmp = a;
                    a     = b;
                    b     = tmp;
                }
                else
                {
                    if (&a == &b) return;
                    T tmp = Move(a);
                    a     = Move(b);
                    b     = Move(tmp);
                }
            }
            template <typename It>
            constexpr void SortIterSwap(It a, It b)
            {
                SortSwap(*a, *b);
            }

            template <typename It, typename Compare>
            constexpr void Sort2(It a, It b, Compare& comp)
            {
                if (comp(*b, *a)) SortIterSwap(a, b);
            }
            template <typename It, typename Compare>
            constexpr It Median3(It a, It b, It c, Compare& comp)
            {
                if (comp(*a, *b))
                {
                    if (comp(*b, *c)) return b;
                    return comp(*a, *c) ? c : a;
                }
                if (comp(*a, *c)) return a;
                return comp(*b, *c) ? c : b;
            }
            // Leaves *a <= *b <= *c.
            template <typename It, typename Compare>
            constexpr void Sort3(It a, It b, It c, Compare& comp)
            {
                Sort2(a, b, comp);
                Sort2(b, c, comp);
                Sort2(a, b, comp);
            }

            // Inserts each element of [mid, end) into the sorted, non-empty
            // range [begin, mid). Stable.
            template <typename It, typename Compare>
            constexpr void InsertionSortFrom(It begin, It mid, It end,
                                             Compare& comp)
            {
                using T = SortValueType<It>;
                for (It current = mid; current != end; ++current)
                {
                    // A new minimum shifts the whole prefix with a single
                    // comparison (reversed input). Otherwise *begin stops the
                    // scan, so it needs no bounds check.
                    if (comp(*current, *begin))
                    {
                        T value = Move(*current);
                        if constexpr (IsPointerV<It> && IsTriviallyCopyableV<T>)
                            __builtin_memmove(begin + 1, begin,
                                              static_cast<usize>(current - begin)
                                                  * sizeof(T));
                        else
                            for (It it = current; it != begin; --it)
                                *it = Move(*(it - 1));
                        *begin = Move(value);
                        continue;
                    }

                    It sift  = current;
                    It sift1 = current - 1;
                    // Only elements that are out of place get picked up.
                    if (sift1 == begin || !comp(*sift, *sift1)) continue;

                    T value = Move(*sift);
                    do {
                        *sift-- = Move(*sift1);
                    } while (comp(value, *--sift1));
                    *sift = Move(value);
                }
            }

            template <typename It, typename Compare>
            constexpr void InsertionSortImpl(It begin, It end, Compare& comp)
            {
                if (end - begin < 2) return;
                InsertionSortFrom(begin, begin + 1, end, comp);
            }

            // Requires an element before begin that is not greater than any
            // element in [begin, end); that element stops every scan.
            template <typename It, typename Compare>
            constexpr void UnguardedInsertionSort(It begin, It end,
                                                  Compare& comp)
            {
                using T = SortValueType<It>;
                if (begin == end) return;

                for (It current = begin + 1; current != end; ++current)
                {
                    It sift  = current;
                    It sift1 = current - 1;
                    if (!comp(*sift, *sift1)) continue;

                    T value = Move(*sift);
                    do {
                        *sift-- = Move(*sift1);
                    } while (comp(value, *--sift1));
                    *sift = Move(value);
                }
            }

            // Insertion sort that gives up once it has made more element moves
            // than the range has elements (or SortPartialInsertionLimit).
            // Returns whether the range ended up sorted.
            template <typename It, typename Compare>
            constexpr bool PartialInsertionSort(It begin, It end, Compare& comp)
            {
                using T = SortValueType<It>;
                if (begin == end) return true;

                const usize size  = static_cast<usize>(end - begin);
                const usize limit = size > SortPartialInsertionLimit
                                      ? size
                                      : SortPartialInsertionLimit;
                usize       moves = 0;
                for (It current = begin + 1; current != end; ++current)
                {
                    It sift  = current;
                    It sift1 = current - 1;
                    if (!comp(*sift, *sift1)) continue;

                    T value = Move(*sift);
                    do {
                        *sift-- = Move(*sift1);
                    } while (sift != begin && comp(value, *--sift1));
                    *sift = Move(value);

                    moves += current - sift;
                    if (moves > limit) return false;
                }
                return true;
            }

            // Floyd's sift-down: walk the hole down along the larger child
            // (one comparison per level), then sift value back up from the
            // bottom, where it usually belongs anyway.
            template <typename It, typename Compare, typename T>
            constexpr void SiftDown(It first, isize size, isize hole, T value,
                                    Compare& comp)
            {
                const isize top   = hole;
                isize       child = hole;

                while (child < (size - 1) / 2)
                {
                    child = 2 * (child + 1);
                    if (comp(*(first + child), *(first + (child - 1)))) --child;
                    *(first + hole) = Move(*(first + child));
                    hole            = child;
                }
                if ((size & 1) == 0 && child == (size - 2) / 2)
                {
                    child           = 2 * (child + 1);
                    *(first + hole) = Move(*(first + (child - 1)));
                    hole            = child - 1;
                }

                isize parent = (hole - 1) / 2;
                while (hole > top && comp(*(first + parent), value))
                {
                    *(first + hole) = Move(*(first + parent));
                    hole            = parent;
                    parent          = (hole - 1) / 2;
                }
                *(first + hole) = Move(value);
            }

            template <typename It, typename Compare>
            constexpr void HeapSortImpl(It first, It last, Compare& comp)
            {
                using T    = SortValueType<It>;
                isize size = last - first;
                if (size < 2) return;

                for (isize parent = (size - 2) / 2; parent >= 0; --parent)
                {
                    T value = Move(*(first + parent));
                    SiftDown(first, size, parent, Move(value), comp);
                }
                for (isize end = size - 1; end > 0; --end)
                {
                    T value        = Move(*(first + end));
                    *(first + end) = Move(*first);
                    SiftDown(first, end, 0, Move(value), comp);
                }
            }

            template <typename It>
            struct PartitionResult
            {
                It    Pivot;
                bool  AlreadyPartitioned;
                // Branchless partition only: runs of consecutive misplaced
                // elements, about the number of mispredictions a branching
                // partition would have had.
                usize Runs = 0;
            };

            // Number of runs of consecutive values in offsets[0, count).
            constexpr usize CountOffsetRuns(const u8* offsets, usize count)
            {
                if (count == 0) return 0;
                usize runs = 1;
                for (usize i = 1; i < count; ++i)
                    runs += offsets[i] != offsets[i - 1] + 1;
                return runs;
            }

            // Partitions [begin, end) around the pivot *begin. Elements equal
            // to the pivot go right. Needs an element >= pivot somewhere after
            // begin (the median selection guarantees one).
            template <typename It, typename Compare>
            constexpr PartitionResult<It> PartitionRight(It begin, It end,
                                                         Compare& comp)
            {
                using T  = SortValueType<It>;
                T  pivot = Move(*begin);
                It first = begin;
                It last  = end;

                while (comp(*++first, pivot));
                // No element was smaller than the pivot: nothing stops the
                // scan from the right, so bound it.
                if (first - 1 == begin)
                    while (first < last && !comp(*--last, pivot));
                else
                    while (!comp(*--last, pivot));

                // If the scans already crossed, nothing has to move.
                const bool alreadyPartitioned = first >= last;
                while (first < last)
                {
                    SortIterSwap(first, last);
                    while (comp(*++first, pivot));
                    while (!comp(*--last, pivot));
                }

                It pivotPos = first - 1;
                *begin      = Move(*pivotPos);
                *pivotPos   = Move(pivot);
                return {pivotPos, alreadyPartitioned};
            }

            // Moves num misplaced elements from the left block to the right
            // block and the other way round, as one cycle of moves rather
            // than num swaps. Equal counts use real swaps instead, which keeps
            // descending input linear.
            template <typename It>
            constexpr void SwapOffsets(It first, It last, const u8* offsetsL,
                                       const u8* offsetsR, usize num,
                                       bool useSwaps)
            {
                using T = SortValueType<It>;
                if (useSwaps)
                {
                    for (usize i = 0; i < num; ++i)
                        SortIterSwap(first + offsetsL[i], last - offsetsR[i]);
                    return;
                }
                if (num == 0) return;

                It l   = first + offsetsL[0];
                It r   = last - offsetsR[0];
                T  tmp = Move(*l);
                *l     = Move(*r);
                for (usize i = 1; i < num; ++i)
                {
                    l  = first + offsetsL[i];
                    *r = Move(*l);
                    r  = last - offsetsR[i];
                    *l = Move(*r);
                }
                *r = Move(tmp);
            }

            // PartitionRight without data-dependent branches, after
            // "BlockQuicksort: How Branch Mispredictions don't affect
            // Quicksort" (Edelkamp, Weiss): each side records the offsets of
            // the elements that belong on the other side into a small block,
            // using the comparison result as an increment, then the two
            // blocks are swapped pairwise.
            template <typename It, typename Compare>
            constexpr PartitionResult<It>
            PartitionRightBranchless(It begin, It end, Compare& comp)
            {
                using T  = SortValueType<It>;
                T  pivot = Move(*begin);
                It first = begin;
                It last  = end;

                while (comp(*++first, pivot));
                if (first - 1 == begin)
                    while (first < last && !comp(*--last, pivot));
                else
                    while (!comp(*--last, pivot));

                const bool alreadyPartitioned = first >= last;
                usize      runs               = 0;
                if (!alreadyPartitioned)
                {
                    SortIterSwap(first, last);
                    ++first;

                    alignas(64) u8 offsetsL[SortBlockSize];
                    alignas(64) u8 offsetsR[SortBlockSize];
                    It             offsetsLBase = first;
                    It             offsetsRBase = last;
                    usize          numL = 0, numR = 0, startL = 0, startR = 0;

                    while (first < last)
                    {
                        // How many unknown elements each block looks at: an
                        // empty block refills, a block with leftovers waits.
                        const usize numUnknown = static_cast<usize>(last - first);
                        const usize leftSplit
                            = numL == 0 ? (numR == 0 ? numUnknown / 2 : numUnknown)
                                        : 0;
                        const usize rightSplit
                            = numR == 0 ? numUnknown - leftSplit : 0;

                        if (leftSplit >= SortBlockSize)
                        {
                            for (usize i = 0; i < SortBlockSize;)
                            {
                                offsetsL[numL] = static_cast<u8>(i++);
                                numL += !comp(*first, pivot);
                                ++first;
                                offsetsL[numL] = static_cast<u8>(i++);
                                numL += !comp(*first, pivot);
                                ++first;
                                offsetsL[numL] = static_cast<u8>(i++);
                                numL += !comp(*first, pivot);
                                ++first;
                                offsetsL[numL] = static_cast<u8>(i++);
                                numL += !comp(*first, pivot);
                                ++first;
                            }
                        }
                        else
                        {
                            for (usize i = 0; i < leftSplit;)
                            {
                                offsetsL[numL] = static_cast<u8>(i++);
                                numL += !comp(*first, pivot);
                                ++first;
                            }
                        }

                        if (leftSplit) runs += CountOffsetRuns(offsetsL, numL);

                        if (rightSplit >= SortBlockSize)
                        {
                            for (usize i = 0; i < SortBlockSize;)
                            {
                                offsetsR[numR] = static_cast<u8>(++i);
                                numR += comp(*--last, pivot);
                                offsetsR[numR] = static_cast<u8>(++i);
                                numR += comp(*--last, pivot);
                                offsetsR[numR] = static_cast<u8>(++i);
                                numR += comp(*--last, pivot);
                                offsetsR[numR] = static_cast<u8>(++i);
                                numR += comp(*--last, pivot);
                            }
                        }
                        else
                        {
                            for (usize i = 0; i < rightSplit;)
                            {
                                offsetsR[numR] = static_cast<u8>(++i);
                                numR += comp(*--last, pivot);
                            }
                        }

                        if (rightSplit) runs += CountOffsetRuns(offsetsR, numR);

                        const usize num = numL < numR ? numL : numR;
                        SwapOffsets(offsetsLBase, offsetsRBase,
                                    offsetsL + startL, offsetsR + startR, num,
                                    numL == numR);
                        numL -= num;
                        numR -= num;
                        startL += num;
                        startR += num;

                        if (numL == 0)
                        {
                            startL       = 0;
                            offsetsLBase = first;
                        }
                        if (numR == 0)
                        {
                            startR       = 0;
                            offsetsRBase = last;
                        }
                    }

                    // One block may still hold misplaced elements: move them
                    // to the boundary.
                    if (numL)
                    {
                        const u8* offsets = offsetsL + startL;
                        while (numL--)
                            SortIterSwap(offsetsLBase + offsets[numL], --last);
                        first = last;
                    }
                    if (numR)
                    {
                        const u8* offsets = offsetsR + startR;
                        while (numR--)
                        {
                            SortIterSwap(offsetsRBase - offsets[numR], first);
                            ++first;
                        }
                        last = first;
                    }
                }

                It pivotPos = first - 1;
                *begin      = Move(*pivotPos);
                *pivotPos   = Move(pivot);
                return {pivotPos, alreadyPartitioned, runs};
            }

            // Like PartitionRight, but elements equal to the pivot go left.
            // Used when the pivot equals the element just before the range,
            // i.e. it's the smallest value here: the whole run of pivot
            // values ends up in place and is never looked at again, so many
            // duplicates cost linear time.
            template <typename It, typename Compare>
            constexpr It PartitionLeft(It begin, It end, Compare& comp)
            {
                using T  = SortValueType<It>;
                T  pivot = Move(*begin);
                It first = begin;
                It last  = end;

                while (comp(pivot, *--last));
                if (last + 1 == end)
                    while (first < last && !comp(pivot, *++first));
                else
                    while (!comp(pivot, *++first));

                while (first < last)
                {
                    SortIterSwap(first, last);
                    while (comp(pivot, *--last));
                    while (!comp(pivot, *++first));
                }

                It pivotPos = last;
                *begin      = Move(*pivotPos);
                *pivotPos   = Move(pivot);
                return pivotPos;
            }

            // badAllowed: how many more badly unbalanced partitions to
            // tolerate before switching to heap sort (< 0: never).
            // leftmost: whether [begin, end) is the leftmost part of the
            // input; otherwise *(begin - 1) is <= everything in it.
            // branchless: whether this subtree still uses the branchless
            // partition; it switches to the branching one for data whose
            // comparisons turn out predictable.
            template <bool Branchless, typename It, typename Compare>
            constexpr void PdqSortLoop(It begin, It end, Compare& comp,
                                       isize badAllowed, bool leftmost,
                                       bool branchless = Branchless)
            {
                while (true)
                {
                    const isize size = end - begin;
                    if (size < SortInsertionThreshold)
                    {
                        if (leftmost) InsertionSortImpl(begin, end, comp);
                        else UnguardedInsertionSort(begin, end, comp);
                        return;
                    }

                    // Pivot to *begin: median of 3 quartiles, or a pseudomedian
                    // of 9 evenly spaced samples. They're compared in place and
                    // only the pivot moves, so runs stay intact.
                    const isize half = size / 2;
                    if (size > SortNintherThreshold)
                    {
                        const isize step = size / 8;
                        It          s1 = begin + step, s2 = s1 + step;
                        It          s3 = s2 + step, s4 = s3 + step;
                        It          s5 = s4 + step, s6 = s5 + step;
                        It          s7 = s6 + step;
                        It          pivot = Median3(Median3(begin, s1, s2, comp),
                                                    Median3(s3, s4, s5, comp),
                                                    Median3(s6, s7, end - 1, comp), comp);
                        SortIterSwap(begin, pivot);
                    }
                    else
                    {
                        const isize quarter = size / 4;
                        SortIterSwap(begin, Median3(begin + quarter, begin + half,
                                                    begin + (half + quarter), comp));
                    }

                    // The pivot equals the element before the range, so it's
                    // the minimum here: put every copy of it in place at once.
                    if (!leftmost && !comp(*(begin - 1), *begin))
                    {
                        begin = PartitionLeft(begin, end, comp) + 1;
                        continue;
                    }

                    PartitionResult<It> result;
                    if constexpr (Branchless)
                    {
                        if (branchless && size > SortBranchlessThreshold)
                        {
                            result = PartitionRightBranchless(begin, end, comp);
                            // Few runs of misplaced elements: a branching
                            // partition would predict well, and is cheaper.
                            if (result.Runs * SortPredictableRunsRatio
                                < static_cast<usize>(size))
                                branchless = false;
                        }
                        else result = PartitionRight(begin, end, comp);
                    }
                    else result = PartitionRight(begin, end, comp);
                    const It   pivotPos           = result.Pivot;
                    const bool alreadyPartitioned = result.AlreadyPartitioned;

                    const isize leftSize  = pivotPos - begin;
                    const isize rightSize = end - (pivotPos + 1);
                    const bool  unbalanced
                        = leftSize < size / 8 || rightSize < size / 8;

                    if (unbalanced)
                    {
                        if (badAllowed >= 0 && --badAllowed == 0)
                        {
                            HeapSortImpl(begin, end, comp);
                            return;
                        }

                        // Swap a few elements around to break up whatever
                        // pattern made the pivot bad.
                        if (leftSize >= SortInsertionThreshold)
                        {
                            const isize q = leftSize / 4;
                            SortIterSwap(begin, begin + q);
                            SortIterSwap(pivotPos - 1, pivotPos - q);
                            if (leftSize > SortNintherThreshold)
                            {
                                SortIterSwap(begin + 1, begin + (q + 1));
                                SortIterSwap(begin + 2, begin + (q + 2));
                                SortIterSwap(pivotPos - 2, pivotPos - (q + 1));
                                SortIterSwap(pivotPos - 3, pivotPos - (q + 2));
                            }
                        }
                        if (rightSize >= SortInsertionThreshold)
                        {
                            const isize q = rightSize / 4;
                            SortIterSwap(pivotPos + 1, pivotPos + (1 + q));
                            SortIterSwap(end - 1, end - q);
                            if (rightSize > SortNintherThreshold)
                            {
                                SortIterSwap(pivotPos + 2, pivotPos + (2 + q));
                                SortIterSwap(pivotPos + 3, pivotPos + (3 + q));
                                SortIterSwap(end - 2, end - (1 + q));
                                SortIterSwap(end - 3, end - (2 + q));
                            }
                        }
                    }
                    // A balanced partition that moved nothing: the range is
                    // probably sorted already, so try to finish it cheaply.
                    else if (alreadyPartitioned
                             && PartialInsertionSort(begin, pivotPos, comp)
                             && PartialInsertionSort(pivotPos + 1, end, comp))
                        return;

                    // Recurse into the smaller side, loop on the larger one.
                    if (leftSize < rightSize)
                    {
                        PdqSortLoop<Branchless>(begin, pivotPos, comp,
                                                badAllowed, leftmost, branchless);
                        begin    = pivotPos + 1;
                        leftmost = false;
                    }
                    else
                    {
                        PdqSortLoop<Branchless>(pivotPos + 1, end, comp,
                                                badAllowed, false, branchless);
                        end = pivotPos;
                    }
                }
            }

            template <typename It>
            constexpr void ReverseRange(It first, It last)
            {
                while (first < last && first < --last) SortIterSwap(first++, last);
            }
            template <typename It, typename Compare>
            constexpr void PdqSortLarge(It begin, It end, Compare& comp,
                                        bool heapFallback);

            // Small inputs are handled inline, everything else out of line.
            template <typename It, typename Compare>
            constexpr void PdqSort(It begin, It end, Compare& comp,
                                   bool heapFallback)
            {
                if (end - begin < SortInsertionThreshold)
                {
                    InsertionSortImpl(begin, end, comp);
                    return;
                }
                PdqSortLarge(begin, end, comp, heapFallback);
            }

            template <typename It, typename Compare>
            [[gnu::noinline]] constexpr void
            PdqSortLarge(It begin, It end, Compare& comp, bool heapFallback)
            {
                // A leading run that covers (almost) everything: sorted or
                // strictly descending input, maybe with a few elements
                // appended, is finished in linear time.
                {
                    const isize n   = end - begin;
                    isize       run = 2;
                    const bool  descending = comp(*(begin + 1), *begin);
                    if (descending)
                        while (run < n && comp(*(begin + run), *(begin + (run - 1)))) ++run;
                    else
                        while (run < n && !comp(*(begin + run), *(begin + (run - 1)))) ++run;

                    if (n - run <= static_cast<isize>(SortPartialInsertionLimit))
                    {
                        if (descending) ReverseRange(begin, begin + run);
                        InsertionSortFrom(begin, begin + run, end, comp);
                        return;
                    }

                    // Mostly descending (a reversed input with a few changes):
                    // flip it, nearly sorted input is much cheaper.
                    if (n >= SortDescentSampleThreshold)
                    {
                        const isize step    = (n - 1) / 16;
                        usize       descent = 0;
                        for (isize k = 0; k < 16; ++k)
                            descent += comp(*(begin + (k * step + 1)), *(begin + k * step));
                        if (descent >= 14) ReverseRange(begin, end);
                    }
                }
                const isize badAllowed
                    = heapFallback ? SortLog2(static_cast<usize>(end - begin))
                                   : -1;
                PdqSortLoop<SortUseBranchless<SortValueType<It>>>(
                    begin, end, comp, badAllowed, true);
            }

            // Scratch space for count elements, made by moving the first count
            // elements out and back, so T only needs to be movable.
            template <typename It>
            Vector<SortValueType<It>> MakeSortBuffer(It first, usize count)
            {
                Vector<SortValueType<It>> buffer;
                buffer.Reserve(count);
                for (usize i = 0; i < count; ++i)
                    buffer.EmplaceBack(Move(*(first + i)));
                for (usize i = 0; i < count; ++i)
                    *(first + i) = Move(buffer[i]);
                return buffer;
            }

            // Stable merge of [first, mid) and [mid, last) with [first, mid)
            // moved out to buffer (which holds at least mid - first elements).
            template <typename It, typename Buffer, typename Compare>
            constexpr void MergeWithBuffer(It first, It mid, It last,
                                           Buffer* buffer, Compare& comp)
            {
                // Already in order: nothing to merge.
                if (!comp(*mid, *(mid - 1))) return;

                Buffer* bufferEnd = buffer;
                for (It it = first; it != mid; ++it) *bufferEnd++ = Move(*it);

                Buffer* l   = buffer;
                It      r   = mid;
                It      out = first;
                while (l != bufferEnd && r != last)
                {
                    if (comp(*r, *l)) *out++ = Move(*r++);
                    else *out++ = Move(*l++);
                }
                // Whatever is left of the right half is already in place.
                while (l != bufferEnd) *out++ = Move(*l++);
            }

            template <typename It, typename Buffer, typename Compare>
            constexpr void MergeSortImpl(It first, It last, Buffer* buffer,
                                         Compare& comp)
            {
                const isize size = last - first;
                if (size <= SortInsertionThreshold)
                {
                    // Insertion sort is stable.
                    InsertionSortImpl(first, last, comp);
                    return;
                }

                It mid = first + size / 2;
                MergeSortImpl(first, mid, buffer, comp);
                MergeSortImpl(mid, last, buffer, comp);
                MergeWithBuffer(first, mid, last, buffer, comp);
            }
        } // namespace Details

        /**
         * @brief Sorts the three elements so that *first <= *mid <= *last.
         */
        template <typename It, typename Compare>
        constexpr void MedianOfThree(It first, It mid, It last, Compare comp)
        {
            Details::Sort3(first, mid, last, comp);
        }

        /**
         * @brief Stable insertion sort. O(n^2), but the fastest choice for
         *        small or nearly sorted ranges.
         */
        template <typename It, typename Compare = Less<>>
        constexpr void InsertionSort(It first, It last, Compare comparator = {})
        {
            Details::InsertionSortImpl(first, last, comparator);
        }

        /**
         * @brief Restores the heap property for the subtree at root, in a max
         *        heap of size elements starting at first.
         */
        template <typename It, typename Compare>
        constexpr void Heapify(It first, isize size, isize root, Compare comp)
        {
            if (root >= size) return;
            auto value = Move(*(first + root));
            Details::SiftDown(first, size, root, Move(value), comp);
        }

        /**
         * @brief Unstable heap sort. O(n log n) in every case, in place,
         *        without recursion.
         */
        template <typename It, typename Compare = Less<>>
        constexpr void HeapSort(It first, It last, Compare comp = {})
        {
            Details::HeapSortImpl(first, last, comp);
        }

        /**
         * @brief Stable merge of the sorted ranges [first, mid) and
         *        [mid, last). Allocates a buffer for [first, mid).
         */
        template <typename It, typename Compare>
        void Merge(It first, It mid, It last, Compare comp)
        {
            if (first == mid || mid == last) return;

            auto buffer = Details::MakeSortBuffer(
                first, static_cast<usize>(mid - first));
            Details::MergeWithBuffer(first, mid, last, buffer.Raw(), comp);
        }

        /**
         * @brief Stable merge sort. O(n log n), allocates one buffer of
         *        (last - first) / 2 elements.
         */
        template <typename It, typename Compare = Less<>>
        void MergeSort(It first, It last, Compare comp = {})
        {
            const isize size = last - first;
            if (size < 2) return;
            if (size <= Details::SortInsertionThreshold)
            {
                Details::InsertionSortImpl(first, last, comp);
                return;
            }

            auto buffer = Details::MakeSortBuffer(
                first, static_cast<usize>(size - size / 2));

            Details::MergeSortImpl(first, last, buffer.Raw(), comp);
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
            while (first != last && pred(*first)) ++first;
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

        /**
         * @brief Partitions [first, last) around the median of its first,
         *        middle and last elements. Returns an iterator such that
         *        everything before it is <= everything from it on; both sides
         *        are non-empty when last - first >= 2.
         */
        template <typename It, typename Compare>
        It HoarePartition(It first, It last, Compare comp)
        {
            if (last - first < 2) return last;
            if (last - first < 3)
            {
                Details::Sort2(first, first + 1, comp);
                return first + 1;
            }

            It mid = first + (last - first) / 2;
            Details::Sort3(first, mid, last - 1, comp);
            IteratorSwap(first, mid);
            auto result = Details::PartitionRight(first, last, comp);
            // The pivot sits at result.Pivot; split right after it if that
            // keeps the left side non-empty.
            return result.Pivot == first ? result.Pivot + 1 : result.Pivot;
        }

        /**
         * @brief Unstable quicksort (pattern-defeating, without the heap sort
         *        fallback): O(n log n) on average, O(n^2) worst case.
         */
        template <typename It, typename Compare = Less<>>
        void QuickSort(It first, It last, Compare comp = {})
        {
            Details::PdqSort(first, last, comp, false);
        }

        /**
         * @brief Unstable introsort: quicksort that switches to heap sort
         *        when partitions keep going bad. O(n log n) worst case.
         */
        template <typename It, typename Compare = Less<>>
        void IntroSort(It first, It last, Compare comp = {})
        {
            Details::PdqSort(first, last, comp, true);
        }

        /**
         * @brief Sorts [first, last). Unstable, in place, O(n log n) worst
         *        case, O(n) for sorted, reversed or all-equal input.
         */
        template <typename It, typename Compare = Less<>>
        void Sort(It first, It last, Compare compare = {})
        {
            Details::PdqSort(first, last, compare, true);
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
}
