/*
 * Created by v1tr10l7 on 24.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/String/StringBuilder.hpp>

namespace Prism
{
    template <typename Context>
    class Formatter
    {
      public:
        constexpr Formatter(Context& builder)
            : m_Builder(builder)
        {
        }

        constexpr void Parse(const char* fmt)
        {
            while (*fmt)
            {
                if (*fmt == '{' && fmt[1] == '}')
                {
                    // Too few arguments
                    m_Builder << "<?>";
                    fmt += 2;
                }
                else m_Builder.Append(*fmt++);
            }
        }
        struct FormatSpec
        {
            char  PaddingChar     = ' ';
            usize Width           = 0;
            bool  PrintBasePrefix = false;
            bool  UpperCase       = false;

            enum Base
            {
                eBinary      = 2,
                eOctal       = 8,
                eDecimal     = 10,
                eHexadecimal = 16,
            } Base
                = eDecimal;
        };

        template <typename T, typename... Rest>
        constexpr void Parse(const char* fmt, T&& value, Rest&&... rest)
        {
            while (*fmt)
            {
                if (*fmt == '{')
                {
                    FormatSpec spec;
                    ++fmt;
                    fmt = ParseFormatSpec(fmt, spec);
                    VisitArgument(std::forward<T>(value), spec);
                    continue;
                }

                m_Builder << *fmt++;
            }
            // Extra args ignored
        }
        template <typename T>
        constexpr void VisitArgument(const T& value, const FormatSpec& spec)
        {
            if constexpr (std::is_integral_v<T>)
                VisitArgument(static_cast<u64>(value), spec);
            else if constexpr (IsSameV<T, const char*>
                               || IsSameV<T, StringView>)
                m_Builder << value;
            else static_assert("Unsupported format");
        }
        constexpr void VisitArgument(const char* s, const FormatSpec& spec)
        {
            m_Builder << s;
        }
        constexpr void VisitArgument(const String& s, const FormatSpec& spec)
        {
            m_Builder << s;
        }
        constexpr void VisitArgument(char c, const FormatSpec& spec)
        {
            m_Builder << c;
        }
        constexpr void VisitArgument(u64 val, const FormatSpec& spec)
        {
            if (spec.PrintBasePrefix)
            {
                if (spec.Base == FormatSpec::eBinary) m_Builder.Append("0b");
                else if (spec.Base == FormatSpec::eOctal) m_Builder.Append("0");
                else if (spec.Base == FormatSpec::eHexadecimal)
                    m_Builder.Append(spec.UpperCase ? "0X" : "0x");
            }

            auto  string = StringUtils::ToString<u64>(val, spec.Base);
            usize length = string.Size();

            // Padding
            if (length < spec.Width)
                for (usize i = 0; i < spec.Width - length; ++i)
                    m_Builder << spec.PaddingChar;

            m_Builder << string;
        }

        constexpr const char* ParseFormatSpec(const char* fmt, FormatSpec& spec)
        {
            if (*fmt != ':') return ++fmt; // "{}"
            ++fmt;                         // skip ':'

            if (*fmt == '0')
            {
                spec.PaddingChar = '0';
                ++fmt;
            }

            while (StringUtils::IsDigit(*fmt))
            {
                spec.Width = spec.Width * 10 + (*fmt - '0');
                ++fmt;
            }

            if (*fmt == '#')
            {
                spec.PrintBasePrefix = true;
                ++fmt;
            }
            if (*fmt == 'b') spec.Base = FormatSpec::eBinary, ++fmt;
            else if (*fmt == '0') spec.Base = FormatSpec::eOctal, ++fmt;
            else if (*fmt == 'x')
            {
                spec.Base = FormatSpec::eHexadecimal;
                ++fmt;
            }
            else if (*fmt == 'X')
            {
                spec.Base      = FormatSpec::eHexadecimal, ++fmt;
                spec.UpperCase = true;
            }

            if (*fmt != '}') assert(false);
            return fmt + 1;
        }

      private:
        Context& m_Builder;
    };

    template <typename Context, typename... Args>
    void Format(Context& context, const char* fmt, Args&&... args)
    {
        const char* current = fmt;
        Formatter   formatter(context);
        formatter.Parse(current, std::forward<Args>(args)...);
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::Format;
using Prism::Formatter;
#endif
