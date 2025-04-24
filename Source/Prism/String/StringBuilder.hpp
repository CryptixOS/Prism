/*
 * Created by v1tr10l7 on 24.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Vector.hpp>
#include <Prism/String/String.hpp>
#include <Prism/String/StringUtils.hpp>

namespace Prism
{
    template <typename C, typename Traits = CharTraits<C>>
    class StringBuilder
    {
      public:
        StringBuilder(usize capacity = 64)
        {
            m_Segments.PushBack(BasicString<C, Traits>{capacity});
        }

        constexpr usize TotalLength() const
        {
            usize totalLength = 0;
            for (const auto& segment : m_Segments)
                totalLength += segment.Size();

            return totalLength;
        }

        constexpr void Append(C ch)
        {
            Reinvalidate();
            auto& segment = m_Segments.Back();
            segment += ch;
        }
        constexpr void Append(BasicStringView<C, Traits> other)
        {
            Reinvalidate();
            auto& segment = Segment();
            segment += other;
        }
        constexpr void Append(const char* string)
        {
            if (!string) return;
            Append({string, Traits::Length(string)});
        }
        constexpr void PutU64(u64 value)
        {
            Reinvalidate();
            Segment() += StringUtils::ToString<u64>(value);
        }

        constexpr operator BasicString<C, Traits>()
        {
            BasicString<C, Traits> string(TotalLength());
            for (const auto& segment : m_Segments) string += segment;

            return string;
        }

      private:
        using SegmentType                       = BasicString<C, Traits>;
        constexpr static usize MAX_SEGMENT_SIZE = 64;
        Vector<SegmentType>    m_Segments;

        constexpr SegmentType& Segment() { return m_Segments.Back(); }

        void                   Reinvalidate()
        {
            if (Segment().Size() > MAX_SEGMENT_SIZE)
                m_Segments.PushBack(BasicString<C, Traits>{64});
        }
        void EnsureCapacity(usize desired)
        {
            if (m_Segments.Back().Capacity() < desired)
                m_Segments.Back().Reserve(desired);
        }
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::StringBuilder;
#endif
