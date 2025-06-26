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
        constexpr Path(const char path[])
            : m_Path(path)
        {
        }
        constexpr Path(const String& path)
            : m_Path(path.Raw())
        {
        }
        constexpr Path(StringView path)
            : m_Path(path.Raw())
        {
        }
        constexpr Path(const char* path, usize length)
            : m_Path(path, length)
        {
        }

        constexpr const char* Raw() const { return m_Path.Raw(); }
        constexpr StringView  StrView() const { return m_Path; }
        constexpr usize       Size() const { return m_Path.Size(); }

        constexpr          operator StringView() const { return m_Path.View(); }
        constexpr          operator PathView() const { return m_Path.View(); }

        constexpr PathView View() const { return m_Path.View(); }

        constexpr bool     Empty() const { return m_Path.Empty(); }
        constexpr bool Absolute() const { return !Empty() && m_Path[0] == '/'; }
        Vector<String> SplitPath();

        constexpr int  Compare(const Path& str) const noexcept
        {
            return m_Path.Compare(str.m_Path);
        }

        inline bool ValidateLength() const { return View().ValidateLength(); }

      private:
        String m_Path;
    };

    constexpr bool operator==(const Path& lhs, const Path& rhs) noexcept
    {
        return lhs.Compare(rhs) == 0;
    }
    constexpr bool operator==(const Path& lhs, const char* rhs)
    {
        return lhs.Compare(rhs) == 0;
    }

    constexpr std::strong_ordering operator<=>(const Path& lhs,
                                               const Path& rhs) noexcept
    {
        return lhs.Compare(rhs) <=> 0;
    }

    constexpr std::strong_ordering operator<=>(const Path& lhs, const char* rhs)
    {
        return lhs.Compare(rhs) <=> 0;
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::Path;
#endif

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
