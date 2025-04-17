/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/Utility/Math.hpp>

namespace Prism
{
    namespace StringUtils
    {
        template <typename T>
            requires(std::is_arithmetic_v<T>)
        constexpr usize GetDigitCount(T value)
        {
            usize result = 0;
            for (usize i = 10; static_cast<usize>(value) < i; i *= 10) ++result;

            return result;
        }

        template <typename T>
            requires(std::is_arithmetic_v<T>)
        constexpr char* ToString(T value, char* dest, i32 base)
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
        template <typename T>
            requires(std::is_arithmetic_v<T>)
        constexpr char* ToString(T value, i32 base)
        {
            const bool  isNegative = value < 0 && base == 10;
            const usize size       = GetDigitCount(value) + isNegative + 1;

            char*       dest       = new char[size];
            return ToString(value, dest, base);
        }

        template <typename T>
            requires(std::is_arithmetic_v<T>)
        T ToNumber(const char* str, usize length)
        {
            T     integer      = 0;
            bool  isNegative   = str[0] == '-';

            usize index        = isNegative;
            usize stringLength = length, power = stringLength - isNegative;

            for (; index < stringLength; index++)
                integer += (str[index] - 48) * Math::PowerOf<T>(10, --power);

            return (isNegative) ? -integer : integer;
        }
        template <typename T>
            requires(std::is_arithmetic_v<T>)
        T ToNumber(const char* str)
        {
            return ToNumber<T>(str, std::strlen(str));
        }

    }; // namespace StringUtils
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
namespace StringUtils = Prism::StringUtils;
using Prism::StringUtils::GetDigitCount;
using Prism::StringUtils::ToNumber;
using Prism::StringUtils::ToString;
#endif
