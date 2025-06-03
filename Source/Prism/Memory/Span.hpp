/*
 * Created by v1tr10l7 on 28.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

#include <span>

namespace Prism
{
    inline constexpr static usize DYNAMIC_EXTENT = usize(-1);
    template <typename T, usize ExtentType = DYNAMIC_EXTENT>
    class Span
    {
      public:
        using ElementType             = T;
        using ValueType               = RemoveCvType<T>;
        using SizeType                = usize;
        using PointerType             = T*;
        using ReferenceType           = T&;
        using Iterator                = PointerType;
        using ReverseIterator         = std::reverse_iterator<Iterator>;

        constexpr static usize Extent = 0;

        constexpr Span() PM_NOEXCEPT  = default;

        template <typename It>
        constexpr explicit(Extent != DYNAMIC_EXTENT) Span(It first, usize count)
            : m_Data(first)
            , m_Size(count)
        {
        }
        constexpr ~Span()
        {
            m_Data = nullptr;
            m_Size = 0;
        }

        constexpr Span&    operator=(const Span& other) PM_NOEXCEPT = default;

        constexpr Iterator begin() const PM_NOEXCEPT { return m_Data; }
        constexpr const Iterator cbegin() const PM_NOEXCEPT { return m_Data; }

        constexpr Iterator end() const PM_NOEXCEPT { return m_Data + m_Size; }
        constexpr const Iterator cend() const PM_NOEXCEPT
        {
            return m_Data + m_Size;
        }

        constexpr Iterator rbegin() const PM_NOEXCEPT
        {
            return ReverseIterator(end());
        }
        constexpr const Iterator crbegin() const PM_NOEXCEPT
        {
            return ReverseIterator(end());
        }

        constexpr Iterator rend() const PM_NOEXCEPT
        {
            return ReverseIterator(begin());
        }
        constexpr const Iterator crend() const PM_NOEXCEPT
        {
            return ReverseIterator(begin());
        }

        constexpr ReferenceType Front() const
        {
            assert(!Empty());
            return m_Data[0];
        }
        constexpr ReferenceType Back() const
        {
            assert(!Empty());
            return m_Data[m_Size - 1];
        }
        constexpr ReferenceType At(usize pos) const
        {
            assert(pos < m_Size);
            return m_Data[pos];
        }
        constexpr ReferenceType operator[](usize index) const
        {
            return At(index);
        }
        constexpr PointerType Raw() const PM_NOEXCEPT { return m_Data; }

        constexpr SizeType    Size() const PM_NOEXCEPT { return m_Size; }
        constexpr SizeType    SizeInBytes() const PM_NOEXCEPT
        {
            return m_Size * sizeof(ElementType);
        }
        constexpr bool Empty() const PM_NOEXCEPT { return m_Size == 0; }

        template <SizeType Count>
        constexpr Span<ElementType, Count> First() const
        {
            return Span<ElementType, Count>(m_Data, Count);
        }
        constexpr Span<ElementType, DYNAMIC_EXTENT> First(SizeType size) const
        {
            return Span<ElementType, DYNAMIC_EXTENT>(m_Data, size);
        }
        template <SizeType Count>
        constexpr Span<ElementType, Count> Last() const
        {
            assert(Count < m_Size);
            return Span<ElementType, Count>(m_Data + m_Size - Count, Count);
        }
        constexpr Span<ElementType, DYNAMIC_EXTENT> Last(SizeType size) const
        {
            assert(size < m_Size);
            return Span<ElementType, DYNAMIC_EXTENT>(m_Data, size);
        }
        template <SizeType Offset, SizeType Count = DYNAMIC_EXTENT>
        constexpr Span<ElementType> SubSpan() const
        {
            assert(Offset + Count < m_Size);
            return Span<ElementType>(m_Data + Offset, Count);
        }
        constexpr Span<ElementType, DYNAMIC_EXTENT>
        SubSpan(SizeType offset, SizeType count = DYNAMIC_EXTENT) const
        {
            assert(offset + count < m_Size);
            return Span<ElementType>(m_Data + offset, count);
        }

      private:
        PointerType m_Data = nullptr;
        SizeType    m_Size = 0;
    };
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX == 1
using Prism::Span;
#endif
