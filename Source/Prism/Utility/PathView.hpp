/*
 * Created by v1tr10l7 on 23.01.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Vector.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/String/String.hpp>

namespace Prism
{
    class PathView
    {
      public:
        using ValueType             = StringView::ValueType;
        using ConstReferenceType    = StringView::ConstReferenceType;
        using ConstPointerType      = StringView::ConstPointerType;
        using ConstIterator         = StringView::ConstIterator;
        using ConstReverseIterator  = StringView::ConstReverseIterator;
        using TraitsType            = StringView::TraitsType;
        constexpr static usize NPos = StringView::NPos;

        constexpr PathView()        = default;

        constexpr PathView(const ValueType* path)
            : PathView(path, TraitsType::Length(path ?: ""))
        {
        }
        constexpr PathView(const ValueType* path, usize size)
            : PathView(StringView(path))
        {
        }
        constexpr PathView(const String& path)
            : PathView(path.View())
        {
        }
        constexpr PathView(StringView path)
            : m_Path(path)
        {
        }
        constexpr PathView(const PathView&)            = default;
        constexpr PathView(PathView&&)                 = default;
        constexpr PathView(NullType)                   = delete;

        constexpr PathView& operator=(const PathView&) = default;
        constexpr PathView& operator=(PathView&&)      = default;

        //--------------------------------------------------------------------------
        // Iterators
        //--------------------------------------------------------------------------
        [[nodiscard]] constexpr ConstIterator begin() const PM_NOEXCEPT
        {
            return m_Path.begin();
        }
        [[nodiscard]] constexpr ConstIterator end() const PM_NOEXCEPT
        {
            return m_Path.end();
        }
        [[nodiscard]] constexpr ConstIterator cbegin() const PM_NOEXCEPT
        {
            return m_Path.cbegin();
        }

        [[nodiscard]] constexpr ConstIterator cend() const PM_NOEXCEPT
        {
            return m_Path.cend();
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
        operator[](usize pos) const PM_NOEXCEPT
        {
            return m_Path[pos];
        }
        [[nodiscard]] constexpr ConstReferenceType
        At(usize pos) const PM_NOEXCEPT
        {
            return m_Path.At(pos);
        }
        [[nodiscard]] constexpr ConstReferenceType Front() const PM_NOEXCEPT
        {
            return m_Path.Front();
        }
        [[nodiscard]] constexpr ConstReferenceType Back() const PM_NOEXCEPT
        {
            return m_Path.Back();
        }
        [[nodiscard]] constexpr ConstPointerType Raw() const PM_NOEXCEPT
        {
            return m_Path.Raw();
        }
        [[nodiscard]] constexpr StringView StrView() const PM_NOEXCEPT
        {
            return m_Path;
        }
        constexpr operator StringView() const { return m_Path; }

        //--------------------------------------------------------------------------
        // Capacity
        //--------------------------------------------------------------------------
        [[nodiscard]] constexpr usize Size() const PM_NOEXCEPT
        {
            return m_Path.Size();
        }
        [[nodiscard]] constexpr usize MaxSize() const PM_NOEXCEPT
        {
            return m_Path.MaxSize();
        }

        [[nodiscard]] constexpr bool Empty() const PM_NOEXCEPT
        {
            return m_Path.Empty();
        }
        bool           ValidateLength();

        //--------------------------------------------------------------------------
        // Modifiers
        //--------------------------------------------------------------------------
        constexpr void RemovePrefix(usize n) PM_NOEXCEPT
        {
            m_Path.RemovePrefix(n);
        }
        constexpr void RemoveSuffix(usize n) PM_NOEXCEPT
        {
            m_Path.RemoveSuffix(n);
        }
        constexpr void Swap(PathView& other) PM_NOEXCEPT
        {
            *this = Exchange(other, *this);
        }

        inline bool Absolute() const { return !Empty() && m_Path[0] == '/'; }
        StringView  BaseName() const;
        StringView  ParentName() const;
        PathView    ParentPath() const;
        StringView  Extension() const;

        //--------------------------------------------------------------------------
        // String Operations
        //--------------------------------------------------------------------------
        constexpr usize Copy(char* str, usize count, usize pos = 0) const
        {
            return m_Path.Copy(str, count, pos);
        }

        inline Vector<String> Split(ValueType delimiter = '/') const
        {
            return m_Path.Split(delimiter);
        }
        [[nodiscard]] constexpr StringView
        Substr(usize pos = 0, usize count = NPos) const PM_NOEXCEPT
        {
            return m_Path.Substr(pos, count);
        }
        [[nodiscard]] constexpr i32 Compare(StringView other) const PM_NOEXCEPT
        {
            return m_Path.Compare(other);
        }
        [[nodiscard]] constexpr i32 Compare(usize pos, usize count,
                                            StringView str) const
        {
            return m_Path.Compare(pos, count, str);
        }
        [[nodiscard]] constexpr i32 Compare(usize pos1, usize count1,
                                            StringView str, usize pos2,
                                            usize count2) const
        {
            return m_Path.Compare(pos1, count1, str, pos2, count2);
        }
        [[nodiscard]] constexpr i32
        Compare(const ValueType* str) const PM_NOEXCEPT
        {
            return m_Path.Compare(str);
        }
        [[nodiscard]] constexpr i32 Compare(usize pos, usize count,
                                            const ValueType* str) const

        {
            return m_Path.Compare(pos, count, str);
        }
        [[nodiscard]] constexpr i32 Compare(usize pos, usize count1,
                                            const ValueType* str,
                                            usize count2) const PM_NOEXCEPT
        {
            return m_Path.Compare(pos, count1, str, count2);
        }
        [[nodiscard]] constexpr bool
        StartsWith(StringView str) const PM_NOEXCEPT
        {
            return m_Path.StartsWith(str);
        }
        [[nodiscard]] constexpr bool
        StartsWith(const ValueType* str) const PM_NOEXCEPT
        {
            return m_Path.StartsWith(str);
        }

        [[nodiscard]] constexpr bool EndsWith(StringView str) const PM_NOEXCEPT
        {
            return m_Path.EndsWith(str);
        }
        [[nodiscard]] constexpr bool
        EndsWith(const ValueType* str) const PM_NOEXCEPT
        {
            return m_Path.EndsWith(str);
        }

        [[nodiscard]] constexpr bool Contains(StringView str) const PM_NOEXCEPT
        {
            return m_Path.Contains(str);
        }
        [[nodiscard]] constexpr bool Contains(ValueType ch) const PM_NOEXCEPT
        {
            return m_Path.Contains(ch);
        }
        [[nodiscard, gnu::nonnull]] constexpr bool
        Contains(const ValueType* ch) const PM_NOEXCEPT
        {
            return m_Path.Contains(ch);
        }

        [[nodiscard]] constexpr usize Find(StringView pattern,
                                           usize      pos = 0) const PM_NOEXCEPT
        {
            return m_Path.Find(pattern, pos);
        }
        [[nodiscard]] constexpr usize Find(const ValueType* pattern, usize pos,
                                           usize count) const PM_NOEXCEPT
        {
            return m_Path.Find(pattern, pos, count);
        }
        [[nodiscard]] constexpr usize Find(const ValueType* pattern,
                                           usize pos = 0) const PM_NOEXCEPT
        {
            return m_Path.Find(pattern, pos);
        }

        [[nodiscard]] constexpr usize Find(ValueType ch,
                                           usize     pos = 0) const PM_NOEXCEPT
        {
            return m_Path.Find(ch, pos);
        }
        [[nodiscard]] constexpr usize RFind(const ValueType* pattern, usize pos,
                                            usize count) const PM_NOEXCEPT
        {
            return m_Path.RFind(pattern, pos, count);
        }
        [[nodiscard]] constexpr usize RFind(const ValueType* pattern,
                                            usize pos = NPos) const PM_NOEXCEPT
        {
            return m_Path.RFind(pattern, pos);
        }
        [[nodiscard]] constexpr usize RFind(ValueType ch,
                                            usize pos = NPos) const PM_NOEXCEPT
        {
            return m_Path.RFind(ch, pos);
        }
        template <typename StringViewLike>
        [[nodiscard]] constexpr usize RFind(const StringViewLike& pattern,
                                            usize pos = 0) const PM_NOEXCEPT
        {
            return m_Path.RFind(pattern, pos);
        }

        [[nodiscard]] constexpr usize
        FindFirstOf(StringView chars, usize pos = 0) const PM_NOEXCEPT
        {
            return m_Path.FindFirstOf(chars, pos);
        }
        [[nodiscard]] constexpr usize
        FindFirstOf(ValueType ch, usize pos = 0) const PM_NOEXCEPT
        {
            return m_Path.FindFirstOf(ch, pos);
        }
        [[nodiscard]] constexpr usize FindFirstOf(const ValueType* chars,
                                                  usize            pos,
                                                  usize count) const PM_NOEXCEPT
        {
            return m_Path.FindFirstOf(chars, pos, count);
        }
        [[nodiscard, gnu::nonnull]] constexpr usize
        FindFirstOf(const ValueType* chars, usize pos = 0) const PM_NOEXCEPT
        {
            return m_Path.FindFirstOf(chars, pos);
        }
        [[nodiscard]] constexpr usize
        FindLastOf(StringView chars, usize pos = NPos) const PM_NOEXCEPT
        {
            return m_Path.FindLastOf(chars, pos);
        }
        [[nodiscard]] constexpr usize
        FindLastOf(ValueType c, usize pos = NPos) const PM_NOEXCEPT
        {
            return m_Path.FindLastOf(c, pos);
        }
        [[nodiscard]] constexpr usize FindLastOf(const ValueType* chars,
                                                 usize            pos,
                                                 usize count) const PM_NOEXCEPT
        {
            return m_Path.FindLastOf(chars, pos, count);
        }
        [[nodiscard, gnu::nonnull]] constexpr usize
        FindLastOf(const ValueType* chars, usize pos = NPos) const PM_NOEXCEPT
        {
            return m_Path.FindLastOf(chars, pos);
        }

        [[nodiscard]] constexpr usize
        FindFirstNotOf(StringView chars, usize pos = 0) const PM_NOEXCEPT
        {
            return m_Path.FindFirstNotOf(chars, pos);
        }
        [[nodiscard]] constexpr usize
        FindFirstNotOf(ValueType ch, usize pos = 0) const PM_NOEXCEPT
        {
            return m_Path.FindFirstNotOf(ch, pos);
        }

        [[nodiscard]] constexpr usize
        FindFirstNotOf(const ValueType* chars, usize pos,
                       usize count) const PM_NOEXCEPT
        {
            return m_Path.FindFirstNotOf(chars, pos, count);
        }

        [[nodiscard, gnu::nonnull]] constexpr usize
        FindFirstNotOf(const ValueType* chars, usize pos = 0) const PM_NOEXCEPT
        {
            return m_Path.FindFirstNotOf(chars, pos);
        }
        [[nodiscard]] constexpr usize
        FindLastNotOf(StringView chars, usize pos = NPos) const PM_NOEXCEPT
        {
            return m_Path.FindLastNotOf(chars, pos);
        }

        [[nodiscard]] constexpr usize
        FindLastNotOf(ValueType ch, usize pos = NPos) const PM_NOEXCEPT
        {
            return m_Path.FindLastNotOf(ch, pos);
        }

        [[nodiscard]] constexpr usize
        FindLastNotOf(const ValueType* chars, usize pos,
                      usize count) const PM_NOEXCEPT
        {
            return m_Path.FindLastNotOf(chars, pos, count);
        }

        [[nodiscard, gnu::nonnull]] constexpr usize
        FindLastNotOf(const ValueType* chars,
                      usize            pos = NPos) const PM_NOEXCEPT
        {
            return m_Path.FindLastNotOf(chars, pos);
        }

      private:
        StringView m_Path;
    };

    [[nodiscard]] constexpr bool operator==(PathView lhs,
                                            PathView rhs) PM_NOEXCEPT
    {
        return lhs.Size() == rhs.Size() && lhs.Compare(rhs) == 0;
    }
    [[nodiscard]] constexpr auto operator<=>(const PathView lhs,
                                             const PathView rhs) PM_NOEXCEPT
    {
        return lhs.Compare(rhs);
    }

    inline namespace PathViewLiterals
    {
        inline constexpr PathView operator""_pv(const char* str,
                                                usize       len) PM_NOEXCEPT
        {
            return PathView(str, len);
        }
    }; // namespace PathViewLiterals

#ifndef PRISM_DISABLE_STDHASH
    #define PRISM_DISABLE_STDHASH 0
#endif
#if PRISM_DISABLE_STDHASH == 0
    template <>
    struct Hash<PathView> : public Detail::StringHashBase<char>
    {
    };
    template <>
    struct IsFastHash<Hash<PathView>> : FalseType
    {
    };
#endif

}; // namespace Prism
#if PRISM_TARGET_CRYPTIX != 0
using Prism::PathView;
using namespace Prism::PathViewLiterals;
#endif

// Opt-in to borrowed_range concept
template <>
inline constexpr bool std::ranges::enable_borrowed_range<Prism::PathView>
    = true;

// Opt-in to view concept
template <>
inline constexpr bool std::ranges::enable_view<Prism::PathView> = true;

#if PRISM_DISABLE_FMT == 0
template <>
struct fmt::formatter<Prism::PathView> : fmt::formatter<fmt::string_view>
{
    template <typename FormatContext>
    auto format(const Prism::PathView& src, FormatContext& ctx) const
    {
        return fmt::formatter<fmt::string_view>::format(
            fmt::format("{}", fmt::string_view{src.Raw(), src.Size()}), ctx);
    }
};
#endif
