/*
 * Created by v1tr10l7 on 01.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Iterator.hpp>
#include <Prism/Core/TypeTraits.hpp>

#include <Prism/Utility/Compare.hpp>

namespace Prism
{
    /**
     *  @brief  Return an iterator pointing to the first element of
     *          the container.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto Begin(Container& container)
        PM_NOEXCEPT(PM_NOEXCEPT(container.begin()))
            -> decltype(container.begin())
    {
        return container.begin();
    }
    /**
     *  @brief  Return an iterator pointing to the first element of
     *          the const container.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto
    Begin(const Container& container)
        PM_NOEXCEPT(PM_NOEXCEPT(container.begin()))
            -> decltype(container.begin())
    {
        return container.begin();
    }
    /**
     *  @brief  Return an iterator pointing to one past the last element of
     *          the container.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto End(Container& container)
        PM_NOEXCEPT(PM_NOEXCEPT(container.end())) -> decltype(container.end())
    {
        return container.end();
    }
    /**
     *  @brief  Return an iterator pointing to one past the last element of
     *          the const container.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto End(const Container& container)
        PM_NOEXCEPT(PM_NOEXCEPT(container.end())) -> decltype(container.end())
    {
        return container.end();
    }
    /**
     *  @brief  Return an iterator pointing to the first element of the array.
     *  @param  arr  Array.
     */
    template <typename T, usize Count>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr T*
    Begin(T (&arr)[Count]) PM_NOEXCEPT
    {
        return arr;
    }
    /**
     *  @brief  Return an iterator pointing to one past the last element
     *          of the array.
     *  @param  arr  Array.
     */
    template <typename T, usize Count>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr T* End(T (&arr)[Count]) PM_NOEXCEPT
    {
        return arr + Count;
    }

    template <typename T>
    class ValueArray;
    // These overloads must be declared for cbegin and cend to use them.
    template <typename T>
    T* Begin(ValueArray<T>&) PM_NOEXCEPT;
    template <typename T>
    const T* Begin(const ValueArray<T>&) PM_NOEXCEPT;
    template <typename T>
    T* End(ValueArray<T>&) PM_NOEXCEPT;
    template <typename T>
    const T* End(const ValueArray<T>&) PM_NOEXCEPT;

    /**
     *  @brief  Return an iterator pointing to the first element of
     *          the const container.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto
    ConstBegin(const Container& container)
        PM_NOEXCEPT(PM_NOEXCEPT(Prism::Begin(container)))
            -> decltype(Prism::Begin(container))
    {
        return Begin(container);
    }
    /**
     *  @brief  Return an iterator pointing to one past the last element of
     *          the const container.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto
    ConstEnd(const Container& container)
        PM_NOEXCEPT(PM_NOEXCEPT(Prism::End(container)))
            -> decltype(Prism::End(container))
    {
        return Prism::End(container);
    }
    /**
     *  @brief  Return a reverse iterator pointing to the last element of
     *          the container.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto
    ReverseBegin(Container& container)
        PM_NOEXCEPT(PM_NOEXCEPT(container.rbegin()))
            -> decltype(container.rbegin())
    {
        return container.rbegin();
    }
    /**
     *  @brief  Return a reverse iterator pointing to the last element of
     *          the const container.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto
    ReverseBegin(const Container& container)
        PM_NOEXCEPT(PM_NOEXCEPT(container.rbegin()))
            -> decltype(container.rbegin())
    {
        return container.rbegin();
    }
    /**
     *  @brief  Return a reverse iterator pointing one past the first element of
     *          the container.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto
    ReverseEnd(Container& container) PM_NOEXCEPT(PM_NOEXCEPT(container.rend()))
        -> decltype(container.rend())
    {
        return container.rend();
    }

    /**
     *  @brief  Return a reverse iterator pointing one past the first element of
     *          the const container.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto
    ReverseEnd(const Container& container)
        PM_NOEXCEPT(PM_NOEXCEPT(container.rend())) -> decltype(container.rend())
    {
        return container.rend();
    }
    /**
     *  @brief  Return a reverse iterator pointing to the last element of
     *          the array.
     *  @param  arr  Array.
     */
    template <typename T, usize Count>
    PM_NODISCARD inline constexpr ReverseIterator<T*>
    ReverseBegin(T (&arr)[Count]) PM_NOEXCEPT
    {
        return ReverseIterator<T*>(arr + Count);
    }
    /**
     *  @brief  Return a reverse iterator pointing one past the first element of
     *          the array.
     *  @param  arr  Array.
     */
    template <typename T, usize Count>
    PM_NODISCARD inline constexpr ReverseIterator<T*>
    ReverseEnd(T (&arr)[Count]) PM_NOEXCEPT
    {
        return ReverseIterator<T*>(arr);
    }
    /**
     *  @brief  Return a reverse iterator pointing to the last element of
     *          the InitializerList.
     *  @param  ilist  InitializerList.
     */
    template <typename T>
    PM_NODISCARD inline constexpr ReverseIterator<const T*>
    ReverseBegin(InitializerList<T> ilist) PM_NOEXCEPT
    {
        return ReverseIterator<const T*>(ilist.end());
    }
    /**
     *  @brief  Return a reverse iterator pointing one past the first element of
     *          the InitializerList.
     *  @param  ilist  InitializerList.
     */
    template <typename T>
    PM_NODISCARD inline constexpr ReverseIterator<const T*>
    ReverseEnd(InitializerList<T> ilist) PM_NOEXCEPT
    {
        return ReverseIterator<const T*>(ilist.begin());
    }
    /**
     *  @brief  Return a reverse iterator pointing to the last element of
     *          the const container.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto
    ReverseConstBegin(const Container& container)
        PM_NOEXCEPT(PM_NOEXCEPT(ReverseBegin(container)))
            -> decltype(ReverseBegin(container))
    {
        return ReverseBegin(container);
    }
    /**
     *  @brief  Return a reverse iterator pointing one past the first element of
     *          the const container.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto
    ReverseConstEnd(const Container& container)
        PM_NOEXCEPT(PM_NOEXCEPT(ReverseEnd(container)))
            -> decltype(ReverseEnd(container))
    {
        return ReverseEnd(container);
    }

    /**
     *  @brief  Return the Size of a container.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto
    Size(const Container& container) PM_NOEXCEPT(PM_NOEXCEPT(container.Size()))
        -> decltype(container.Size())
    {
        return container.Size();
    }

    /**
     *  @brief  Return the Size of an array.
     */
    template <typename T, usize Count>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr usize
    Size(const T (&)[Count]) PM_NOEXCEPT
    {
        return Count;
    }

    /**
     *  @brief  Return whether a container is Empty.
     *  @param  container  Container.
     */
    template <typename Container>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr auto
    Empty(const Container& container)
        PM_NOEXCEPT(PM_NOEXCEPT(container.Empty()))
            -> decltype(container.Empty())
    {
        return container.Empty();
    }

    /**
     *  @brief  Return whether an array is Empty (always false).
     */
    template <typename T, usize Count>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr bool
    Empty(const T (&)[Count]) PM_NOEXCEPT
    {
        return false;
    }

    /**
     *  @brief  Return whether an InitializerList is Empty.
     *  @param  ilist  Initializer list.
     */
    template <typename T>
    PM_NODISCARD PM_ALWAYS_INLINE constexpr bool
    Empty(InitializerList<T> ilist) PM_NOEXCEPT
    {
        return ilist.size() == 0;
    }

    /**
     *  @brief  Searches for the first element in the partitioned range [first,
     * last) which is not ordered before value.
     *  @param  first Iterator to the beginning of the range
     *  @param  last Iterator to the end of the range
     *  @param  value Value to compare the elements to
     *  @param  comparator Binary predicate which returns true if the first
     * argument is ordered before the second
     *
     * @return Iterator to the first element of new range
     */
    template <typename ForwardIterator, typename T, typename Comparator>
    constexpr ForwardIterator LowerBound(ForwardIterator first,
                                         ForwardIterator last, const T& value,
                                         Comparator comparator)
    {
        using DistanceType =
            typename IteratorTraits<ForwardIterator>::DifferenceType;

        DistanceType length = Distance(first, last);

        while (length > 0)
        {
            DistanceType    half   = length >> 1;
            ForwardIterator middle = first;
            Advance(middle, half);
            if (comparator(middle, value))
            {
                first = middle;
                ++first;
                length -= half - 1;
                continue;
            }

            length = half;
        }
        return first;
    }

    /**
     *  @brief Finds the first position in which @a val could be inserted
     *         without changing the ordering.
     *  @param  first   An iterator.
     *  @param  last    Another iterator.
     *  @param  value     The search term.
     *
     *  @return         An iterator pointing to the first element <em>not less
     *                  than</em> @a val, or end() if every element is less than
     *                  @a val.
     */
    template <typename ForwardIterator, typename T>
    PM_NODISCARD constexpr inline ForwardIterator
    LowerBound(ForwardIterator first, ForwardIterator last, const T& value)
    {
        return LowerBound(first, last, value, IteratorLessValueType());
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Empty;
using Prism::LowerBound;
using Prism::Size;
#endif
