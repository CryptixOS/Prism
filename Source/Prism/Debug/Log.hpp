/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/String/FormatString.hpp>
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
        isize Log(LogLevel logLevel, StringView tag, StringView string,
                  bool endl = true);
        isize Logf(LogLevel logLevel, const char* fmt, ...);
        isize Logv(LogLevel logLevel, const char* fmt, VaList& args,
                   bool endl = true);

        void  Print(LogLevel logLevel, StringView str);

        template <typename... Args>
        inline void Print(LogLevel logLevel, FormatString<Args...> format,
                          Args&&... args)
        {
            // Unqualified so ADL finds Prism::Format at instantiation:
            // Formatter.hpp includes this header, so it may not be declared
            // yet.
            String formattedString = Format(format, Forward<Args>(args)...);
            Print(logLevel, formattedString.View());
        }

        template <typename... Args>
        inline void Log(LogLevel level, FormatString<Args...> format,
                        Args&&... args)
        {
            String formattedString = Format(format, Forward<Args>(args)...);
            Log(level, formattedString.View());
        }

        template <typename... Args>
        inline void Message(FormatString<Args...> format, Args&&... args)
        {
            Log(LogLevel::eNone, format, Forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Debug(FormatString<Args...> format, Args&&... args)
        {
            Log(LogLevel::eDebug, format, Forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Trace(FormatString<Args...> format, Args&&... args)
        {
            Log(LogLevel::eTrace, format, Forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Info(FormatString<Args...> format, Args&&... args)
        {
            Log(LogLevel::eInfo, format, Forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Warn(FormatString<Args...> format, Args&&... args)
        {
            Log(LogLevel::eWarn, format, Forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Error(FormatString<Args...> format, Args&&... args)
        {
            Log(LogLevel::eError, format, Forward<Args>(args)...);
        }
        template <typename... Args>
        inline void Fatal(FormatString<Args...> format, Args&&... args)
        {
            Log(LogLevel::eFatal, format, Forward<Args>(args)...);
        }
    }; // namespace Log
}; // namespace Prism

#if PRISM_USE_NAMESPACE
namespace Log = Prism::Log;
#endif

// Last, since Formatter.hpp depends on the declarations above.
#include <Prism/String/Formatter.hpp>

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
