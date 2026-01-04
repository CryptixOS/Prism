/*
 * Created by v1tr10l7 on 20.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Debug/Log.hpp>
#include <Prism/String/StringView.hpp>

namespace Prism
{
    template <typename C, typename Traits>
    Vector<BasicString<C, Traits>>
    BasicStringView<C, Traits>::Split(C delimiter)
    {
        if (Empty()) return {};

        Vector<BasicString<C, Traits>> segments;
        usize                          start = At(0) == '/';
        usize                          end   = start;

        auto finddelimiter = [this](usize startPos, C delimiter) -> usize
        {
            usize pos = startPos;
            while (pos < Size() && At(pos) != delimiter) ++pos;

            return pos == Size() ? NPos : pos;
        };

        while ((end = finddelimiter(start, delimiter)) != NPos)
        {
            BasicString segment = Substr(start, end - start);
            if (start == end) break;

            segments.PushBack(
                BasicString<C, Traits>(segment.Raw(), segment.Size()));

            start = end + 1;
        }

        // handle last segment
        if (start < Size()) segments.PushBack(Substr(start));
        return Move(segments);
    }

    template <typename C, typename Traits>
    [[nodiscard]] constexpr usize
    BasicStringView<C, Traits>::Find(BasicStringView str,
                                     SizeType        pos) const PM_NOEXCEPT
    {

        if (str.m_Size == 0) return pos <= m_Size ? pos : NPos;
        if (pos >= m_Size) return NPos;

        const ValueType  first = str.m_Data[0];
        ConstPointerType begin = m_Data + pos;
        ConstPointerType end   = m_Data + m_Size;
        SizeType         len   = m_Size - pos;

        while (len >= str.m_Size)
        {
            begin = Traits::find(begin, len - str.m_Size + 1, first);
            if (*begin == '\0') return NPos;

            if (Traits::compare(begin, str.m_Data, str.m_Size) == 0)
                return begin - m_Data;
            len = end - ++begin;
        }
        return NPos;
    }
    template <typename C, typename Traits>
    [[nodiscard]] constexpr usize
    BasicStringView<C, Traits>::Find(const C* str, usize pos,
                                     usize count) const PM_NOEXCEPT
    {
        return Find(BasicStringView(str, count), pos);
    }
    template <typename C, typename Traits>
    [[nodiscard]] constexpr usize
    BasicStringView<C, Traits>::Find(const C* str, usize pos) const PM_NOEXCEPT
    {
        return Find(str, pos, Traits::length(str));
    }
    template <typename C, typename Traits>
    [[nodiscard]] constexpr usize
    BasicStringView<C, Traits>::Find(C ch, usize pos) const PM_NOEXCEPT
    {
        return Find(BasicStringView(AddressOf(ch), 1), pos);
    }

    template <typename C, typename Traits>
    [[nodiscard]] constexpr usize
    BasicStringView<C, Traits>::RFind(BasicStringView str,
                                      usize           pos) const PM_NOEXCEPT
    {
        if (str.Size() > Size()) return NPos;

        usize limit = pos >= Size() ? Size() : pos + 1;
        usize start = limit - str.Size();
        do {
            if (Traits::compare(Raw() + start, str.Raw(), str.Size()) == 0)
                return start;
        } while (start-- > 0);

        return NPos;
    }
    template <typename C, typename Traits>
    [[nodiscard]] constexpr usize
    BasicStringView<C, Traits>::RFind(const C* str, SizeType pos,
                                      SizeType count) const PM_NOEXCEPT
    {
        return RFind(BasicStringView(str, count), pos);
    }
    template <typename C, typename Traits>
    [[nodiscard]] constexpr usize
    BasicStringView<C, Traits>::RFind(const C* str,
                                      SizeType pos) const PM_NOEXCEPT
    {
        return RFind(str, pos, TraitsType::length(str));
    }
    template <typename C, typename Traits>
    [[nodiscard]] constexpr usize
    BasicStringView<C, Traits>::RFind(C ch, SizeType pos) const PM_NOEXCEPT
    {
        return RFind(BasicStringView(AddressOf(ch), 1), pos);
    }

    template <typename C, typename Traits>
    template <typename ViewType = BasicStringView<C, Traits>>
    [[nodiscard, gnu::nonnull]]
    constexpr ViewType::SizeType  ViewType::FindLastNotOf(const C* chars, SizeType pos)
    {

    }
} // namespace Prism
