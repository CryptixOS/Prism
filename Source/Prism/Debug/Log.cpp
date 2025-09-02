/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Log.hpp>
#include <Prism/Debug/LogSink.hpp>

#include <Prism/String/Printf.hpp>
#include <Prism/String/String.hpp>
#include <Prism/String/StringUtils.hpp>

#if PRISM_TARGET_CRYPTIX != 0
    #include <Library/Logger.hpp>
#endif

namespace Prism::Log
{
#if PRISM_TARGET_CRYPTIX == 0
    #ifndef STDOUT_FILENO
    constexpr usize STDOUT_FILENO = 1;
    #endif
    extern "C" isize write(i32 fd, const void* buf, usize count);

    namespace
    {
        constexpr u64 FOREGROUND_COLOR_RED     = 0x6d31335b1b;
        constexpr u64 FOREGROUND_COLOR_GREEN   = 0x6d32335b1b;
        constexpr u64 FOREGROUND_COLOR_YELLOW  = 0x6d33335b1b;
        constexpr u64 FOREGROUND_COLOR_MAGENTA = 0x6d35335b1b;
        constexpr u64 FOREGROUND_COLOR_CYAN    = 0x6d36335b1b;
        constexpr u64 FOREGROUND_COLOR_WHITE   = 0x6d37335b1b;

        constexpr u64 BACKGROUND_COLOR_BLACK   = 0x6d30345b1b;
        constexpr u64 BACKGROUND_COLOR_RED     = 0x6d31345b1b;
        constexpr u64 RESET_COLOR              = 0x6d305b1b;

        // FIXME(v1tr10l7): Spinlock s_Lock;
        u64           s_LogForegroundColors[]  = {
            FOREGROUND_COLOR_WHITE,  FOREGROUND_COLOR_MAGENTA,
            FOREGROUND_COLOR_GREEN,  FOREGROUND_COLOR_CYAN,
            FOREGROUND_COLOR_YELLOW, FOREGROUND_COLOR_RED,
            FOREGROUND_COLOR_WHITE,
        };

        using ArgumentType       = Printf::ArgumentType;
        using Sign               = Printf::Sign;
        using PrintfFormatSpec   = Printf::FormatSpec;
        using PrintfFormatParser = Printf::FormatParser;
        template <typename T>
        isize LogNumber(va_list& args, PrintfFormatSpec& spec)
        {
            isize nwritten = 0;
            T     value    = va_arg(args, T);

            if (value < 0 && spec.Base == 10)
            {
                spec.Sign = Sign::eMinus;
                value *= -1;
            }
            String str     = StringUtils::ToString(value, spec.Base);

            char   padding = spec.ZeroPad ? '0' : ' ';
            if (spec.Sign != Sign::eNone && spec.Length > 0) spec.Length--;
            if (!spec.JustifyLeft)
            {
                while (static_cast<isize>(str.Size()) < spec.Length)
                {
                    LogChar(padding);
                    --spec.Length;
                    ++nwritten;
                }
            }

            if (spec.Sign == Sign::ePlus) LogChar('+'), ++nwritten;
            else if (spec.Sign == Sign::eMinus) LogChar('-'), ++nwritten;
            else if (spec.Sign == Sign::eSpace) LogChar(' '), ++nwritten;

            if (spec.Base != 10 && spec.PrintBase)
            {
                if (spec.Base == 2) nwritten += Print("0b");
                else if (spec.Base == 8) nwritten += Print("0");
                else if (spec.Base == 16) nwritten += Print("0x");
            }

            for (const auto c : str)
                LogChar(spec.UpperCase && c >= 'a' && c <= 'z'
                            ? StringUtils::ToUpper(c)
                            : c),
                    ++nwritten;
            while (spec.JustifyLeft
                   && static_cast<isize>(str.Size()) < spec.Length)
            {
                LogChar(padding);
                --spec.Length;
                ++nwritten;
            }

            return nwritten;
        }

        isize PrintLogLevel(LogLevel logLevel)
        {
            isize nwritten = 0;
            if (logLevel == LogLevel::eNone) return nwritten;
            nwritten += Print("[");

            LogChar(s_LogForegroundColors[ToUnderlying(logLevel)]);
            ++nwritten;
            if (logLevel == LogLevel::eFatal)
                LogChar(BACKGROUND_COLOR_RED), ++nwritten;

            auto logLevelString = StringUtils::ToString(logLevel);
            logLevelString.RemovePrefix(1);

            nwritten += Print(logLevelString);

            LogChar(FOREGROUND_COLOR_WHITE);
            LogChar(BACKGROUND_COLOR_BLACK);
            LogChar(RESET_COLOR);

            nwritten += Print("]:") + 3;

            for (usize i = 0; i < 8 - logLevelString.Size(); i++, nwritten++)
                LogChar(' ');

            return nwritten;
        }
    }; // namespace

