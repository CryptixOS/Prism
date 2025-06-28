/*
 * Created by v1tr10l7 on 04.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Array.hpp>
#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Iterator.hpp>
#include <Prism/Core/Limits.hpp>

namespace Prism
{
    inline constexpr usize DynamicExtent = NumericLimits<usize>::Max();
    template <typename T, usize Extent = DynamicExtent>
    class Span
    {
      public:
        using ElementType            = T;
        using ValueType              = RemoveCvType<T>;
        using SizeType               = usize;
        using Pointer                = T*;
        using ConstPointer           = const T*;
        using Reference              = T&;
        using ConstReference         = const T&;
        using Iterator               = Pointer;
        using ConstIterator          = ConstPointer;
        using ReverseIterator        = ::Prism::ReverseIterator<Iterator>;
        using ConstReverseIterator   = ::Prism::ReverseIterator<ConstIterator>;

        constexpr Span() PM_NOEXCEPT = default;
        template <typename It>
        explicit(Extent != DynamicExtent) constexpr Span(It first, usize count)
            : m_Data(first)
            , m_Size(count)
        {
        }
        template <typename It, typename End>
        explicit(Extent != DynamicExtent) constexpr Span(It first, It last)
            : m_Data(first)
            , m_Size(last - first)
        {
        }
        template <usize N>
        constexpr Span(TypeIdentityType<ElementType> (&arr)[N]) PM_NOEXCEPT
            : m_Data(arr),
              m_Size(N)
        {
        }
        template <typename U, usize N>
        constexpr Span(Array<U, N>& arr) PM_NOEXCEPT : m_Data(arr.Raw()),
                                                       m_Size(N)
        {
        }
        template <typename U, usize N>
        constexpr Span(const Array<U, N>& arr) PM_NOEXCEPT : m_Data(arr.Raw()),
                                                             m_Size(N)
        {
        }

        template <typename R>
        explicit(Extent != DynamicExtent) constexpr Span(R&& r)
            : m_Data()
        {
        }
        explicit(Extent != DynamicExtent) constexpr Span(
            std::initializer_list<ValueType> il) PM_NOEXCEPT
            : m_Data(il.begin()),
              m_Size(il.size())
        {
        }
        template <typename U, usize N>
        explicit(Extent != DynamicExtent && N == DynamicExtent) constexpr Span(
            const Span<U, N>& source) PM_NOEXCEPT : m_Data(source.Raw()),
                                                    m_Size(source.Size())
        {
        }
        constexpr Span(const Span& other) PM_NOEXCEPT               = default;

        constexpr Span&    operator=(const Span& other) PM_NOEXCEPT = default;

        constexpr Iterator begin() const PM_NOEXCEPT { return m_Data; }
        constexpr ConstIterator cbegin() const PM_NOEXCEPT { return m_Data; }

        constexpr Iterator end() const PM_NOEXCEPT { return m_Data + m_Size; }
        constexpr ConstIterator cend() const PM_NOEXCEPT
        {
            return m_Data + m_Size;
        }

        constexpr ReverseIterator rbegin() const PM_NOEXCEPT
        {
            return m_Data + m_Size;
        }
        constexpr ConstReverseIterator crbegin() const PM_NOEXCEPT
        {
            return m_Data + m_Size;
        }

        constexpr ReverseIterator rend() const PM_NOEXCEPT { return m_Data; }
        constexpr ConstReverseIterator crend() const PM_NOEXCEPT
        {
            return m_Data;
        }

        constexpr Reference Front() const
        {
            assert(!Empty());
            return m_Data[0];
        }
        constexpr Reference Back() const
        {
            assert(!Empty());
            return m_Data[m_Size - 1];
        }
        constexpr Reference At(SizeType pos) const
        {
            assert(pos < m_Size);
            return m_Data[pos];
        }
        constexpr Reference operator[](SizeType index) const
        {
            return At(index);
        }
        constexpr Pointer  Raw() const PM_NOEXCEPT { return m_Data; }

        constexpr SizeType Size() const PM_NOEXCEPT { return m_Size; }
        constexpr SizeType SizeInBytes() const PM_NOEXCEPT
        {
            return m_Size * sizeof(T);
        }
        constexpr bool Empty() const PM_NOEXCEPT { return m_Size == 0; }

        template <usize Count>
        constexpr Span<ElementType, Count> First() const
        {
            return {m_Data, m_Size};
        }
        constexpr Span<ElementType, DynamicExtent> First(SizeType count) const
        {
            assert(count <= m_Size);
            return {m_Data, count};
        }

        template <usize Count>
        constexpr Span<ElementType, Count> Last() const
        {
            return {m_Data + m_Size - Count, Count};
        }
        constexpr Span<ElementType, DynamicExtent> Last(SizeType count) const
        {
            assert(count <= m_Size);
            return {m_Data + m_Size - count, count};
        }
        template <usize Offset, usize Count = DynamicExtent>
        constexpr Span<ElementType> SubSpan() const
        {
            assert(Offset + Count <= m_Size);
            return {m_Data + Offset, Count};
        }
        constexpr Span<ElementType, DynamicExtent>
        SubSpan(SizeType offset, SizeType count = DynamicExtent) const
        {
            assert(offset + count <= m_Size);
            return {m_Data + offset, count};
        }

      private:
        Pointer  m_Data = nullptr;
        SizeType m_Size = 0;
    };
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX == 1
using Prism::DynamicExtent;
using Prism::Span;
#endif
