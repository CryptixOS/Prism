/*
 * Created by v1tr10l7 on 09.01.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Concepts.hpp>

namespace Prism
{
    namespace CodePoints
    {
        constexpr u64 ToLower(u64 c)
        {
            return c >= 'A' && c <= 'Z' ? c + 32 : c;
        }
        constexpr u64 ToUpper(u64 c)
        {
            return c >= 'a' && c <= 'z' ? c - 32 : c;
        }

        constexpr bool IsDigit(u64 c) { return c >= '0' && c <= '9'; }
        constexpr bool IsHexDigit(u64 c)
        {
            c = ToLower(c);

            return IsDigit(c) || (c >= 'a' && c <= 'f');
        }

        template <typename T>
        constexpr bool IsLower(T c)
        {
            return c >= 'a' && c <= 'z';
        }
        template <typename T>
        constexpr bool IsUpper(T c)
        {
            return c >= 'A' && c <= 'Z';
        }
        template <typename T>
        constexpr bool IsAlpha(T c)
        {
            return IsLower(c) || IsUpper(c);
        }
        template <typename T>
        constexpr bool IsAlphanumeric(T c)
        {
            return IsAlpha(c) || IsDigit(c);
        }
        template <typename T>
        constexpr bool IsControl(T c)
        {
            return c < 32 || c == '\x7f';
        }
        template <typename T>
        constexpr bool IsBlank(T c);
        template <typename T>
        constexpr bool IsSpace(T c)
        {
            return (c >= '\n' && c <= '\r') || IsBlank(c);
        }
        template <typename T>
        constexpr bool IsBlank(T c)
        {
            return c == ' ' || c == '\t';
        }
        template <typename T>
        constexpr bool IsPunctuation(T c)
        {
            return (c >= '!' && c <= '/') || (c >= ':' && c <= '@')
                || (c >= '[' && c <= '`') || (c >= '{' && c <= '~');
        }
        template <typename T>
        constexpr bool IsGraph(T c)
        {
            return c >= '!' && c <= '~';
        }
        template <typename T>
        constexpr bool IsPrintable(T c)
        {
            return IsAlphanumeric(c) || IsSpace(c) || IsPunctuation(c);
        }

        template <ArithmeticType T>
        constexpr T ToDigit(u64 c)
        {
            if (IsDigit(c)) return c - '0';

            c = ToLower(c);
            return IsHexDigit(c) ? 10 + c - 'a' : 0;
        }
    }; // namespace CodePoints
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::CodePoints::IsAlpha;
using Prism::CodePoints::IsAlphanumeric;
using Prism::CodePoints::IsBlank;
using Prism::CodePoints::IsControl;
using Prism::CodePoints::IsGraph;
using Prism::CodePoints::IsHexDigit;
using Prism::CodePoints::IsLower;
using Prism::CodePoints::IsPrintable;
using Prism::CodePoints::IsPunctuation;
using Prism::CodePoints::IsSpace;
using Prism::CodePoints::IsUpper;
using Prism::CodePoints::ToDigit;
using Prism::CodePoints::ToLower;
using Prism::CodePoints::ToUpper;
#endif
