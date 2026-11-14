/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

export module Prism.String.StringUtils;

export import Prism.String.CodePoints;
export import Prism.String.String;
export import Prism.String.StringView;
export import Prism.Utility.Enum;

export extern "C++"
{
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

            /// Writes value in `base` (2 to 36) to dest, NUL-terminated, and
            /// returns the digits. dest needs room for 66 characters (64
            /// binary digits, a sign and the NUL). Only base 10 shows a sign;
            /// other bases print the two's complement bits, like printf's %x.
            template <Integral T>
            constexpr StringView ToString(T value, char* dest, i32 base = 10)
            {
                PrismCheck(base >= 2 && base <= 36);
                using Unsigned        = MakeUnsignedType<
                           ConditionalType<IsSameV<T, bool>, u8, T>>;
                const bool isNegative = IsSignedV<T> && value < 0 && base == 10;
                // The magnitude, computed without negating T (which overflows
                // for the minimum value).
                Unsigned   magnitude  = static_cast<Unsigned>(value);
                if (isNegative) magnitude = Unsigned(0) - magnitude;

                char  digits[64];
                usize count = 0;
                do {
                    u32 digit       = static_cast<u32>(magnitude % Unsigned(base));
                    digits[count++] = static_cast<char>(
                        digit < 10 ? '0' + digit : 'a' + (digit - 10));
                    magnitude /= Unsigned(base);
                } while (magnitude != 0);

                usize length = 0;
                if (isNegative) dest[length++] = '-';
                while (count > 0) dest[length++] = digits[--count];
                dest[length] = '\0';

                return {dest, length};
            }
            template <Integral T>
            constexpr String ToString(T value, i32 base = 10)
            {
                char buffer[72];
                return String(ToString(value, buffer, base));
            }

            template <EnumType T>
            constexpr StringView ToString(T enumerator)
            { return EnumName(enumerator); }

            /**
             * @brief Parses an integer in `base` (2 to 36) from the start of
             * `string`, with an optional leading '-', stopping at the first
             * character that isn't a digit of the base. Values out of range
             * wrap around like unsigned arithmetic (no undefined behaviour);
             * "" and "-" give 0.
             */
            template <Integral T>
            constexpr T ToNumber(StringView string, usize base = 10)
            {
                PrismCheck(base >= 2 && base <= 36);
                using Unsigned  = MakeUnsignedType<
                     ConditionalType<IsSameV<T, bool>, u8, T>>;
                bool     isNegative = !string.Empty() && string[0] == '-';

                Unsigned number     = 0;
                for (usize i = isNegative; i < string.Size(); ++i)
                {
                    char  c     = string[i];
                    usize digit = base;
                    if (c >= '0' && c <= '9') digit = c - '0';
                    else if (c >= 'a' && c <= 'z') digit = 10 + (c - 'a');
                    else if (c >= 'A' && c <= 'Z') digit = 10 + (c - 'A');
                    if (digit >= base) break;

                    number = static_cast<Unsigned>(number * Unsigned(base)
                                                   + Unsigned(digit));
                }

                if (isNegative) number = Unsigned(0) - number;
                return static_cast<T>(number);
            }
        }; // namespace StringUtils
    }; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    namespace StringUtils = Prism::StringUtils;

    using Prism::StringUtils::GetDigitCount;
    using Prism::StringUtils::ToNumber;
    using Prism::StringUtils::ToString;
#endif
}
