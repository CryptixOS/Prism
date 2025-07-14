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
        constexpr PathView() = default;

        constexpr PathView(const char* path)
            : m_Path(path)
        {
        }
        constexpr PathView(StringView path)
            : m_Path(path.Raw(), path.Size())
        {
        }
        constexpr PathView(const String& path)
            : m_Path(path.Raw(), path.Size())
        {
        }

        constexpr      operator StringView() const { return m_Path; }

        bool           ValidateLength();
        constexpr bool Empty() const { return m_Path.Empty(); }

        inline bool    Absolute() const { return !Empty() && m_Path[0] == '/'; }
        inline Vector<String> SplitPath() const { return m_Path.Split('/'); }

        constexpr const char& operator[](usize i) const { return m_Path[i]; }

        constexpr i32         Compare(const PathView& str) const noexcept
        {
            return m_Path.Compare(str.m_Path);
        }

        constexpr usize       Size() const { return m_Path.Size(); }
        constexpr const char* Raw() const { return m_Path.Raw(); }

        StringView            GetLastComponent() const;

      private:
        StringView m_Path;
    };

    constexpr bool operator==(const PathView& lhs, const PathView& rhs) noexcept
    {
        return lhs.Compare(rhs) == 0;
    }

    constexpr std::strong_ordering operator<=>(const PathView& lhs,
                                               const PathView& rhs) noexcept
    {
        return lhs.Compare(rhs) <=> 0;
    }
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX == 1
using Prism::PathView;
#endif

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
