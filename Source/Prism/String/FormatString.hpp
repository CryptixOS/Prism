/*
 * Created by v1tr10l7 on 15.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Algorithm/Find.hpp>
#include <Prism/Containers/Array.hpp>
#include <Prism/String/FormatHandler.hpp>
#include <Prism/String/StringView.hpp>

namespace Prism
{
    template <typename Char, typename... Args>
    class BasicFormatString
    {
      public:
        template <usize N>
        inline constexpr BasicFormatString(Char const (&format)[N])
            : m_String({format, N - 1})
        {
        }

        template <typename T>
        BasicFormatString(T const& formatString)
            requires(requires(T t) { StringView{t}; })
            : m_String(formatString)
        {
        }

        auto       View() const { return m_String; }

        StringView m_String = "";
    };

    template <typename... Args>
    using FormatString = BasicFormatString<TypeIdentityType<Args>...>;
}; // namespace Prism
