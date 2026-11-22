/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>

#include <Prism/Debug/Log.hpp>

export module Prism.Debug.Log;

export import Prism.String.FormatString;
export import Prism.String.StringView;
export import Prism.String.Formatter;

export extern "C++"
{
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

    namespace Prism
    {
        // Here rather than in Formatter.hpp, which can't depend on Log.
        /// Formats straight to Log::Print, in chunks, without allocating.
        inline void VPrint(StringView format, FormatArgs args)
        {
            VFormatToSink([](const char* data, usize size)
                          { Log::Print(StringView(data, size)); },
                          format, args);
        }
        template <typename... Args>
        void Print(FormatString<Args...> format, Args&&... args)
        {
            VPrint(format.View(), MakeFormatArgs(args...));
        }
        template <typename... Args>
        void Println(FormatString<Args...> format, Args&&... args)
        {
            VPrint(format.View(), MakeFormatArgs(args...));
            Log::Print("\n");
        }

        template <typename... Args>
        void Print(const TextStyle& style, FormatString<Args...> format,
                   Args&&... args)
        {
            VFormatToSink([](const char* data, usize size)
                          { Log::Print(StringView(data, size)); }, style,
                          format.View(), MakeFormatArgs(args...));
        }
    }; // namespace Prism
}
