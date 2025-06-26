/*
 * Created by v1tr10l7 on 24.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/DoublyLinkedList.hpp>
#include <Prism/String/String.hpp>
#include <Prism/String/StringUtils.hpp>

namespace Prism
{
    template <typename C = char, typename Traits = CharTraits<C>>
    class StringBuilder
    {
      public:
        constexpr explicit StringBuilder(usize segmentCapacity = 100)
            : m_Length(0)
        {
            m_Segments.PushBack(String{""});
            auto& segment = m_Segments.Tail();
            segment.Reserve(segmentCapacity);
        }
        constexpr ~StringBuilder() {}

        constexpr bool Empty() const { return m_Length == 0; }

        constexpr void Append(C c)
        {
            Reinvalidate();
            Segment() += c;
            ++m_Length;
        }
        constexpr void Append(const C* str, usize len)
        {
            Reinvalidate();
            Segment() += BasicStringView<C, Traits>(str, len);
            m_Length += len;
        }
        constexpr void Append(const char* str)
        {
            Append(str, Traits::Length(str));
        }
        constexpr void Append(const BasicString<char>& str)
        {
            Append(str.Raw(), str.Size());
        }
        constexpr void Append(u64 value)
        {
            Reinvalidate();

            auto string = StringUtils::ToString<u64>(value);
            m_Length += string.Size();
            Segment() += string;
        }

        constexpr void Insert(BasicStringView<C, Traits> other)
        {
            m_Segments.PushFront(other);
            m_Length += other.Size();
        }

        constexpr StringBuilder& operator<<(C ch)
        {
            Append(ch);
            return *this;
        }
        constexpr StringBuilder& operator<<(BasicStringView<C, Traits> str)
        {
            Append(str.Raw(), str.Size());
            return *this;
        }
        constexpr StringBuilder& operator<<(const BasicString<C, Traits>& rhs)
        {
            Append(rhs);
            return *this;
        }
        constexpr StringBuilder& operator<<(u64 value)
        {
            Append(value);
            return *this;
        }

        constexpr BasicString<C, Traits> ToString() const
        {
            BasicString<C, Traits> string;
            string.Reserve(m_Length);

            for (auto& segment : m_Segments) string += segment;
            return string;
        }
        constexpr operator auto() { return ToString(); }

        usize     TotalLength() const { return m_Length; }
        using SegmentType = BasicString<C, Traits>;

      private:
        constexpr static usize MAX_SEGMENT_SIZE = 128;

        constexpr SegmentType& Segment() { return m_Segments.Tail(); }
        constexpr void         Reinvalidate()
        {
            auto& segment = Segment();
            if (segment.Size() < MAX_SEGMENT_SIZE) return;

            auto& newSegment = m_Segments.EmplaceBack();
            newSegment.Reserve(MAX_SEGMENT_SIZE);
        }

        DoublyLinkedList<SegmentType> m_Segments;
        usize                         m_Length;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::StringBuilder;
#endif