    void  LogChar(u64 c) { Print(reinterpret_cast<const char*>(&c)); }
    isize Print(StringView string)
    {
        return write(STDOUT_FILENO, string.Raw(), string.Size());
    }
    isize Printv(const char* format, va_list* args)
    {
        return Logv(LogLevel::eNone, format, *args);
    }

    isize Log(LogLevel logLevel, StringView str, bool endl)
    {
        // FIXME(v1tr10l7): locking
        isize nwritten = PrintLogLevel(logLevel);
        nwritten += Print(str);

        if (endl && logLevel != LogLevel::eNone) LogChar('\n');
        return nwritten;
    }
    isize Logf(LogLevel logLevel, const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        auto nwritten = Logv(logLevel, fmt, args);
        va_end(args);

        return nwritten;
    }

    isize PrintArgument(StringView::Iterator& it, va_list& args,
                        PrintfFormatSpec& specs)
    {
        isize nwritten = 0;

        switch (specs.Type)
        {
            case ArgumentType::eChar:
            {
                i32 c = va_arg(args, i32);
                LogChar(c);
                ++nwritten;
                break;
            }
            case ArgumentType::eInteger:
                nwritten += LogNumber<int>(args, specs);
                break;
            case ArgumentType::eLong:
                nwritten += LogNumber<long>(args, specs);
                break;
            case ArgumentType::eLongLong:
                nwritten += LogNumber<long long>(args, specs);
                break;
            case ArgumentType::eSize:
                nwritten += LogNumber<isize>(args, specs);
                break;
            case ArgumentType::eUnsignedChar:
            {
                i32 c = va_arg(args, i32);
                LogChar(c);
                ++nwritten;
                break;
            }
            case ArgumentType::eUnsignedInteger:
                nwritten += LogNumber<unsigned int>(args, specs);
                break;
            case ArgumentType::eUnsignedLong:
                nwritten += LogNumber<unsigned long>(args, specs);
                break;
            case ArgumentType::eUnsignedLongLong:
                nwritten += LogNumber<unsigned long long>(args, specs);
                break;
            case ArgumentType::eUnsignedSize:
                nwritten += LogNumber<usize>(args, specs);
                break;
            case ArgumentType::eString:
            {
                StringView string = va_arg(args, const char*);
                usize      size   = string.Size();
                if (specs.Precision > 0
                    && specs.Precision < static_cast<isize>(size))
                    size = static_cast<usize>(specs.Precision);
                if (size == 0) break;

                for (const auto& c : string.Substr(0, size))
                    LogChar(c), ++nwritten;
                break;
            }
            case ArgumentType::eOutWrittenCharCount:
            {
                i32* out = va_arg(args, i32*);
                *out     = nwritten;
                break;
            }

            default: break;
        }

        return nwritten;
    }
    isize Logv(LogLevel level, const char* fmt, va_list& args,
               bool printNewline)
    {
        // FIXME(v1tr10l7): ScopedLock guard(s_Lock, true);
        PrintLogLevel(level);

        isize nwritten = 0;
        auto  it       = fmt;
        while (*it)
        {
            if (*it == '%' && *(it + 1) != '%')
            {
                PrintfFormatParser parser;
                auto               specs = parser(it, args);

                nwritten += PrintArgument(it, args, specs);
                continue;
            }
            else if (*it == '\n')
            {
                LogChar('\r');
                LogChar('\n');
                it++;
                nwritten += 2;
                continue;
            }
            LogChar(*it++), nwritten++;
        }

        if (printNewline) LogChar('\n'), ++nwritten;
        return nwritten;
    }
#endif

#if PRISM_TARGET_CRYPTIX != 0
    isize Log(LogLevel logLevel, StringView str, bool endl)
    {
        return Logger::Log(logLevel, str, endl);
    }
#endif
    void Print(LogLevel logLevel, StringView str)
    {
#if PRISM_TARGET_CRYPTIX != 0
        Logger::Log(logLevel, str);
#else
        PrintLogLevel(logLevel);
        std::printf("%s", str.Raw());

        std::putchar('\n');
#endif
    }
}; // namespace Prism::Log
