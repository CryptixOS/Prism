module;

#include <Prism/Core/Types.hpp>
#if defined(__linux__ ) && PRISM_TARGET_CRYPTIX == 0
    #include <unistd.h>
#endif

export module Prism.Debug.Logger;

export import Prism.Debug.LogSink;
export import Prism.Memory.Ref;
export import Prism.String.String;
export import Prism.String.Formatter;
export import Prism.Utility.Delegate;

/*
 * Created by v1tr10l7 on 23.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

export extern "C++"
{
    namespace Prism
    {
        template <typename... Args>
        using LoggerFormatString = FormatString<Args...>;

        template <typename LockingPolicy>
        class Logger
        {
          public:
            Logger(StringView name)
                : m_Name(name)
            {
            }

            inline StringView Name() const { return m_Name; }
            inline LogLevel   LogLevelFilter() const { return m_LogLevelFilter; }

            inline void       SetName(StringView name) { m_Name = name; }
            inline void       SetLogLevelFilter(LogLevel logLevelFilter)
            {
                m_LogLevelFilter = logLevelFilter;
            }

            void Log(LogLevel level, StringView payload, StringView tag = "")
            {
                if (level < m_LogLevelFilter) return;

                LogMessage message;
                message.LoggerName = m_Name;
                message.Tag        = tag;
                message.Level      = level;
                message.Payload    = payload;

#if PRISM_TARGET_CRYPTIX == 0 && defined(__linux__)
                struct timespec ts;
                if (clock_gettime(CLOCK_REALTIME, &ts) == 0)
                {
                    const u64 totalNanos
                        = static_cast<u64>(ts.tv_sec) * 1000000000ull
                        + static_cast<u64>(ts.tv_nsec);
                    message.Timestamp = Time::Timestep(totalNanos);
                }

                message.ThreadID = gettid();
#endif

                ScopeLock<LockingPolicy> guard(m_Lock);
                for (auto sink : m_Sinks)
                {
                    if (sink) sink->Log(message);
                }
            }

            void AddSink(LogSinkBase* sink)
            {
                ScopeLock<LockingPolicy> guard(m_Lock);
                m_Sinks.PushBack(sink);
            }

            void RemoveAllSinks()
            {
                ScopeLock<LockingPolicy> guard(m_Lock);
                m_Sinks.Clear();
            }

            void ForEachSink(Delegate<IterationResult(LogSinkBase*)> fn)
            {
                ScopeLock<LockingPolicy> guard(m_Lock);
                for (auto sink : m_Sinks)
                {
                    auto result = fn(sink);
                    if (result == IterationResult::eBreak) return;
                }
            }

            template <typename... Args>
            inline void Log(LoggerFormatString<Args...> format, Args&&... args)
            {
                Log(LogLevel::eNone, format, Forward<Args>(args)...);
            }
            template <typename... Args>
            inline void Log(LogLevel logLevel, LoggerFormatString<Args...> format,
                            Args&&... args)
            {
                if (logLevel < m_LogLevelFilter) return;
                FormatAndLog(logLevel, "", format, Forward<Args>(args)...);
            }

            template <typename... Args>
            inline void Debug(LoggerFormatString<Args...> format, Args&&... args)
            {
                Log(LogLevel::eDebug, format, Forward<Args>(args)...);
            }
            template <typename... Args>
            inline void Trace(LoggerFormatString<Args...> format, Args&&... args)
            {
                Log(LogLevel::eTrace, format, Forward<Args>(args)...);
            }
            template <typename... Args>
            inline void Info(LoggerFormatString<Args...> format, Args&&... args)
            {
                Log(LogLevel::eInfo, format, Forward<Args>(args)...);
            }
            template <typename... Args>
            inline void Warn(LoggerFormatString<Args...> format, Args&&... args)
            {
                Log(LogLevel::eWarn, format, Forward<Args>(args)...);
            }
            template <typename... Args>
            inline void Error(LoggerFormatString<Args...> format, Args&&... args)
            {
                Log(LogLevel::eError, format, Forward<Args>(args)...);
            }
            template <typename... Args>
            inline void Fatal(LoggerFormatString<Args...> format, Args&&... args)
            {
                Log(LogLevel::eFatal, format, Forward<Args>(args)...);
            }

            template <typename... Args>
            inline void LogTagged(LoggerFormatString<Args...> format,
                                  Args&&... args)
            {
                Log(LogLevel::eNone, format, Forward<Args>(args)...);
            }
            template <typename... Args>
            inline void LogTagged(LogLevel logLevel, StringView tag,
                                  LoggerFormatString<Args...> format,
                                  Args&&... args)
            {
                if (logLevel < m_LogLevelFilter) return;
                FormatAndLog(logLevel, tag, format, Forward<Args>(args)...);
            }

            template <typename... Args>
            inline void DebugTag(StringView tag, LoggerFormatString<Args...> format,
                                 Args&&... args)
            {
                LogTagged(LogLevel::eDebug, tag, format, Forward<Args>(args)...);
            }
            template <typename... Args>
            inline void TraceTag(StringView tag, LoggerFormatString<Args...> format,
                                 Args&&... args)
            {
                LogTagged(LogLevel::eTrace, tag, format, Forward<Args>(args)...);
            }
            template <typename... Args>
            inline void InfoTag(StringView tag, LoggerFormatString<Args...> format,
                                Args&&... args)
            {
                LogTagged(LogLevel::eInfo, tag, format, Forward<Args>(args)...);
            }
            template <typename... Args>
            inline void WarnTag(StringView tag, LoggerFormatString<Args...> format,
                                Args&&... args)
            {

                LogTagged(LogLevel::eWarn, tag, format, Forward<Args>(args)...);
            }
            template <typename... Args>
            inline void ErrorTag(StringView tag, LoggerFormatString<Args...> format,
                                 Args&&... args)
            {
                LogTagged(LogLevel::eError, tag, format, Forward<Args>(args)...);
            }
            template <typename... Args>
            inline void FatalTag(StringView tag, LoggerFormatString<Args...> format,
                                 Args&&... args)
            {
                LogTagged(LogLevel::eFatal, tag, format, Forward<Args>(args)...);
            }

          private:
            template <typename... Args>
            inline void FormatAndLog(LogLevel logLevel, StringView tag,
                                     LoggerFormatString<Args...> format,
                                     Args&&... args)
            {
                String formatted = Prism::Format(format, Forward<Args>(args)...);
                Log(logLevel, formatted.View(), tag);
            }

            String               m_Name;
            LogLevel             m_LogLevelFilter = LogLevel::eTrace;
            Vector<LogSinkBase*> m_Sinks;
            LockingPolicy        m_Lock;
        };
        class NamedLogger
        {
          public:
            NamedLogger(StringView name, bool endl = false);

            constexpr inline StringView Name() const PM_NOEXCEPT { return m_Name; }

            constexpr inline StringView LabelColor() const PM_NOEXCEPT
            {
                return m_LabelColor;
            }

            constexpr inline StringView TextColor() const PM_NOEXCEPT
            {
                return m_TextColor;
            }

            constexpr inline void SetName(StringView name) { m_Name = name; }
            constexpr inline void SetLabelColor(StringView color)
            {
                m_LabelColor = color;
            }
            constexpr inline void SetTextColor(StringView color)
            {
                m_TextColor = color;
            }

            void operator<<(StringView string) const;

          private:
            String     m_Name         = "";
            bool       m_PrintNewLine = false;

            StringView m_TextColor    = "\e[0m";
            StringView m_LabelColor   = "\e[0m\e[35";
        };
    }; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    using Prism::Logger;
#endif
}
