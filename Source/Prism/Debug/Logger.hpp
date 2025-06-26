/*
 * Created by v1tr10l7 on 23.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/String/String.hpp>

namespace Prism
{
    class Logger
    {
      public:
        Logger(StringView name, bool endl = false);

        constexpr inline StringView Name() const PM_NOEXCEPT { return m_Name; }

        constexpr inline StringView LabelColor() const PM_NOEXCEPT
        {
            return m_LabelColor;
        }

        constexpr inline StringView TextColor() const PM_NOEXCEPT
        {
            return m_TextColor;
        }

        constexpr inline void SetName(StringView name) { m_Name = name; }
        constexpr inline void SetLabelColor(StringView color)
        {
            m_LabelColor = color;
        }
        constexpr inline void SetTextColor(StringView color)
        {
            m_TextColor = color;
        }

        void operator<<(StringView string) const;

      private:
        String     m_Name         = "";
        bool       m_PrintNewLine = false;

        StringView m_TextColor    = "\e[0m";
        StringView m_LabelColor   = "\e[0m\e[35";
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
#endif
