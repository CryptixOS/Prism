/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/String/StringView.hpp>

namespace Prism
{
    enum class LogLevel
    {
        eNone  = 0,
        eDebug = 1,
        eTrace = 2,
        eInfo  = 3,
        eWarn  = 4,
        eError = 5,
        eFatal = 6,
    };

    namespace Log
    {
        void  LogChar(u64 c);
        isize Print(StringView string);
        isize Printv(const char* format, VaList* args);
        isize Log(LogLevel logLevel, StringView string, bool endl = true);
        isize Logf(LogLevel logLevel, const char* fmt, ...);
        isize Logv(LogLevel logLevel, const char* fmt, VaList& args,
                   bool endl = true);

        void  Print(LogLevel logLevel, StringView str);

#if PRISM_DISABLE_FMT == 0
        template <typename... Args>
        inline void Print(LogLevel logLevel, fmt::format_string<Args...> format,
                          Args&&... args)
        {
            auto formattedString = fmt::format(format, Forward<Args>(args)...);
            Print(logLevel,
                  StringView(formattedString.data(), formattedString.size()));
        }

        template <typename... Args>
        inline void Log(LogLevel level, fmt::format_string<Args...> format,
                        Args&&... args)
        {
            auto formattedString = fmt::format(format, Forward<Args>(args)...);
            Log(level,
                StringView(formattedString.data(), formattedString.size()));
        }

        template <typename... Args>
        inline void Message(fmt::format_string<Args...> format, Args&&... args)
        {
            Log(LogLevel::eNone, format, Forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Debug(fmt::format_string<Args...> format, Args&&... args)
        {
            Log(LogLevel::eDebug, format, Forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Trace(fmt::format_string<Args...> format, Args&&... args)
        {
            Log(LogLevel::eTrace, format, Forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Info(fmt::format_string<Args...> format, Args&&... args)
        {
            Log(LogLevel::eInfo, format, Forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Warn(fmt::format_string<Args...> format, Args&&... args)
        {
            Log(LogLevel::eWarn, format, Forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Error(fmt::format_string<Args...> format, Args&&... args)
        {
            Log(LogLevel::eError, format, Forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Fatal(fmt::format_string<Args...> format, Args&&... args)
        {
            Log(LogLevel::eFatal, format, Forward<Args>(args)...);
        }
#endif
    }; // namespace Log
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX
namespace Log = Prism::Log;
#endif

#ifdef PRISM_BUILD_DEBUG
    #define PrismDebug(...)        Log::Debug(__VA_ARGS__)
    #define PrismDebugNoAlloc(...) Log::Printf(LogLevel::eDebug, __VA_ARGS__)
#else
    #define PrismDebug(...)
    #define PrismDebugNoAlloc(...)
#endif

#define PrismMessage(...)        Log::Message(__VA_ARGS__)
#define PrismMessageNoAlloc(...) Log::Printf(LogLevel::eNone, __VA_ARGS__)

#if PRISM_LOG_ENABLE != 0
    #define PrismTraceNoAlloc(...) Log::Logf(LogLevel::eTrace, __VA_ARGS__)
    #define PrismInfoNoAlloc(...)  Log::Logf(LogLevel::eInfo, __VA_ARGS__)
    #define PrismWarnNoAlloc(...)  Log::Logf(LogLevel::eWarn, __VA_ARGS__)
    #define PrismErrorNoAlloc(...) Log::Logf(LogLevel::eError, __VA_ARGS__)
    #define PrismFatalNoAlloc(...) Log::Logf(LogLevel::eFatal, __VA_ARGS__)

    #define PrismTrace(...)        Log::Trace(__VA_ARGS__)
    #define PrismInfo(...)         Log::Info(__VA_ARGS__)
    #define PrismWarn(...)         Log::Warn(__VA_ARGS__)
    #define PrismError(...)        Log::Error(__VA_ARGS__)
    #define PrismFatal(...)        Log::Fatal(__VA_ARGS__)
#else
    #define PrismTraceNoAlloc(...)
    #define PrismInfoNoAlloc(...)
    #define PrismWarnNoAlloc(...)
    #define PrismErrorNoAlloc(...)
    #define PrismFatalNoAlloc(...)

    #define PrismTrace(...)
    #define PrismInfo(...)
    #define PrismWarn(...)
    #define PrismError(...)
    #define PrismFatal(...)
#endif
