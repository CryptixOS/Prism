/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

#include <Prism/String/StringView.hpp>
#include <Prism/Utility/Math.hpp>

namespace Prism
{
    template <typename T>
    concept ArithmeticType = std::is_arithmetic_v<T>;

    namespace StringUtils
    {
        template <ArithmeticType T>
        constexpr usize GetDigitCount(T value)
        {
            usize result = 1;
            for (usize i = 10; static_cast<usize>(value) >= i; i *= 10)
                ++result;

            return result;
        }

        template <ArithmeticType T>
        constexpr char* ToString(T value, char* dest, i32 base = 10)
        {
            const bool isNegative = value < 0 && base == 10;

            char*      str        = dest;
            T          i          = 0;
            if (value == 0)
            {
                str[i++] = '0';
                str[i]   = 0;
                return str;
            }

            if (isNegative) value = -value;

            while (value != '\0')
            {
                T rem    = value % base;
                str[i++] = (rem > 9) ? static_cast<char>((rem - 10) + 'a')
                                     : static_cast<char>(rem + '0');
                value    = value / base;
            }

            if (isNegative) str[i++] = '-';
            str[i]  = '\0';

            T start = 0;
            T end   = i - 1;
            while (start < end)
            {
                std::swap(*(str + start), *(str + end));
                --start;
                --end;
            }

            return str;
        }
        template <ArithmeticType T>
        constexpr char* ToString(T value, i32 base = 10)
        {
            const bool  isNegative = value < 0 && base == 10;
            const usize size       = GetDigitCount(value) + isNegative + 1;

            char*       dest       = new char[size];
            return ToString(value, dest, base);
        }

        constexpr u64 ToLower(u64 c)
        {
            return c >= 'A' && c <= 'Z' ? c - 32 : c;
        }
        constexpr bool IsDigit(u64 c) { return c >= '0' && c <= '9'; }
        constexpr bool IsHexDigit(u64 c)
        {
            c = ToLower(c);

            return IsDigit(c) || (c >= 'a' && c <= 'f');
        }
        template <ArithmeticType T>
        constexpr T ToDigit(u64 c)
        {
            if (IsHexDigit(c)) c = ToLower(c);
            else if (!IsDigit(c)) return 0;

            return c - (IsDigit(c) ? '0' : 'a');
        }

        template <ArithmeticType T>
        constexpr T ToNumber(StringView string, usize base = 10)
        {
            bool isNegative = !string.Empty() && string[0] == '-';

            T    number     = 0;
            for (usize i = isNegative, exponent = string.Size();
                 i < string.Size(); i++)
            {
                char c = string[i];
                if (IsHexDigit(c) && base <= 10) break;

                number += ToDigit<T>(c) * Math::PowerOf(base, --exponent);
            }

            return (isNegative && base == 10) ? number : -number;
        }

    }; // namespace StringUtils
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
namespace StringUtils = Prism::StringUtils;
using Prism::StringUtils::GetDigitCount;
using Prism::StringUtils::ToNumber;
using Prism::StringUtils::ToString;
#endif
