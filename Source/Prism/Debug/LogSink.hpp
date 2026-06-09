/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Log.hpp>
#include <Prism/Utility/Atomic.hpp>
#include <Prism/Utility/Time.hpp>

namespace Prism
{
    struct LogMessage
    {
        StringView     LoggerName;
        LogLevel       Level;
        StringView     Payload;
        Time::Timestep Timestamp;
        usize          ThreadID;
    };

    class LogSinkBase
    {
      public:
        virtual ~LogSinkBase()                      = default;

        virtual void Log(const LogMessage& message) = 0;
        virtual void Flush()                        = 0;
        virtual void SetPattern(StringView pattern) = 0;

        inline void  SetLevelFilter(LogLevel level) { m_LevelFilter = level; }
        inline LogLevel LevelFilter() const { return m_LevelFilter; }
        inline bool     ShouldLog(LogLevel level) const
        {
            return level >= m_LevelFilter.Load();
        }

      protected:
        Atomic<LogLevel> m_LevelFilter = LogLevel::eTrace;
    };

    template <typename LockingPolicy>
    class LogSink : public LogSinkBase
    {
      public:
        virtual ~LogSink() override               = default;

        inline LogSink(const LogSink&)            = delete;
        inline LogSink(LogSink&&)                 = delete;

        inline LogSink& operator=(const LogSink&) = delete;
        inline LogSink& operator=(LogSink&&)      = delete;

        inline void     Log(const LogMessage& message) override final
        {
            m_Lock.Lock();
            Write(message);
            m_Lock.Unlock();
        }
        inline void Flush() override final
        {
            m_Lock.Lock();
            FlushIt();
            m_Lock.Unlock();
        }
        inline void SetPattern(StringView) override final
        {
            m_Lock.Lock();
            SetPatternNoLock(StringView());
            m_Lock.Unlock();
        }

        isize Write(const LogMessage& message)
        {
            isize nwritten = 0;
            m_Lock.Lock();
            if (message.Level >= m_LevelFilter) nwritten = SinkIt(message);

            m_Lock.Unlock();
            return nwritten;
        }
        isize Write(StringView message)
        {
            m_Lock.Lock();
            isize nwritten = 0;

            nwritten += WriteNoLock(message);
            EndOfLine();
            ++nwritten;

            m_Lock.Unlock();
            return nwritten;
        }
        void PutChar(u64 c) { WriteNoLock(reinterpret_cast<const char*>(&c)); }

        virtual isize WriteNoLock(StringView str) = 0;
        virtual isize SinkIt(const LogMessage& message)
        {
            return Write(message.Payload);
        }
        virtual void   FlushIt() {}
        virtual void   SetPatternNoLock(StringView) {}

        void           EndOfLine() { WriteNoLock("\n"); }

        constexpr bool IsEnabled() const { return m_Enabled; }

        bool           Enable() { return m_Enabled.Exchange(true); }
        bool           Disable() { return m_Enabled.Exchange(false); }

      protected:
        LockingPolicy m_Lock;
        Atomic<bool>  m_Enabled = true;
    };
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::LogSinkBase;
using Prism::LogSink;
#endif
