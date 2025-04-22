/*
 * Created by v1tr10l7 on 27.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Vector.hpp>
#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/String/CharTraits.hpp>
#include <Prism/TypeTraits.hpp>

#include <cassert>
#include <ranges>
#include <utility>

namespace Prism
{
    template <typename C, typename Traits>
    class BasicString;

    template <typename C, typename Traits = CharTraits<C>>
    class BasicStringView
    {
        static_assert(!std::is_array_v<C>);
        static_assert(std::is_trivial_v<C> && std::is_standard_layout_v<C>);
        static_assert(IsSameV<C, typename CharTraits<C>::CharType>);

      public:
        using TraitsType               = Traits;
        using ValueType                = C;
        using PointerType              = ValueType*;
        using ConstPointerType         = const ValueType*;
        using ReferenceType            = ValueType&;
        using ConstReferenceType       = const ValueType&;
        using ConstIterator            = const ValueType*;
        using Iterator                 = ConstIterator;
        using ConstReverseIterator     = std::reverse_iterator<ConstIterator>;
        using ReverseIterator          = ConstReverseIterator;
        using SizeType                 = usize;
        using DifferenceType           = std::ptrdiff_t;
        static constexpr SizeType NPos = SizeType(-1);

        //--------------------------------------------------------------------------
        // Constructors
        //--------------------------------------------------------------------------
        constexpr BasicStringView() PM_NOEXCEPT : m_Data(nullptr), m_Size(0) {}
        constexpr BasicStringView(const BasicStringView&) PM_NOEXCEPT = default;

        constexpr BasicStringView(const C* str) PM_NOEXCEPT
            : BasicStringView(str, str ? TraitsType::Length(str) : 0)
        {
        }
        constexpr BasicStringView(const C* str, SizeType size) PM_NOEXCEPT
            : m_Data(const_cast<C*>(str)),
              m_Size(size)
        {
        }
        template <std::contiguous_iterator It, std::sized_sentinel_for<It> End>
            requires std::same_as<std::iter_value_t<It>, C>
                  && (!std::convertible_to<End, SizeType>)

        constexpr BasicStringView(It first, End last) PM_NOEXCEPT
            : BasicStringView(first, last - first)
        {
        }
        constexpr explicit BasicStringView(BasicStringView&& str)
            : BasicStringView(std::move(str.m_Data), std::move(str.m_Size))
        {
            str.m_Data = nullptr;
            str.m_Size = 0;
        }
        constexpr BasicStringView(std::nullptr_t) = delete;

        constexpr BasicStringView& operator=(const BasicStringView&) PM_NOEXCEPT
            = default;

        //--------------------------------------------------------------------------
        // Iterators
        //--------------------------------------------------------------------------
        [[nodiscard]] constexpr ConstIterator begin() const PM_NOEXCEPT
        {
            return m_Data;
        }
        [[nodiscard]] constexpr ConstIterator end() const PM_NOEXCEPT
        {
            return m_Data + m_Size;
        }
        [[nodiscard]] constexpr ConstIterator cbegin() const PM_NOEXCEPT
        {
            return m_Data;
        }

        [[nodiscard]] constexpr ConstIterator cend() const PM_NOEXCEPT
        {
            return m_Data + m_Size;
        }
        [[nodiscard]] constexpr ConstReverseIterator rbegin() const PM_NOEXCEPT
        {
            return ConstReverseIterator(end());
        }
        [[nodiscard]] constexpr ConstReverseIterator rend() const PM_NOEXCEPT
        {
            return ConstReverseIterator(begin());
        }

        [[nodiscard]] constexpr ConstReverseIterator crbegin() const PM_NOEXCEPT
        {
            return ConstReverseIterator(end());
        }
        [[nodiscard]] constexpr ConstReverseIterator crend() const PM_NOEXCEPT
        {
            return ConstReverseIterator(begin());
        }

        //--------------------------------------------------------------------------
        // Element Access
        //--------------------------------------------------------------------------
        [[nodiscard]] constexpr ConstReferenceType
        operator[](SizeType pos) const PM_NOEXCEPT
        {
            assert(pos < m_Size);
            return *(m_Data + pos);
        }
        [[nodiscard]] constexpr ConstReferenceType
        At(SizeType pos) const PM_NOEXCEPT
        {
            assert(pos < m_Size);
            return *(m_Data + pos);
        }
        [[nodiscard]] constexpr ConstReferenceType Front() const PM_NOEXCEPT
        {
            assert(m_Size > 0);
            return *m_Data;
        }
        [[nodiscard]] constexpr ConstReferenceType Back() const PM_NOEXCEPT
        {
            assert(m_Size > 0);
            return *(m_Data + m_Size - 1);
        }
        [[nodiscard]] constexpr ConstPointerType Raw() const PM_NOEXCEPT
        {
            return m_Data;
        }

        //--------------------------------------------------------------------------
        // Capacity
        //--------------------------------------------------------------------------
        [[nodiscard]] constexpr SizeType Size() const PM_NOEXCEPT
        {
            return m_Size;
        }
        [[nodiscard]] constexpr SizeType MaxSize() const PM_NOEXCEPT
        {
            return (NPos - sizeof(SizeType) - sizeof(void*))
                 / sizeof(ValueType);
        }

        [[nodiscard]] constexpr bool Empty() const PM_NOEXCEPT
        {
            return m_Size == 0;
        }

        //--------------------------------------------------------------------------
        // Modifiers
        //--------------------------------------------------------------------------
        constexpr void RemovePrefix(SizeType n) PM_NOEXCEPT
        {
            assert(n < m_Size);
            m_Data += n;
            m_Size -= n;
        }
        constexpr void RemoveSuffix(SizeType n) PM_NOEXCEPT
        {
            assert(n > m_Size);
            m_Size -= n;
        }
        constexpr void Swap(BasicStringView& other) PM_NOEXCEPT
        {
            *this = std::exchange(other, *this);
        }

        //--------------------------------------------------------------------------
        // String Operations
        //--------------------------------------------------------------------------
        constexpr SizeType Copy(C* str, SizeType n, SizeType pos = 0) const
        {
            assert(pos < m_Size);
            const SizeType count = std::min(n, m_Size - pos);

            TraitsType::Copy(str, m_Data + pos, count);
            return count;
        }

        inline Vector<BasicString<C, Traits>> Split(C delimiter)
        {
            Vector<BasicString<C, Traits>> segments;
            usize                          start = Raw()[0] == delimiter;
            usize                          end   = start;

            while ((end = FindFirstOf(delimiter, start)) != NPos)
            {
                BasicString<C, Traits> segment = Substr(start, end - start);
                if (start != end) segments.PushBack(segment);

                start = end + 1;
            }

            // handle last segment
            if (start < Size()) segments.PushBack(Substr(start));
            return segments;
        }
        [[nodiscard]] constexpr BasicStringView
        Substr(SizeType pos = 0, SizeType count = NPos) const PM_NOEXCEPT
        {
            assert(pos < m_Size);
            count = std::min(count, m_Size - pos);
            return BasicStringView(m_Data + pos, count);
        }
        [[nodiscard]] constexpr i32
        Compare(BasicStringView other) const PM_NOEXCEPT
        {
            const SizeType count = std::min(Size(), other.Size());
            i32 result = TraitsType::Compare(m_Data, other.Raw(), count);

            if (result != 0) return result;
            return static_cast<isize>(Size())
                 - static_cast<isize>(other.Size());
        }
        [[nodiscard]] constexpr i32 Compare(SizeType pos, SizeType count,
                                            BasicStringView str) const
        {
            return Substr(pos, count).Compare(str);
        }
        [[nodiscard]] constexpr i32 Compare(SizeType pos1, SizeType count1,
                                            BasicStringView str, SizeType pos2,
                                            SizeType count2) const
        {
            return Substr(pos1, count1).Compare(str.Substr(pos2, count2));
        }
        [[nodiscard]] constexpr i32 Compare(const C* str) const PM_NOEXCEPT
        {
            return Compare(BasicStringView(str));
        }
        [[nodiscard]] constexpr i32 Compare(SizeType pos, SizeType count,
                                            const C* str) const

        {
            return Substr(pos, count).Compare(BasicStringView(str));
        }
        [[nodiscard]] constexpr i32 Compare(SizeType pos, SizeType count1,
                                            const C* str,
                                            SizeType count2) const PM_NOEXCEPT
        {
            return Substr(pos, count1).Compare(BasicStringView(str, count2));
        }
        [[nodiscard]] constexpr bool
        StartsWith(BasicStringView str) const PM_NOEXCEPT
        {
            return m_Size >= str.m_Size
                && TraitsType::Compare(m_Data, str.m_Data, str.m_Size) == 0;
        }
        [[nodiscard]] constexpr bool StartsWith(C* c) const PM_NOEXCEPT
        {
            return !Empty() && TraitsType::Equal(Front(), c);
        }
        [[nodiscard]] constexpr bool StartsWith(const C* str) const PM_NOEXCEPT
        {
            return StartsWith(BasicStringView(str));
        }

        [[nodiscard]] constexpr bool
        EndsWith(BasicStringView str) const PM_NOEXCEPT
        {
            const SizeType size      = Size();
            const auto     otherSize = str.Size();

            return size >= otherSize
                && TraitsType::Compare(end() - otherSize, str.m_Data, otherSize)
                       == 0;
        }
        [[nodiscard]] constexpr bool EndsWith(C* c) const PM_NOEXCEPT
        {
            return !Empty() && TraitsType::Equal(Back(), c);
        }
        [[nodiscard]] constexpr bool EndsWith(const C* str) const PM_NOEXCEPT
        {
            return EndsWith(BasicStringView(str));
        }

        [[nodiscard]] constexpr bool
        Contains(BasicStringView str) const PM_NOEXCEPT
        {
            return Find(str) != NPos;
        }
        [[nodiscard]] constexpr bool contains(C ch) const PM_NOEXCEPT
        {
            return Find(ch) != NPos;
        }
        [[nodiscard, gnu::nonnull]] constexpr bool
        Contains(const C* ch) const PM_NOEXCEPT
        {
            return Find(ch) != NPos;
        }

        [[nodiscard]] constexpr SizeType
        Find(BasicStringView str, SizeType pos = 0) const PM_NOEXCEPT
        {

            if (str.m_Size == 0) return pos <= m_Size ? pos : NPos;
            if (pos >= m_Size) return NPos;

            const ValueType  first = str.m_Data[0];
            ConstPointerType begin = m_Data + pos;
            ConstPointerType end   = m_Data + m_Size;
            SizeType         len   = m_Size - pos;

            while (len >= str.m_Size)
            {
                begin = Traits::Find(begin, len - str.m_Size + 1, first);
                if (*begin == '\0') return NPos;

                if (Traits::Compare(begin, str.m_Data, str.m_Size) == 0)
                    return begin - m_Data;
                len = end - ++begin;
            }
            return NPos;
        }
        [[nodiscard]] constexpr SizeType Find(const C* str, SizeType pos,
                                              SizeType count) const PM_NOEXCEPT
        {
            return Find(BasicStringView(str, count), pos);
        }
        [[nodiscard]] constexpr SizeType
        Find(const C* str, SizeType pos = 0) const PM_NOEXCEPT
        {
            return Find(str, pos, TraitsType::Length(str));
        }

        [[nodiscard]] constexpr SizeType
        Find(ValueType ch, SizeType pos = 0) const PM_NOEXCEPT
        {
            return Find(BasicStringView(std::addressof(ch), 1), pos);
        }
        //[[nodiscard]]
        // constexpr SizeType RFind(const BasicString& str,
        //                         SizeType pos = NPos) const PM_NOEXCEPT;
        [[nodiscard]] constexpr SizeType RFind(const C* str, SizeType pos,
                                               SizeType count) const PM_NOEXCEPT
        {
            return RFind(BasicStringView(str, count), pos);
        }
        [[nodiscard]] constexpr SizeType
        RFind(const C* str, SizeType pos = NPos) const PM_NOEXCEPT
        {
            return RFind(str, pos, TraitsType::Length(str));
        }
        [[nodiscard]] constexpr SizeType RFind(C ch, SizeType pos
                                                     = NPos) const PM_NOEXCEPT
        {
            return RFind(BasicStringView(std::addressof(ch), 1), pos);
        }
        template <typename StringViewLike>
        [[nodiscard]] constexpr SizeType
        RFind(const StringViewLike& str, SizeType pos = 0) const PM_NOEXCEPT
        {
            if (str.Size() > Size()) return NPos;

            size_t limit = (pos >= Size() ? Size() : pos + 1);
            size_t start = limit - str.Size();

            do {
                if (Traits::Compare(Raw() + start, str.Raw(), str.Size()) == 0)
                    return start;
            } while (start-- > 0);

            return NPos;
        }

        [[nodiscard]] constexpr usize
        FindFirstOf(BasicStringView str, usize pos = 0) const PM_NOEXCEPT
        {
            for (; str.m_Size && pos < m_Size; ++pos)
            {
                ConstPointerType p
                    = Traits::Find(str.m_Data, str.m_Size, m_Data[pos]);
                if (p) return pos;
            }
            return NPos;
        }
        [[nodiscard]] constexpr usize FindFirstOf(C ch, usize pos
                                                        = 0) const PM_NOEXCEPT
        {
            return Find(ch, pos);
        }
        [[nodiscard]] constexpr SizeType
        FindFirstOf(const C* str, SizeType pos,
                    SizeType count) const PM_NOEXCEPT
        {
            return FindFirstOf(BasicString(str, count), pos);
        }
        [[nodiscard, gnu::nonnull]] constexpr SizeType
        FindFirstOf(const C* str, SizeType pos = 0) const PM_NOEXCEPT
        {
            return FindFirstOf(str, pos, Traits::Length(str));
        }
        [[nodiscard]] constexpr SizeType
        FindLastOf(BasicStringView str, SizeType pos = NPos) const PM_NOEXCEPT
        {
            SizeType len = Size();
            if (len && str.m_Size)
            {
                if (--len > pos) len = pos;
                do {
                    if (Traits::Find(str.m_Data, str.m_Size, m_Data[len]))
                        return len;
                } while (len-- != 0);
            }
            return NPos;
        }
        [[nodiscard]] constexpr SizeType
        FindLastOf(C c, SizeType pos = NPos) const PM_NOEXCEPT
        {
            return RFind(c, pos);
        }
        [[nodiscard]] constexpr SizeType
        FindLastOf(const C* str, SizeType pos, SizeType count) const PM_NOEXCEPT
        {
            return FindLastOf(BasicStringView(str, count), pos);
        }
        [[nodiscard, gnu::nonnull]] constexpr SizeType
        FindLastOf(const C* str, SizeType pos = NPos) const PM_NOEXCEPT
        {
            return FindLastOf(str, pos, Traits::Length(str));
        }

        [[nodiscard]] constexpr SizeType
        FindFirstNotOf(BasicStringView str, SizeType pos = 0) const PM_NOEXCEPT
        {
            for (; pos < m_Size; ++pos)
                if (!Traits::Find(str.m_Data, str.m_Size, m_Data[pos]))
                    return pos;
            return NPos;
        }
        [[nodiscard]] constexpr SizeType
        FindFirstNotOf(C ch, SizeType pos = 0) const PM_NOEXCEPT
        {
            return FindFirstNotOf(BasicStringView(std::addressof(ch), 1), pos);
        }

        [[nodiscard]] constexpr SizeType
        FindFirstNotOf(const C* str, SizeType pos,
                       SizeType count) const PM_NOEXCEPT
        {
            return FindFirstNotOf(BasicStringView(str, count), pos);
        }

        [[nodiscard, gnu::nonnull]] constexpr SizeType
        FindFirstNotOf(const C* str, SizeType pos = 0) const PM_NOEXCEPT
        {
            return FindFirstNotOf(str, pos, Traits::Length(str));
        }
        [[nodiscard]] constexpr SizeType FindLastNotOf(BasicStringView str,
                                                       SizeType        pos
                                                       = NPos) const PM_NOEXCEPT
        {

            SizeType len = m_Size;
            if (len)
            {
                if (--len > pos) len = pos;
                do {
                    if (!Traits::Find(str.m_Data, str.m_Size, m_Data[len]))
                        return len;
                } while (len--);
            }
            return NPos;
            return FindLastNotOf(str.m_Data, pos, str.m_Size);
        }

        [[nodiscard]] constexpr SizeType
        FindLastNotOF(C ch, SizeType pos = NPos) const PM_NOEXCEPT
        {
            return FindLastNotOf(BasicStringView(std::addressof(ch), 1), pos);
        }

        [[nodiscard]] constexpr SizeType
        FindLastNotOF(const C* str, SizeType pos,
                      SizeType count) const PM_NOEXCEPT
        {
            return FindLastNotOf(BasicStringView(str, count), pos);
        }

        [[nodiscard, gnu::nonnull]] constexpr SizeType
        FindLastNotOf(const C* str, SizeType pos = NPos) const PM_NOEXCEPT
        {
            return FindLastNotOf(str, pos, Traits::Length(str));
        }

      private:
        C*    m_Data = nullptr;
        usize m_Size = 0;
    };

    using StringView    = BasicStringView<char>;
    using WStringView   = BasicStringView<wchar_t>;
    using U8StringView  = BasicStringView<char8_t>;
    using U16StringView = BasicStringView<char16_t>;
    using U32StringView = BasicStringView<char32_t>;

    template <typename C, typename Traits>
    [[nodiscard]] constexpr bool
    operator==(BasicStringView<C, Traits>                       lhs,
               std::type_identity_t<BasicStringView<C, Traits>> rhs) PM_NOEXCEPT
    {
        return lhs.Size() == rhs.Size() && lhs.Compare(rhs) == 0;
    }

    template <typename C, typename Traits>
    [[nodiscard]] constexpr auto operator<=>(
        const BasicStringView<C, Traits>                       lhs,
        std::type_identity_t<const BasicStringView<C, Traits>> rhs) PM_NOEXCEPT
    {
        return lhs.Compare(rhs);
    }

    inline namespace StringViewLiterals
    {
        inline constexpr BasicStringView<char>
        operator""_sv(const char* str, usize len) PM_NOEXCEPT
        {
            return BasicStringView<char>(str, len);
        }
        inline constexpr BasicStringView<wchar_t>
        operator""_sv(const wchar_t* str, usize len) PM_NOEXCEPT
        {
            return BasicStringView<wchar_t>(str, len);
        }
        inline constexpr BasicStringView<char8_t>
        operator""_sv(const char8_t* str, usize len) PM_NOEXCEPT
        {
            return BasicStringView<char8_t>(str, len);
        }
        inline constexpr BasicStringView<char16_t>
        operator""_sv(const char16_t* str, usize len) PM_NOEXCEPT
        {
            return BasicStringView<char16_t>(str, len);
        }
        inline constexpr BasicStringView<char32_t>
        operator""_sv(const char32_t* str, usize len) PM_NOEXCEPT
        {
            return BasicStringView<char32_t>(str, len);
        }
    }; // namespace StringViewLiterals
}; // namespace Prism

template <>
struct std::hash<Prism::StringView>
    : public std::__hash_base<Prism::usize, Prism::StringView>
{
    [[nodiscard]] Prism::usize
    operator()(const Prism::StringView& str) const noexcept
    {
        return std::hash<std::string_view>{}({str.Raw(), str.Size()});
    }
};
template <>
struct std::__is_fast_hash<std::hash<Prism::StringView>> : std::false_type
{
};
template <>
struct std::hash<Prism::WStringView>
    : public std::__hash_base<Prism::usize, Prism::WStringView>
{
    [[nodiscard]] Prism::usize
    operator()(const Prism::WStringView& str) const noexcept
    {
        return std::_Hash_impl::hash(str.Raw(), str.Size() * sizeof(wchar_t));
    }
};

template <>
struct std::__is_fast_hash<std::hash<Prism::WStringView>> : std::false_type
{
};

template <>
struct std::hash<Prism::U8StringView>
    : public std::__hash_base<Prism::usize, Prism::U8StringView>
{
    [[nodiscard]] Prism::usize
    operator()(const Prism::U8StringView& str) const noexcept
    {
        return std::_Hash_impl::hash(str.Raw(), str.Size());
    }
};
template <>
struct std::__is_fast_hash<std::hash<Prism::U8StringView>> : std::false_type
{
};

template <>
struct std::hash<Prism::U16StringView>
    : public std::__hash_base<Prism::usize, Prism::U16StringView>
{
    [[nodiscard]] Prism::usize
    operator()(const Prism::U16StringView& str) const noexcept
    {
        return std::_Hash_impl::hash(str.Raw(), str.Size() * sizeof(char16_t));
    }
};

template <>
struct std::__is_fast_hash<std::hash<Prism::U16StringView>> : std::false_type
{
};

template <>
struct std::hash<Prism::U32StringView>
    : public std::__hash_base<Prism::usize, Prism::U32StringView>
{
    [[nodiscard]] Prism::usize
    operator()(const Prism::U32StringView& str) const noexcept
    {
        return std::_Hash_impl::hash(str.Raw(), str.Size() * sizeof(char32_t));
    }
};

template <>
struct std::__is_fast_hash<std::hash<Prism::U32StringView>> : std::false_type
{
};

// Opt-in to borrowed_range concept
template <typename C, typename Traits>
inline constexpr bool
    std::ranges::enable_borrowed_range<Prism::BasicStringView<C, Traits>>
    = true;

// Opt-in to view concept
template <typename C, typename Traits>
inline constexpr bool
    std::ranges::enable_view<Prism::BasicStringView<C, Traits>>
    = true;

template <>
struct fmt::formatter<Prism::StringView> : fmt::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(const Prism::StringView& src, FormatContext& ctx) const
    {
        return fmt::formatter<std::string_view>::format(
            fmt::format("{}", src.Raw()), ctx);
    }
};

#if PRISM_TARGET_CRYPTIX == 1
using Prism::StringView;
using Prism::StringViewLiterals::operator""_sv;
#endif
