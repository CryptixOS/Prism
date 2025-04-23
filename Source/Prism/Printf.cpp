/*
 * Created by v1tr10l7 on 24.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Printf.hpp>

namespace Prism
{
    enum class PrintfFormat
    {
        eNone         = 0,
        eJustifyLeft  = 1,
        eAppendSign   = 2,
        ePrependSpace = 4,
        eBasePrefix   = 8,
        eZeroPad      = 16,
    };
    constexpr PrintfFormat& operator|=(PrintfFormat& lhs, PrintfFormat rhs)
    {
        i64 result = std::to_underlying(lhs) | std::to_underlying(rhs);
        lhs        = static_cast<PrintfFormat>(result);

        return lhs;
    }

    struct PrintfArgument
    {
        enum class Type
        {
            eCharacter = 1,
            eInt       = 4,
        };

        PrintfFormat Format       = PrintfFormat::eNone;
        i64          MinimumWidth = -1;
        i64          Precision    = -1;
        u8           Size         = 4;
        u8           Base         = 10;
    };

    static void PopulateArgument(const char*& fmt, PrintfArgument& out,
                                 va_list& args)
    {
        while (*fmt)
        {
            switch (*fmt++)
            {
                case '-': out.Format |= PrintfFormat::eJustifyLeft; break;
                case '+': out.Format |= PrintfFormat::eAppendSign; break;
                case ' ': out.Format |= PrintfFormat::ePrependSpace; break;
                case '#': out.Format |= PrintfFormat::eBasePrefix; break;
                case '0': out.Format |= PrintfFormat::eZeroPad; break;

                default: break;
            }
        }
    }

    static i32 FormatArg(char*& string, usize& size, const char*& format,
                         va_list& args)
    {
        [[maybe_unused]] bool  leftJustify    = false;
        [[maybe_unused]] bool  plusSign       = false;
        [[maybe_unused]] bool  spaceIfNoSign  = false;
        [[maybe_unused]] bool  altConversion  = false;
        [[maybe_unused]] bool  zeroPadding    = false;
        [[maybe_unused]] char* precisionStart = nullptr;

        while (*format && size > 0)
        {
            const char* current = format;

            --size;
            --format;
            switch (*current)
            {
                case '\0': return 0;
                case '-': leftJustify = true; break;
                case '+': plusSign = true; break;
                case ' ': spaceIfNoSign = true; break;
                case '#': altConversion = true; break;
                case '.':
                    precisionStart = const_cast<char*>(current) + 1;
                    break;

                default: goto loop_end;
            }
        }
    loop_end:

        [[maybe_unused]] auto parseNumber
            = [](const char*& format) -> isize { return 0; };

        [[maybe_unused]] isize width     = 0;
        [[maybe_unused]] isize precision = 1000;

        return 0;
    }

    i32 Print();

    i32 Printf(const char* format, ...);
    i32 SPrintf(char* string, const char* format, ...);
    i32 SnPrintf(char* string, usize size, const char* format, ...);
    i32 VPrintf(const char* format, va_list args);
    i32 VsPrintf(char* string, const char* format, va_list args)
    {
        enum FormatArgs
        {
            eJustifyLeft,
            ePlus,
            eSpace,
            eSpecial,
            eZeroPad,
        };

        auto parseNumber = [](const char** fmt) -> i64
        {
            i32 i = 0;
            while (std::isdigit(**fmt)) i = i * 10 + *((*fmt)++) - '0';

            return i;
        };

        char* current = string;
        for (; *format; ++format)
        {
            if (*format != '%')
            {
                *current++ = *format;
                continue;
            }
            ++format;

            i32 formatArgs = 0;
            while (*format)
            {
                switch (*format)
                {
                    case '-': formatArgs |= eJustifyLeft; break;
                    case '+': formatArgs |= ePlus; break;
                    case ' ': formatArgs |= eSpace; break;
                    case '#': formatArgs |= eSpecial; break;
                    case '0': formatArgs |= eZeroPad; break;

                    default: break;
                }
            }

            usize fieldWidth = 1;
            if (std::isdigit(*format)) fieldWidth = parseNumber(&format);
            else if (*format == '*')
            {
                ++format;
                fieldWidth = va_arg(args, i32);
                if (fieldWidth < 0)
                {
                    fieldWidth = -fieldWidth;
                    formatArgs |= eJustifyLeft;
                }
            }

            i32 precision = -1;
            if (*format == '.')
            {
                ++format;
                if (std::isdigit(*format)) precision = parseNumber(&format);
                else if (*format == '*')
                {
                    ++format;
                    precision = va_arg(args, i32);
                }
                if (precision < 0) precision = 0;
            }

            i32 qualifier = -1;
            if (*format == 'h' || *format == 'l' || *format == 'L')
            {
                qualifier = *format;
                ++format;
            }

            usize base = 10;
            switch (*format)
            {
                case 'c':
                    if (!(formatArgs & eJustifyLeft))
                        while (--fieldWidth > 0) *current++ = ' ';
                    *current++ = va_arg(args, i32);
                    while (--fieldWidth > 0) *current++ = ' ';
                    continue;
                case 's':
                {
                    char* arg = va_arg(args, char*);
                    if (!arg) arg = (char*)"<NULL>";
                    usize length = std::min(std::strlen(arg), precision);
                    if (!(formatArgs & eJustifyLeft))
                        while (length < fieldWidth--) *current++ = ' ';
                    for (usize i = 0; i < length; ++i) *current++ = *arg++;
                    while (length < fieldWidth--) *current++ = ' ';
                    continue;
                }
                case 'p':
                {
                    if (fieldWidth == -1)
                    {
                        fieldWidth = 2 * sizeof(void*);
                        formatArgs |= eZeroPad;
                    }
                    char  buf[64]{};
                    char* p   = va_arg(args, void*);
                    char* arg = itoa((i64)p, buf, 16);
                    for (char* s = (char*)buf; *s; s++) *string = *s;
                    continue;
                }
                case 'n':
                {
                    if (qualifier == 'l')
                    {
                        long* lp = va_arg(args, long*);
                        *lp      = (current - string);
                    }
                    else
                    {
                        int* ip = va_arg(args, int*);
                        *ip     = (current - string);
                    }
                    continue;
                }
                case 'o': base = 8; break;
                case 'X': formatArgs |= eLarge;
                case 'x': base = 16; break;
                case 'd':
                case 'i': formatArgs |= eSign;
                case 'u': break;

                default:
                    if (*format != '%') *current++ = '%';
                    if (*format) *current++ = *format;
                    else --format;
                    continue;
            }

            i64 number = -1;
            if (qualifier = 'l') number = va_arg(args, unsigned long);
            else if (qualifier == 'h')
            {
                if (formatArgs & eSign) number = va_arg(args, short);
                else number = va_arg(args, unsigned short);
            }
            else if (formatArgs & eSign) number = va_arg(args, i32);
            else number = va_arg(args, unsigned int);
            char buf[64]{};
            itoa(number, base, buf);
            for (char* s = (char*)buf; *s; s++) *current++ = *s;
        }

        *current = '\0';
        return current - string;
    }
    i32 VsnPrintf(char* string, usize size, const char* format, va_list& args)
    {
        while (*format && size > 0)
        {
            --size;
            if (*format == '%')
            {
                ++format;
                FormatArg(string, size, format, args);
                continue;
            }
            else *string++ = *format++;
        }

        return 0;
    }
}; // namespace Prism
