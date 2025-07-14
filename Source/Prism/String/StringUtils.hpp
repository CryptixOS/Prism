/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

#include <Prism/String/String.hpp>
#include <Prism/String/StringView.hpp>
#include <Prism/Utility/Math.hpp>

#include <magic_enum/magic_enum.hpp>

namespace Prism
{
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
                --start;
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
        {
            auto str = magic_enum::enum_name(enumerator);

            return StringView(str.data(), str.size());
        }

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
        template <ArithmeticType T>
        constexpr T ToDigit(u64 c)
        {
            if (IsDigit(c)) return c - '0';

            c = ToLower(c);
            return IsHexDigit(c) ? 10 + c - 'a' : 0;
        }

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

#if PRISM_TARGET_CRYPTIX == 1
namespace StringUtils = Prism::StringUtils;

using Prism::StringUtils::GetDigitCount;
using Prism::StringUtils::IsHexDigit;
using Prism::StringUtils::ToDigit;
using Prism::StringUtils::ToLower;
using Prism::StringUtils::ToNumber;
using Prism::StringUtils::ToString;
using Prism::StringUtils::ToUpper;
#endif
