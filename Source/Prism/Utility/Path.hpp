/*
 * Created by v1tr10l7 on 19.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Utility/PathView.hpp>

namespace Prism
{
    class Path
    {
      public:
        using ViewType              = PathView;
        using TraitsType            = ViewType::TraitsType;
        using ValueType             = TraitsType::CharType;
        using SizeType              = StringView::SizeType;
        using DifferenceType        = ptrdiff;
        using ReferenceType         = ViewType::ValueType&;
        using ConstReferenceType    = const ViewType::ValueType&;
        using PointerType           = ViewType::ValueType*;
        using ConstPointerType      = const ViewType::ValueType*;
        using Iterator              = ViewType::ValueType*;
        using ConstIterator         = const ViewType::ValueType*;
        using ReverseIterator       = Prism::ReverseIterator<Iterator>;
        using ConstReverseIterator  = Prism::ReverseIterator<ConstIterator>;

        constexpr static usize NPos = PathView::NPos;

        constexpr Path()            = default;

        constexpr Path(const ValueType* path)
            : Path(path, TraitsType::Length(path ?: ""))
        {
        }
        constexpr Path(const String& path)
            : Path(path.View())
        {
        }
        constexpr Path(StringView path)
            : Path(path.Raw(), path.Size())
        {
        }
        constexpr Path(PathView path)
            : m_Path(path.Raw(), path.Size())
        {
        }
        constexpr Path(const ValueType* path, usize size)
            : m_Path(path, size)
        {
        }
        constexpr Path(const Path& path)
            : m_Path(path.m_Path)
        {
        }
        constexpr Path(Path&& path)
            : m_Path(Move(path.m_Path))
        {
        }
        constexpr Path(NullType) = delete;

        constexpr Path& operator=(const Path& other)
        {
            return m_Path = other.m_Path, *this;
        }
        constexpr Path& operator=(Path&& str)
        {
            return m_Path = Move(str.m_Path), *this;
        }
        constexpr Path& operator=(const ValueType* str)
        {
            return m_Path = Move(str), *this;
        }

        template <typename View>
            requires(IsConvertibleV<const View&, ViewType>
                     && !IsConvertibleV<const View&, const ValueType*>)
        constexpr Path& operator=(const View& str)
        {
            return m_Path = str, *this;
        }
        constexpr Path& operator=(ValueType ch) { return m_Path = ch, *this; }
        constexpr Path& operator=(NullType) = delete;

        //--------------------------------------------------------------------------
        // Iterators
        //--------------------------------------------------------------------------
        PM_NODISCARD constexpr ConstIterator begin() const PM_NOEXCEPT
        {
            return m_Path.begin();
        }
        PM_NODISCARD constexpr ConstIterator end() const PM_NOEXCEPT
        {
            return m_Path.end();
        }
        PM_NODISCARD constexpr ConstIterator cbegin() const PM_NOEXCEPT
        {
            return m_Path.cbegin();
        }

        PM_NODISCARD constexpr ConstIterator cend() const PM_NOEXCEPT
        {
            return m_Path.cend();
        }
        PM_NODISCARD constexpr ConstReverseIterator rbegin() const PM_NOEXCEPT
        {
            return ConstReverseIterator(end());
        }
        PM_NODISCARD constexpr ConstReverseIterator rend() const PM_NOEXCEPT
        {
            return ConstReverseIterator(begin());
        }

        PM_NODISCARD constexpr ConstReverseIterator crbegin() const PM_NOEXCEPT
        {
            return ConstReverseIterator(end());
        }
        PM_NODISCARD constexpr ConstReverseIterator crend() const PM_NOEXCEPT
        {
            return ConstReverseIterator(begin());
        }

        //--------------------------------------------------------------------------
        // Element Access
        //--------------------------------------------------------------------------
        PM_NODISCARD constexpr ConstReferenceType
        operator[](usize pos) const PM_NOEXCEPT
        {
            return m_Path[pos];
        }
        PM_NODISCARD constexpr ConstReferenceType
        At(usize pos) const PM_NOEXCEPT
        {
            return m_Path.At(pos);
        }
        PM_NODISCARD constexpr ConstReferenceType Front() const PM_NOEXCEPT
        {
            return m_Path.Front();
        }
        PM_NODISCARD constexpr ConstReferenceType Back() const PM_NOEXCEPT
        {
            return m_Path.Back();
        }
        PM_NODISCARD constexpr ConstPointerType Raw() const PM_NOEXCEPT
        {
            return m_Path.Raw();
        }
        PM_NODISCARD constexpr ViewType View() const PM_NOEXCEPT
        {
            return PathView(Raw(), Size());
        }
        PM_NODISCARD constexpr StringView StrView() const PM_NOEXCEPT
        {
            return m_Path.View();
        }
        constexpr operator PathView() const { return m_Path; }
        constexpr operator StringView() const { return m_Path; }

        //--------------------------------------------------------------------------
        // Capacity
        //--------------------------------------------------------------------------
        PM_NODISCARD constexpr usize Size() const PM_NOEXCEPT
        {
            return m_Path.Size();
        }
        PM_NODISCARD constexpr usize MaxSize() const PM_NOEXCEPT
        {
            return m_Path.MaxSize();
        }

        PM_NODISCARD constexpr bool Empty() const PM_NOEXCEPT
        {
            return m_Path.Empty();
        }
        bool           ValidateLength();

        //--------------------------------------------------------------------------
        // Modifiers
        //--------------------------------------------------------------------------
        // constexpr void RemovePrefix(usize n) PM_NOEXCEPT
        // {
        //     m_Path.RemovePrefix(n);
        // }
        // constexpr void RemoveSuffix(usize n) PM_NOEXCEPT
        // {
        //     m_Path.RemoveSuffix(n);
        // }
        constexpr void Swap(Path& other) PM_NOEXCEPT
        {
            *this = Exchange(other, *this);
        }

        inline bool Absolute() const { return !Empty() && m_Path[0] == '/'; }
        StringView  BaseName() const;
        StringView  ParentName() const;
        Path        ParentPath() const;
        StringView  Extension() const;

        //--------------------------------------------------------------------------
        // String Operations
        //--------------------------------------------------------------------------
        constexpr usize Copy(char* str, usize count, usize pos = 0) const
        {
            return m_Path.Copy(str, count, pos);
        }

        Vector<String> Split(ValueType delimiter = '/') const;
        PM_NODISCARD constexpr StringView
        Substr(usize pos = 0, usize count = NPos) const PM_NOEXCEPT
        {
            return m_Path.Substr(pos, count);
        }
        PM_NODISCARD constexpr i32 Compare(const Path& other) const PM_NOEXCEPT
        {
            return m_Path.Compare(other.m_Path);
        }
        PM_NODISCARD constexpr i32 Compare(usize pos, usize count,
                                           StringView str) const
        {
            return m_Path.Compare(pos, count, str);
        }
        PM_NODISCARD constexpr i32 Compare(usize pos1, usize count1,
                                           StringView str, usize pos2,
                                           usize count2) const
        {
            return m_Path.Compare(pos1, count1, str, pos2, count2);
        }
        PM_NODISCARD constexpr i32
        Compare(const ValueType* str) const PM_NOEXCEPT
        {
            return m_Path.Compare(str);
        }
        PM_NODISCARD constexpr i32 Compare(usize pos, usize count,
                                           const ValueType* str) const

        {
            return m_Path.Compare(pos, count, str);
        }
        PM_NODISCARD constexpr i32 Compare(usize pos, usize count1,
                                           const ValueType* str,
                                           usize count2) const PM_NOEXCEPT
        {
            return m_Path.Compare(pos, count1, str, count2);
        }
        PM_NODISCARD constexpr bool StartsWith(StringView str) const PM_NOEXCEPT
        {
            return m_Path.StartsWith(str);
        }
        PM_NODISCARD constexpr bool StartsWith(char c) const PM_NOEXCEPT
        {
            return m_Path.StartsWith(c);
        }
        PM_NODISCARD constexpr bool
        StartsWith(const ValueType* str) const PM_NOEXCEPT
        {
            return m_Path.StartsWith(str);
        }

        PM_NODISCARD constexpr bool EndsWith(StringView str) const PM_NOEXCEPT
        {
            return m_Path.EndsWith(str);
        }
        PM_NODISCARD constexpr bool EndsWith(char c) const PM_NOEXCEPT
        {
            return m_Path.EndsWith(c);
        }
        PM_NODISCARD constexpr bool
        EndsWith(const ValueType* str) const PM_NOEXCEPT
        {
            return m_Path.EndsWith(str);
        }

        PM_NODISCARD constexpr bool Contains(StringView str) const PM_NOEXCEPT
        {
            return View().Contains(str);
        }
        PM_NODISCARD constexpr bool Contains(ValueType ch) const PM_NOEXCEPT
        {
            return View().Contains(ch);
        }
        [[nodiscard, gnu::nonnull]] constexpr bool
        Contains(const ValueType* ch) const PM_NOEXCEPT
        {
            return View().Contains(ch);
        }

        PM_NODISCARD constexpr usize Find(StringView pattern,
                                          usize      pos = 0) const PM_NOEXCEPT
        {
            return View().Find(pattern, pos);
        }
        PM_NODISCARD constexpr usize Find(const ValueType* pattern, usize pos,
                                          usize count) const PM_NOEXCEPT
        {
            return m_Path.Find(pattern, pos, count);
        }
        PM_NODISCARD constexpr usize Find(const ValueType* pattern,
                                          usize pos = 0) const PM_NOEXCEPT
        {
            return m_Path.Find(pattern, pos);
        }

        PM_NODISCARD constexpr usize Find(ValueType ch,
                                          usize     pos = 0) const PM_NOEXCEPT
        {
            return m_Path.Find(ch, pos);
        }
        PM_NODISCARD constexpr usize RFind(const ValueType* pattern, usize pos,
                                           usize count) const PM_NOEXCEPT
        {
            return m_Path.RFind(pattern, pos, count);
        }
        PM_NODISCARD constexpr usize RFind(const ValueType* pattern,
                                           usize pos = NPos) const PM_NOEXCEPT
        {
            return m_Path.RFind(pattern, pos);
        }
        PM_NODISCARD constexpr usize RFind(ValueType ch,
                                           usize pos = NPos) const PM_NOEXCEPT
        {
            return m_Path.RFind(ch, pos);
        }
        template <typename StringViewLike>
        PM_NODISCARD constexpr usize RFind(const StringViewLike& pattern,
                                           usize pos = 0) const PM_NOEXCEPT
        {
            return m_Path.RFind(pattern, pos);
        }

        PM_NODISCARD constexpr usize
        FindFirstOf(StringView chars, usize pos = 0) const PM_NOEXCEPT
        {
            return m_Path.FindFirstOf(chars, pos);
        }
        PM_NODISCARD constexpr usize
        FindFirstOf(ValueType ch, usize pos = 0) const PM_NOEXCEPT
        {
            return m_Path.FindFirstOf(ch, pos);
        }
        PM_NODISCARD constexpr usize FindFirstOf(const ValueType* chars,
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
        PM_NODISCARD constexpr usize
        FindLastOf(StringView chars, usize pos = NPos) const PM_NOEXCEPT
        {
            return m_Path.FindLastOf(chars, pos);
        }
        PM_NODISCARD constexpr usize
        FindLastOf(ValueType c, usize pos = NPos) const PM_NOEXCEPT
        {
            return m_Path.FindLastOf(c, pos);
        }
        PM_NODISCARD constexpr usize FindLastOf(const ValueType* chars,
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

        PM_NODISCARD constexpr usize
        FindFirstNotOf(StringView chars, usize pos = 0) const PM_NOEXCEPT
        {
            return m_Path.FindFirstNotOf(chars, pos);
        }
        PM_NODISCARD constexpr usize
        FindFirstNotOf(ValueType ch, usize pos = 0) const PM_NOEXCEPT
        {
            return m_Path.FindFirstNotOf(ch, pos);
        }

        PM_NODISCARD constexpr usize
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
        PM_NODISCARD constexpr usize
        FindLastNotOf(StringView chars, usize pos = NPos) const PM_NOEXCEPT
        {
            return m_Path.FindLastNotOf(chars, pos);
        }

        PM_NODISCARD constexpr usize
        FindLastNotOf(ValueType ch, usize pos = NPos) const PM_NOEXCEPT
        {
            return m_Path.FindLastNotOf(ch, pos);
        }

        PM_NODISCARD constexpr usize
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

        inline constexpr Path& operator+=(PathView rhs)
        {
            return m_Path += rhs.StrView(), *this;
        }
        inline constexpr Path& operator+=(ValueType ch)
        {
            return m_Path += ch, *this;
        }

        friend constexpr Path operator+(const Path& lhs, const Path& rhs)
        {
            return Path(lhs.m_Path + rhs.StrView());
        }
        friend constexpr Path operator+(const Path& lhs, const ValueType* rhs)
        {
            Path path = lhs.m_Path;
            path += rhs;

            return path;
        }

      private:
        String m_Path;
    };

    constexpr bool operator==(const Path& lhs, const Path& rhs) PM_NOEXCEPT
    {
        return lhs.Compare(rhs) == 0;
    }
    constexpr bool operator==(const Path& lhs, const PathView& rhs) PM_NOEXCEPT
    {
        return lhs.View().Compare(rhs) == 0;
    }

    constexpr bool operator==(const Path& lhs, const Path::ValueType* rhs)
    {
        return lhs.Compare(rhs) == 0;
    }

    constexpr StrongOrdering operator<=>(const Path& lhs,
                                         PathView    rhs) PM_NOEXCEPT
    {
        return lhs.View().Compare(rhs) <=> 0;
    }

    inline constexpr void Swap(Path& lhs, Path& rhs)
        PM_NOEXCEPT(PM_NOEXCEPT(lhs.Swap(rhs)))
    {
        lhs.Swap(rhs);
    }
    inline constexpr StrongOrdering operator<=>(const Path& lhs,
                                                const Path& rhs) noexcept
    {
        return lhs.Compare(rhs) <=> 0;
    }
    inline constexpr StrongOrdering operator<=>(const Path&            lhs,
                                                const Path::ValueType* rhs)
    {
        return lhs.Compare(rhs) <=> 0;
    }
    constexpr Path operator+(const PathView lhs, const Path::ValueType* rhs)
    {
        return String(lhs) + rhs;
    }

    inline namespace PathLiterals
    {
        inline constexpr Path operator""_p(const char* str,
                                           usize       len) PM_NOEXCEPT
        {
            return Path(str, len);
        }
    }; // namespace PathLiterals

#ifndef PRISM_DISABLE_STDHASH
    #define PRISM_DISABLE_STDHASH 0
#endif
#if PRISM_DISABLE_STDHASH == 0
    template <>
    struct Hash<Path> : public Detail::StringHashBase<char>
    {
    };
    template <>
    struct IsFastHash<Hash<Path>> : FalseType
    {
    };
#endif

}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Path;
using namespace Prism::PathLiterals;
#endif

#if PRISM_DISABLE_FMT == 0
template <>
struct fmt::formatter<Prism::Path> : fmt::formatter<fmt::string_view>
{
    template <typename FormatContext>
    auto format(const Prism::Path& src, FormatContext& ctx) const
    {
        return fmt::formatter<fmt::string_view>::format(
            fmt::format("{}",
                        fmt::string_view{src.View().Raw(), src.View().Size()}),
            ctx);
    }
};
#endif

// Opt-in to borrowed_range concept
template <>
inline constexpr bool std::ranges::enable_borrowed_range<Prism::Path> = true;

// Opt-in to view concept
template <>
inline constexpr bool std::ranges::enable_view<Prism::Path> = true;
