/*
 * Created by v1tr10l7 on 31.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Concepts.hpp>

namespace Prism
{
    /**
     * @brief Finds the first element equal to the given value.
     * @tparam Iterator Type of the iterator.
     * @tparam T Type of the value to compare against.
     * @param first Start of the range.
     * @param last End of the range.
     * @param value Value to search for.
     * @return Iterator to the first match, or last if not found.
     */
    template <typename Iterator, typename T>
    PM_NODISCARD constexpr Iterator Find(Iterator first, Iterator last,
                                         const T& value)
    {
        for (; first != last; ++first)
            if (*first == value) return first;
        return last;
    }

    /**
     * @brief Finds the first element satisfying a predicate.
     * @tparam Iterator Type of the iterator.
     * @tparam UnaryPredicate Type of the predicate function.
     * @param first Start of the range.
     * @param last End of the range.
     * @param pred Predicate taking an element and returning bool.
     * @return Iterator to the first match, or last if not found.
     */
    template <typename Iterator, typename UnaryPredicate>
    PM_NODISCARD constexpr Iterator FindIf(Iterator first, Iterator last,
                                           UnaryPredicate&& pred)
    {
        for (; first != last; ++first)
            if (pred(*first)) return first;
        return last;
    }
    /**
     * @brief Finds the first element that does NOT satisfy a predicate.
     * @tparam Iterator Type of the iterator.
     * @tparam UnaryPredicate Type of the predicate function.
     * @param first Start of the range.
     * @param last End of the range.
     * @param pred Predicate taking an element and returning bool.
     * @return Iterator to the first non-matching element, or last if all match.
     */
    template <typename Iterator, typename UnaryPredicate>
    PM_NODISCARD constexpr Iterator FindIfNot(Iterator first, Iterator last,
                                              UnaryPredicate&& pred)
    {
        for (; first != last; ++first)
            if (!pred(*first)) return first;
        return last;
    }
    /**
     * @brief Finds the first element for which an iterator-based predicate is
     * true.
     * @tparam Iterator Type of the iterator.
     * @tparam Predicate Type of the predicate function.
     * @param first Start of the range.
     * @param last End of the range.
     * @param pred Predicate taking the iterator itself.
     * @return Iterator to the first match, or last if not found.
     */
    template <typename Iterator, typename Predicate>
    PM_NODISCARD constexpr Iterator FindIfIter(Iterator first, Iterator last,
                                               Predicate&& pred)
    {
        for (; first != last; ++first)
            if (pred(first)) return first;
        return last;
    }
    /**
     * @brief Finds first element equal to value using an iterator-aware
     * comparator.
     * @tparam Iterator Type of the iterator.
     * @tparam Comparator Type of the comparator function.
     * @tparam T Type of the value.
     * @param first Start of the range.
     * @param last End of the range.
     * @param value Value to compare.
     * @param comp Comparator taking (Iterator, T&).
     */
    template <typename Iterator, typename Comparator, typename T>
    PM_NODISCARD constexpr Iterator FindValueIter(Iterator first, Iterator last,
                                                  T& value, Comparator comp)
    /**
     * @brief Checks if any element satisfies the predicate.
     */
    {
        for (; first != last; ++first)
            if (comp(first, value)) return first;
        return last;
    }
    /**
     * @brief Find first element matching another iterator using an
     * iterator-aware comparator.
     * @tparam Iterator Main iterator type.
     * @tparam Comparator Type of comparator taking (Iterator, It2).
     * @tparam It2 Target iterator type.
     */
    template <typename Iterator, typename Comparator, typename It2>
    PM_NODISCARD constexpr Iterator
    FindIteratorIter(Iterator first, Iterator last, Comparator comp, It2 it2)
    {
        for (; first != last; ++first)
            if (comp(first, it2)) return first;
        return last;
    }

    /**
     * @brief Finds the first occurrence of any element from a second range.
     */
    template <typename Iterator1, typename Iterator2>
    PM_NODISCARD constexpr Iterator1
    FindFirstOf(Iterator1 first, Iterator1 last, Iterator2 s_first,
                Iterator2 s_last)
    {
        for (; first != last; ++first)
            for (Iterator2 it = s_first; it != s_last; ++it)
                if (*first == *it) return first;
        return last;
    }
    /**
     * @brief Finds the first occurrence of any element from a second range
     * using a comparator.
     */
    template <typename Iterator1, typename Iterator2, typename Comparator>
    PM_NODISCARD constexpr Iterator1
    FindFirstOf(Iterator1 first, Iterator1 last, Iterator2 s_first,
                Iterator2 s_last, Comparator&& comp)
    {
        for (; first != last; ++first)
            for (Iterator2 it = s_first; it != s_last; ++it)
                if (comp(*first, *it)) return first;
        return last;
    }
    /**
     * @brief Iterator-aware version of FindFirstOf.
     */
    template <typename Iterator1, typename Iterator2, typename Comparator>
    PM_NODISCARD constexpr Iterator1
    FindFirstOfIter(Iterator1 first, Iterator1 last, Iterator2 s_first,
                    Iterator2 s_last, Comparator&& comp)
    {
        for (; first != last; ++first)
            for (Iterator2 it = s_first; it != s_last; ++it)
                if (comp(first, it)) return first;
        return last;
    }

    /**
     * @brief Finds the last element satisfying a predicate (linear search from
     * start).
     * @tparam Iterator Type of the iterator.
     * @tparam UnaryPredicate Type of the predicate function.
     * @param first Start of the range.
     * @param last End of the range.
     * @param pred Predicate taking an element and returning bool.
     * @return Iterator to the last match found, or last if none found.
     */
    template <typename Iterator, typename UnaryPredicate>
    PM_NODISCARD constexpr Iterator FindLastIf(Iterator first, Iterator last,
                                               UnaryPredicate&& pred)
    {
        Iterator result = last;
        for (; first != last; ++first)
            if (pred(*first)) result = first;
        return result;
    }
    /**
     * @brief Checks if any element satisfies the predicate.
     */
    template <typename Iterator, typename UnaryPredicate>
    constexpr bool AnyOf(Iterator first, Iterator last, UnaryPredicate&& pred)
    {
        for (; first != last; ++first)
            if (pred(*first)) return true;
        return false;
    }
    /**
     * @brief Checks if any element matches the value using a custom comparator.
     */
    template <typename Iterator, typename Comparator, typename T>
    constexpr bool AnyOf(Iterator first, Iterator last, Comparator&& comp,
                         const T& value)
    {
        for (; first != last; ++first)
            if (comp(*first, value)) return true;
        return false;
    }
    /**
     * @brief Iterator-aware version of AnyOf.
     */
    template <typename Iterator, typename IteratorPredicate>
    constexpr bool AnyOfIter(Iterator first, Iterator last,
                             IteratorPredicate&& pred)
    {
        for (; first != last; ++first)
            if (pred(first)) return true;
        return false;
    }
    /**
     * @brief Checks if all elements satisfy the predicate.
     */
    template <typename Iterator, typename UnaryPredicate>
    constexpr bool AllOf(Iterator first, Iterator last, UnaryPredicate&& pred)
    {
        for (; first != last; ++first)
            if (!pred(*first)) return false;
        return true;
    }
    /**
     * @brief Checks if all elements match the value using a custom comparator.
     */
    template <typename Iterator, typename Comparator, typename T>
    constexpr bool AllOf(Iterator first, Iterator last, Comparator&& comp,
                         const T& value)
    {
        for (; first != last; ++first)
            if (!comp(*first, value)) return false;
        return true;
    }
    /**
     * @brief Iterator-aware version of AllOf.
     */
    template <typename Iterator, typename IteratorPredicate>
    constexpr bool AllOfIter(Iterator first, Iterator last,
                             IteratorPredicate&& pred)
    {
        for (; first != last; ++first)
            if (!pred(first)) return false;
        return true;
    }
    /**
     * @brief Checks if no elements satisfy the predicate.
     */
    template <typename Iterator, typename UnaryPredicate>
    constexpr bool NoneOf(Iterator first, Iterator last, UnaryPredicate&& pred)
    {
        for (; first != last; ++first)
            if (pred(*first)) return false;
        return true;
    }
    /**
     * @brief Checks if no elements match the value using a custom comparator.
     */
    template <typename Iterator, typename Comparator, typename T>
    constexpr bool NoneOf(Iterator first, Iterator last, Comparator&& comp,
                          const T& value)
    {
        for (; first != last; ++first)
            if (comp(*first, value)) return false;
        return true;
    }
    /**
     * @brief Iterator-aware version of NoneOf.
     */
    template <typename Iterator, typename IteratorPredicate>
    constexpr bool NoneOfIter(Iterator first, Iterator last,
                              IteratorPredicate&& pred)
    {
        for (; first != last; ++first)
            if (pred(first)) return false;
        return true;
    }

    /**
     * @brief Counts occurrences of a value in a range.
     * @return Number of matches as @ref usize.
     */
    template <typename Iterator, typename T>
    constexpr usize Count(Iterator first, Iterator last, const T& value)
    {
        usize cnt = 0;
        for (; first != last; ++first)
            if (*first == value) ++cnt;
        return cnt;
    }
    /**
     * @brief Counts elements satisfying a predicate.
     */
    template <typename Iterator, typename UnaryPredicate>
    constexpr usize CountIf(Iterator first, Iterator last,
                            UnaryPredicate&& pred)
    {
        usize cnt = 0;
        for (; first != last; ++first)
            if (pred(*first)) ++cnt;
        return cnt;
    }
    /**
     * @brief Counts elements matching a value using a custom comparator.
     */
    template <typename Iterator, typename Comparator, typename T>
    constexpr usize CountIf(Iterator first, Iterator last, Comparator&& comp,
                            const T& value)
    {
        usize cnt = 0;
        for (; first != last; ++first)
            if (comp(*first, value)) ++cnt;
        return cnt;
    }
    /**
     * @brief Iterator-aware version of CountIf.
     */
    template <typename Iterator, typename IteratorPredicate>
    constexpr usize CountIfIter(Iterator first, Iterator last,
                                IteratorPredicate&& pred)
    {
        usize cnt = 0;
        for (; first != last; ++first)
            if (pred(first)) ++cnt;
        return cnt;
    }

    /**
     * @brief Reorders range so elements for which pred is false are at the
     * beginning.
     * @note Uses Move semantics for efficiency.
     * @return Iterator to the new logical end of the range.
     */
    template <typename Iterator, typename UnaryPredicate>
    constexpr Iterator RemoveIf(Iterator first, Iterator last,
                                UnaryPredicate&& pred)
    {
        Iterator result = first;
        for (; first != last; ++first)
        {
            if (!pred(*first))
            {
                *result = Move(*first); // move surviving element forward
                ++result;
            }
        }
        return result; // new logical end
    }
    /**
     * @brief Reorders range removing elements matching a value via comparator.
     */
    template <typename Iterator, typename Comparator, typename T>
    constexpr Iterator RemoveIf(Iterator first, Iterator last,
                                Comparator&& comp, const T& value)
    {
        Iterator result = first;
        for (; first != last; ++first)
        {
            if (!comp(*first, value))
            {
                *result = Move(*first);
                ++result;
            }
        }
        return result;
    }
    /**
     * @brief Iterator-aware version of RemoveIf.
     */
    template <typename Iterator, typename IteratorPredicate>
    constexpr Iterator RemoveIfIter(Iterator first, Iterator last,
                                    IteratorPredicate&& pred)
    {
        Iterator result = first;
        for (; first != last; ++first)
        {
            if (!pred(first))
            {
                *result = Move(*first);
                ++result;
            }
        }
        return result;
    }
    /**
     * @brief Removes elements satisfying a predicate from a container.
     * @tparam Container A container supporting .begin(), .end(), and .erase().
     */
    template <typename Container, typename UnaryPredicate>
    constexpr void EraseIf(Container& c, UnaryPredicate&& pred)
    {
        auto newEnd = RemoveIf(c.begin(), c.end(), Move(pred));
        c.Erase(newEnd, c.end());
    }

    /**
     * @brief Finds element where the projected value satisfies a predicate.
     * @tparam Projection Function to transform the element before comparison.
     */
    template <typename Iterator, typename UnaryPredicate, typename Projection>
    constexpr Iterator FindIfProj(Iterator first, Iterator last,
                                  UnaryPredicate&& pred, Projection&& proj)
    {
        for (; first != last; ++first)
            if (pred(proj(*first))) return first;
        return last;
    }
    /**
     * @brief Finds element where the projected value matches a value via
     * comparator.
     */
    template <typename Iterator, typename Comparator, typename T,
              typename Projection>
    constexpr Iterator FindIfProj(Iterator first, Iterator last,
                                  Comparator&& comp, const T& value,
                                  Projection&& proj)
    {
        for (; first != last; ++first)
            if (comp(proj(*first), value)) return first;
        return last;
    }

    /**
     * @brief Counts elements NOT satisfying a predicate.
     */
    template <typename Iterator, typename UnaryPredicate>
    constexpr usize CountIfNot(Iterator first, Iterator last,
                               UnaryPredicate&& pred)
    {
        usize cnt = 0;
        for (; first != last; ++first)
            if (!pred(*first)) ++cnt;
        return cnt;
    }

    /**
     * @brief Counts elements NOT matching a value via comparator.
     */
    template <typename Iterator, typename Comparator, typename T>
    constexpr usize CountIfNot(Iterator first, Iterator last, Comparator&& comp,
                               const T& value)
    {
        usize cnt = 0;
        for (; first != last; ++first)
            if (!comp(*first, value)) ++cnt;
        return cnt;
    }

    /**
     * @brief Iterator-aware version of CountIfNot.
     */
    template <typename Iterator, typename IteratorPredicate>
    constexpr usize CountIfNotIter(Iterator first, Iterator last,
                                   IteratorPredicate&& pred)
    {
        usize cnt = 0;
        for (; first != last; ++first)
            if (!pred(first)) ++cnt;
        return cnt;
    }

    /**
     * @brief Wraps an iterator-based predicate to function as a value-based
     * predicate.
     * @tparam IteratorPredicate A predicate that expects an iterator/pointer.
     */
    template <typename IteratorPredicate>
    struct IteratorPredicateAdapter
    {
        IteratorPredicate Pred;

        constexpr explicit IteratorPredicateAdapter(IteratorPredicate pred)
            : Pred(Move(pred))
        {
        }

        template <typename T>
        constexpr bool operator()(T& value) const
        {
            return Pred(&value); // assuming Pred expects an iterator
        }
    };
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::AllOf;
using Prism::AllOfIter;
using Prism::AnyOf;
using Prism::AnyOfIter;
using Prism::Count;
using Prism::CountIf;
using Prism::CountIfIter;
using Prism::EraseIf;
using Prism::Find;
using Prism::FindFirstOf;
using Prism::FindFirstOfIter;
using Prism::FindIf;
using Prism::FindIfIter;
using Prism::FindIfNot;
using Prism::FindIfProj;
using Prism::FindIteratorIter;
using Prism::FindLastIf;
using Prism::FindValueIter;
using Prism::IteratorPredicateAdapter;
using Prism::NoneOf;
using Prism::NoneOfIter;
using Prism::RemoveIf;
using Prism::RemoveIfIter;
#endif
