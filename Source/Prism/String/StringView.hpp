/*
 * Created by v1tr10l7 on 27.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Algorithm/SearchString.hpp>
#include <Prism/Containers/Vector.hpp>
#include <Prism/Core/Concepts.hpp>
#include <Prism/Core/Core.hpp>
#include <Prism/String/CharTraits.hpp>
#include <Prism/Utility/Hash.hpp>
#include <Prism/Utility/Math.hpp>

#include <cassert>
#include <ranges>

#if PRISM_TARGET_CRYPTIX == 1
    #include <unordered_map>
#endif

namespace Prism
{
    template <typename C, typename Traits>
    class BasicString;

    template <typename C, typename Traits = CharTraits<C>>
    class BasicStringView
    {
        static_assert(!IsArrayV<C>);
        static_assert(IsTriviallyCopyableV<C>
                      && IsTriviallyDefaultConstructibleV<C>
                      && IsStandardLayoutV<C>);
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
            requires SameAs<std::iter_value_t<It>, C>
                  && (!ConvertibleTo<End, SizeType>)

        constexpr explicit BasicStringView(BasicStringView&& str)
            : BasicStringView(Move(str.m_Data), Move(str.m_Size))
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
            return Raw()[pos];
        }
        [[nodiscard]] constexpr ConstReferenceType
        At(SizeType pos) const PM_NOEXCEPT
        {
            assert(pos < m_Size);
            return Raw()[pos];
        }
        [[nodiscard]] constexpr ConstReferenceType Front() const PM_NOEXCEPT
        {
            assert(m_Size > 0);
            return m_Data[0];
        }
        [[nodiscard]] constexpr ConstReferenceType Back() const PM_NOEXCEPT
        {
            assert(m_Size > 0);
            return m_Data[Size() - 1];
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
            *this = Exchange(other, *this);
        }

        //--------------------------------------------------------------------------
        // String Operations
        //--------------------------------------------------------------------------
        constexpr SizeType Copy(C* str, SizeType count, SizeType pos = 0) const
        {
            assert(pos < m_Size);
            usize copied = std::min(count, Size() - pos);

            TraitsType::Copy(str, Raw() + pos, copied);

            if (copied < count) TraitsType::Assign(str + copied, 0, 1);
            return count;
        }

        inline Vector<BasicString<C, Traits>> Split(C delimiter) const
        {
            Vector<BasicString<C, Traits>> segments;
            if (Empty()) return {};

            usize start = Raw()[0] == delimiter;
            usize end   = start;

            while ((end = FindFirstOf(delimiter, start)) < Size()
                   && start < Size())
            {
                usize                      segmentLength = end - start;
                BasicStringView<C, Traits> segment
                    = Substr(start, segmentLength);
                if (segmentLength != 0) segments.PushBack(segment);

                start = end + 1;
            }

            // handle last segment
            if (start < Size()) segments.PushBack(Substr(start));
            return Move(segments);
        }
        [[nodiscard]] constexpr BasicStringView
        Substr(SizeType pos = 0, SizeType count = NPos) const PM_NOEXCEPT
        {
            assert(pos < m_Size);
            count = std::min(count, Size() - pos);
            return BasicStringView(Raw() + pos, count);
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
        [[nodiscard]] constexpr bool Contains(C ch) const PM_NOEXCEPT
        {
            return Find(ch) != NPos;
        }
        [[nodiscard, gnu::nonnull]] constexpr bool
        Contains(const C* ch) const PM_NOEXCEPT
        {
            return Find(ch) != NPos;
        }

        [[nodiscard]] constexpr SizeType
        Find(BasicStringView pattern, SizeType pos = 0) const PM_NOEXCEPT
        {
            if (pos >= Size() || pattern.Empty()) return NPos;

            auto  view   = Substr(pos);
            usize result = Algorithm::FindStringKMP(view, pattern);

            return result == usize(-1) ? NPos : result + pos;
        }
        [[nodiscard]] constexpr SizeType Find(const C* pattern, SizeType pos,
                                              SizeType count) const PM_NOEXCEPT
        {
            return Find(BasicStringView(pattern, count), pos);
        }
        [[nodiscard]] constexpr SizeType
        Find(const C* pattern, SizeType pos = 0) const PM_NOEXCEPT
        {
            return Find(pattern, pos, TraitsType::Length(pattern));
        }

        [[nodiscard]] constexpr SizeType
        Find(ValueType ch, SizeType pos = 0) const PM_NOEXCEPT
        {
            return Find(BasicStringView(AddressOf(ch), 1), pos);
        }
        [[nodiscard]] constexpr SizeType RFind(const C* pattern, SizeType pos,
                                               SizeType count) const PM_NOEXCEPT
        {
            return RFind(BasicStringView(pattern, count), pos);
        }
        [[nodiscard]] constexpr SizeType
        RFind(const C* pattern, SizeType pos = NPos) const PM_NOEXCEPT
        {
            return RFind(pattern, pos, TraitsType::Length(pattern));
        }
        [[nodiscard]] constexpr SizeType RFind(C ch, SizeType pos
                                                     = NPos) const PM_NOEXCEPT
        {
            return RFind(BasicStringView(AddressOf(ch), 1), pos);
        }
        template <typename StringViewLike>
        [[nodiscard]] constexpr SizeType
        RFind(const StringViewLike& pattern, SizeType pos = 0) const PM_NOEXCEPT
        {
            if (pattern.Size() > Size()) return NPos;

            usize limit = (pos >= Size() ? Size() : pos + 1);
            usize start = limit - pattern.Size();

            do {
                auto substr = Substr(start);
                if (Algorithm::FindStringKMP(substr, pattern) == 0)
                    return start;
            } while (start-- > 0);

            return NPos;
        }

        [[nodiscard]] constexpr usize
        FindFirstOf(BasicStringView chars, usize pos = 0) const PM_NOEXCEPT
        {
            for (; chars.m_Size && pos < m_Size; ++pos)
            {
                ConstPointerType p
                    = Traits::Find(chars.m_Data, chars.m_Size, m_Data[pos]);
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
        FindFirstOf(const C* chars, SizeType pos,
                    SizeType count) const PM_NOEXCEPT
        {
            return FindFirstOf(BasicStringView(chars, count), pos);
        }
        [[nodiscard, gnu::nonnull]] constexpr SizeType
        FindFirstOf(const C* chars, SizeType pos = 0) const PM_NOEXCEPT
        {
            return FindFirstOf(chars, pos, Traits::Length(chars));
        }
        [[nodiscard]] constexpr SizeType
        FindLastOf(BasicStringView chars, SizeType pos = NPos) const PM_NOEXCEPT
        {
            SizeType len = Size();
            if (len && chars.m_Size)
            {
                if (--len > pos) len = pos;
                do {
                    if (Traits::Find(chars.m_Data, chars.m_Size, m_Data[len]))
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
        FindLastOf(const C* chars, SizeType pos,
                   SizeType count) const PM_NOEXCEPT
        {
            return FindLastOf(BasicStringView(chars, count), pos);
        }
        [[nodiscard, gnu::nonnull]] constexpr SizeType
        FindLastOf(const C* chars, SizeType pos = NPos) const PM_NOEXCEPT
        {
            return FindLastOf(chars, pos, Traits::Length(chars));
        }

        [[nodiscard]] constexpr SizeType FindFirstNotOf(BasicStringView chars,
                                                        SizeType        pos
                                                        = 0) const PM_NOEXCEPT
        {
            for (; pos < m_Size; ++pos)
                if (!Traits::Find(chars.m_Data, chars.m_Size, m_Data[pos]))
                    return pos;
            return NPos;
        }
        [[nodiscard]] constexpr SizeType
        FindFirstNotOf(C ch, SizeType pos = 0) const PM_NOEXCEPT
        {
            return FindFirstNotOf(BasicStringView(AddressOf(ch), 1), pos);
        }

        [[nodiscard]] constexpr SizeType
        FindFirstNotOf(const C* chars, SizeType pos,
                       SizeType count) const PM_NOEXCEPT
        {
            return FindFirstNotOf(BasicStringView(chars, count), pos);
        }

        [[nodiscard, gnu::nonnull]] constexpr SizeType
        FindFirstNotOf(const C* chars, SizeType pos = 0) const PM_NOEXCEPT
        {
            return FindFirstNotOf(chars, pos, Traits::Length(chars));
        }
        [[nodiscard]] constexpr SizeType FindLastNotOf(BasicStringView chars,
                                                       SizeType        pos
                                                       = NPos) const PM_NOEXCEPT
        {
            SizeType len = m_Size;
            if (len)
            {
                if (--len > pos) len = pos;
                do {
                    if (!Traits::Find(chars.m_Data, chars.m_Size, m_Data[len]))
                        return len;
                } while (len--);
            }
            return NPos;
        }

        [[nodiscard]] constexpr SizeType
        FindLastNotOf(C ch, SizeType pos = NPos) const PM_NOEXCEPT
        {
            return FindLastNotOf(BasicStringView(AddressOf(ch), 1), pos);
        }

        [[nodiscard]] constexpr SizeType
        FindLastNotOf(const C* chars, SizeType pos,
                      SizeType count) const PM_NOEXCEPT
        {
            return FindLastNotOf(BasicStringView(chars, count), pos);
        }

        [[nodiscard, gnu::nonnull]] constexpr SizeType
        FindLastNotOf(const C* chars, SizeType pos = NPos) const PM_NOEXCEPT
        {
            return FindLastNotOf(chars, pos, Traits::Length(chars));
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
    operator==(BasicStringView<C, Traits> lhs,
               BasicStringView<C, Traits> rhs) PM_NOEXCEPT
    {
        return lhs.Size() == rhs.Size() && lhs.Compare(rhs) == 0;
    }

    template <typename C, typename Traits>
    [[nodiscard]] constexpr auto
    operator<=>(const BasicStringView<C, Traits> lhs,
                const BasicStringView<C, Traits> rhs) PM_NOEXCEPT
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

    namespace Detail
    {
        template <typename C,
                  typename StringType = BasicString<C, CharTraits<C>>>
        struct StringHashBase
        {
            [[nodiscard]] constexpr usize
            operator()(const StringType& str) const PM_NOEXCEPT
            {
#if PRISM_TARGET_CRYPTIX == 1
                return phmap::Hash<std::string_view>{}({str.Raw(), str.Size()});
#else
                const char* key    = str.Raw();
                usize       length = str.Size() * sizeof(C);
                const usize seed   = 0xc70f6907ul;

                return Hash::MurmurHash2(key, length, seed);
#endif
            }
        };
        template <typename C,
                  typename StringViewType = BasicStringView<C, CharTraits<C>>>
        struct StringViewHashBase
        {
            [[nodiscard]] constexpr usize
            operator()(const StringViewType& str) const PM_NOEXCEPT
            {
#if PRISM_TARGET_CRYPTIX == 1
                return phmap::Hash<std::string_view>{}({str.Raw(), str.Size()});
#else
                const char* key    = str.Raw();
                usize       length = str.Size() * sizeof(C);
                const usize seed   = 0xc70f6907ul;

                return Hash::MurmurHash2(key, length, seed);
#endif
            }
        };
    }; // namespace Detail
}; // namespace Prism

template <>
struct std::hash<Prism::BasicStringView<char, Prism::CharTraits<char>>>
    : public Prism::Detail::StringHashBase<char>
{
};
template <>
struct std::__is_fast_hash<std::hash<Prism::StringView>> : Prism::FalseType
{
};
template <>
struct std::hash<Prism::BasicStringView<wchar_t, Prism::CharTraits<wchar_t>>>
    : public Prism::Detail::StringHashBase<wchar_t>
{
};

template <>
struct std::__is_fast_hash<
    std::hash<Prism::BasicStringView<wchar_t, Prism::CharTraits<wchar_t>>>>
    : Prism::FalseType
{
};

template <>
struct std::hash<Prism::BasicStringView<char8_t, Prism::CharTraits<char8_t>>>
    : public Prism::Detail::StringHashBase<char8_t>
{
};
template <>
struct std::__is_fast_hash<
    std::hash<Prism::BasicStringView<char8_t, Prism::CharTraits<char8_t>>>>
    : Prism::FalseType
{
};

template <>
struct std::hash<Prism::BasicStringView<char16_t, Prism::CharTraits<char16_t>>>
    : public Prism::Detail::StringHashBase<
          Prism::usize,
          Prism::BasicStringView<char16_t, Prism::CharTraits<char16_t>>>
{
};

template <>
struct std::__is_fast_hash<
    std::hash<Prism::BasicStringView<char16_t, Prism::CharTraits<char16_t>>>>
    : Prism::FalseType
{
};

template <>
struct std::hash<Prism::BasicStringView<char32_t, Prism::CharTraits<char32_t>>>
    : public Prism::Detail::StringHashBase<
          Prism::usize,
          Prism::BasicStringView<char32_t, Prism::CharTraits<char32_t>>>
{
};

template <>
struct std::__is_fast_hash<
    std::hash<Prism::BasicStringView<char32_t, Prism::CharTraits<char32_t>>>>
    : Prism::FalseType
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
struct fmt::formatter<Prism::StringView> : fmt::formatter<fmt::string_view>
{
    template <typename FormatContext>
    auto format(const Prism::StringView& src, FormatContext& ctx) const
    {
        return fmt::formatter<fmt::string_view>::format(
            fmt::format("{}", fmt::string_view(src.Raw(), src.Size())), ctx);
    }
};

#if PRISM_TARGET_CRYPTIX == 1
using Prism::StringView;
using Prism::StringViewLiterals::operator""_sv;
#endif
