/*
 * Created by v1tr10l7 on 12.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/TypeTraits.hpp>

namespace Prism
{
    struct InputIteratorTag
    {
    };
    struct OutputIteratorTag
    {
    };
    struct ForwardIteratorTag : InputIteratorTag
    {
    };
    struct BidirectionalIteratorTag : ForwardIteratorTag
    {
    };
    struct RandomAccessIteratorTag : BidirectionalIteratorTag
    {
    };

    template <typename Iterator>
    struct IteratorTraits
    {
      public:
        using DifferenceType   = typename Iterator::DifferenceType;
        using ValueType        = typename Iterator::ValueType;
        using Pointer          = typename Iterator::Pointer;
        using Reference        = typename Iterator::Reference;
        using IteratorCategory = typename Iterator::IteratorCategory;
    };
    // Partial specialization for pointer types
    template <typename T>
    struct IteratorTraits<T*>
    {
      public:
        using DifferenceType   = ptrdiff;
        using ValueType        = T;
        using Pointer          = T*;
        using Reference        = T&;
        using IteratorCategory = RandomAccessIteratorTag;
    };
    // Partial specialization for const pointer types
    template <typename T>
    struct IteratorTraits<const T*>
    {
      public:
        using DifferenceType   = ptrdiff;
        using ValueType        = T;
        using Pointer          = const T*;
        using Reference        = const T&;
        using IteratorCategory = RandomAccessIteratorTag;
    };

    struct DefaultSentinelType
    {
    };

    /// A default sentinel value.
    inline constexpr DefaultSentinelType DefaultSentinel{};

    namespace Details
    {
        template <typename T>
        using WithRef = T&;

        template <typename T>
        concept CanReference = requires { typename WithRef<T>; };

        template <typename T>
        concept Dereferencable = requires(T& t) {
            { *t } -> CanReference;
        };
    } // namespace Details

    template <Details::Dereferencable T>
    using IteratorReferenceType = decltype(*DeclVal<T&>());

    template <typename Iterator>
    struct ReverseIterator
    {
      public:
        using IteratorType = Iterator;
        using IteratorCategory =
            typename IteratorTraits<Iterator>::IteratorCategory;
        using ValueType = typename IteratorTraits<Iterator>::ValueType;
        using DifferenceType =
            typename IteratorTraits<Iterator>::DifferenceType;
        using Pointer   = typename IteratorTraits<Iterator>::Pointer;
        using Reference = typename IteratorTraits<Iterator>::Reference;

        constexpr ReverseIterator() = default;
        explicit constexpr ReverseIterator(IteratorType iter)
            : m_Current(iter)
        {
        }

        template <typename OtherIterator>
        constexpr ReverseIterator(const ReverseIterator<OtherIterator>& other)
            : m_Current(other.Base())
        {
        }

        constexpr IteratorType Base() const { return m_Current; }

        constexpr Reference    operator*() const
        {
            IteratorType tmp = m_Current;
            return *--tmp;
        }
        constexpr Pointer operator->() const
        {
            return std::addressof(operator*());
        }

        constexpr ReverseIterator& operator++()
        {
            --m_Current;
            return *this;
        }
        constexpr ReverseIterator operator++(int)
        {
            ReverseIterator tmp = *this;
            --m_Current;
            return tmp;
        }
        constexpr ReverseIterator& operator--()
        {
            ++m_Current;
            return *this;
        }
        constexpr ReverseIterator operator--(int)
        {
            ReverseIterator tmp = *this;
            ++m_Current;
            return tmp;
        }

        constexpr ReverseIterator operator+(DifferenceType n) const
        {
            return ReverseIterator(m_Current - n);
        }
        constexpr ReverseIterator& operator+=(DifferenceType n)
        {
            m_Current -= n;
            return *this;
        }
        constexpr ReverseIterator operator-(DifferenceType n) const
        {
            return ReverseIterator(m_Current + n);
        }

        constexpr ReverseIterator& operator-=(DifferenceType n)
        {
            m_Current += n;
            return *this;
        }

        constexpr Reference operator[](DifferenceType n) const
        {
            return *(*this + n);
        }

        friend constexpr bool operator==(const ReverseIterator& lhs,
                                         const ReverseIterator& rhs)
        {
            return lhs.m_Current == rhs.m_Current;
        }
        friend constexpr bool operator!=(const ReverseIterator& lhs,
                                         const ReverseIterator& rhs)
        {
            return lhs.m_Current != rhs.m_Current;
        }

        friend constexpr bool operator<(const ReverseIterator& lhs,
                                        const ReverseIterator& rhs)
        {
            return lhs.m_Current > rhs.m_Current;
        }

        friend constexpr bool operator>(const ReverseIterator& lhs,
                                        const ReverseIterator& rhs)
        {
            return lhs.m_Current < rhs.m_Current;
        }

        friend constexpr bool operator<=(const ReverseIterator& lhs,
                                         const ReverseIterator& rhs)
        {
            return lhs.m_Current >= rhs.m_Current;
        }
        friend constexpr bool operator>=(const ReverseIterator& lhs,
                                         const ReverseIterator& rhs)
        {
            return lhs.m_Current <= rhs.m_Current;
        }

        friend constexpr DifferenceType operator-(const ReverseIterator& lhs,
                                                  const ReverseIterator& rhs)
        {
            return rhs.m_Current - lhs.m_Current;
        }
        friend constexpr ReverseIterator operator+(DifferenceType         n,
                                                   const ReverseIterator& it)
        {
            return it + n;
        }

      private:
        IteratorType m_Current;
    };
}; // namespace Prism
