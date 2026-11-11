/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

#include <Prism/String/CodePoints.hpp>
#include <Prism/String/String.hpp>
#include <Prism/String/StringView.hpp>

#include <Prism/Utility/Enum.hpp>

namespace Prism
{
    namespace StringUtils
    {
        using CodePoints::IsAlpha;
        using CodePoints::IsAlphanumeric;
        using CodePoints::IsBlank;
        using CodePoints::IsControl;
        using CodePoints::IsDigit;
        using CodePoints::IsGraph;
        using CodePoints::IsHexDigit;
        using CodePoints::IsLower;
        using CodePoints::IsPrintable;
        using CodePoints::IsPunctuation;
        using CodePoints::IsSpace;
        using CodePoints::IsUpper;
        using CodePoints::ToDigit;
        using CodePoints::ToLower;
        using CodePoints::ToUpper;
        constexpr usize Length(const char* str)
        { return StringView(str).Size(); }

        template <ArithmeticType T>
        constexpr usize GetDigitCount(T value)
        {
            // Count by dividing the magnitude: the old `i *= 10` loop never
            // ended for negative values (cast to a huge usize) or for values
            // past 10^19, as `i` overflowed before exceeding them.
            u64 magnitude = 0;
            if (value < 0) magnitude = 0 - static_cast<u64>(value);
            else magnitude = static_cast<u64>(value);

            usize result = 1;
            for (; magnitude >= 10; magnitude /= 10) ++result;
            return result;
        }
        constexpr bool IsNumber(StringView input)
        {

            for (const auto c : input)
                if (!CodePoints::IsDigit(c)) return false;
            return true;
        }

        template <ArithmeticType T>
        constexpr StringView ToString(T value, char* dest, i32 base = 10)
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
            str[i]       = '\0';
            usize length = i;

            T     start  = 0;
            T     end    = i - 1;
            while (start < end)
            {
                Swap(*(str + start), *(str + end));
                ++start;
                --end;
            }

            return {str, length};
        }
        template <ArithmeticType T>
        constexpr String ToString(T value, i32 base = 10)
        {
            const bool  isNegative = value < 0 && base == 10;
            const usize size       = GetDigitCount(value) + isNegative + 1;

            String      string;
            string.Reserve(size);

            return ToString(value, string.Raw(), base);
        }

        template <EnumType T>
        constexpr StringView ToString(T enumerator)
        { return EnumName(enumerator); }

        template <ArithmeticType T>
        constexpr T ToNumber(StringView string, usize base = 10)
        {
            if (string.Empty()) return {};
            bool isNegative = string[0] == '-';

            T    number     = 0;
            for (usize i = isNegative; i < string.Size(); ++i)
            {
                u8 c     = string[i];
                T  digit = ToDigit<T>(c);

                if (static_cast<usize>(digit) >= base) break;
                number = number * base + digit;
            }

            return isNegative ? -number : number;
        }
    }; // namespace StringUtils
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
namespace StringUtils = Prism::StringUtils;

using Prism::StringUtils::GetDigitCount;
using Prism::StringUtils::ToNumber;
using Prism::StringUtils::ToString;
#endif
