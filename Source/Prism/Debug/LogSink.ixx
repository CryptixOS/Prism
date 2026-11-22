/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Log.hpp>

export module Prism.Debug.LogSink;

export import Prism.Debug.Log;
export import Prism.Memory.RefCounted;
export import Prism.Utility.Atomic;
export import Prism.Utility.Time;

export extern "C++"
{
    namespace Prism
    {
        struct LogMessage
        {
            StringView     LoggerName;
            StringView     Tag;
            LogLevel       Level;
            StringView     Payload;
            Time::Timestep Timestamp;
            usize          ThreadID;

            StringView     SourceFile;
            usize          SourceLine;
        };

        // FIXME(v1tr10l7): Move it somewhere else
        template <typename Lockable>
        struct ScopeLock
        {
            ScopeLock(Lockable& lock)
                : m_Lock(lock)
            {
                m_Lock.Lock();
            }
            ~ScopeLock() { m_Lock.Unlock(); }

            ScopeLock(const ScopeLock&)            = delete;
            ScopeLock& operator=(const ScopeLock&) = delete;

          private:
            Lockable& m_Lock;
        };

        class LogSinkBase : public RefCounted<LogSinkBase>
        {
          public:
            virtual ~LogSinkBase()                      = default;

            virtual void Log(const LogMessage& message) = 0;
            virtual void Flush()                        = 0;
            virtual void SetPattern(StringView pattern) = 0;

            inline void  SetLevelFilter(LogLevel level)
            {
                m_LevelFilter.Store(level);
            }
            inline LogLevel LevelFilter() const { return m_LevelFilter.Load(); }
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
            inline LogSink()                          = default;
            virtual ~LogSink() override               = default;

            inline LogSink(const LogSink&)            = delete;
            inline LogSink(LogSink&&)                 = delete;

            inline LogSink& operator=(const LogSink&) = delete;
            inline LogSink& operator=(LogSink&&)      = delete;

            inline void     Log(const LogMessage& message) override final
            {
                if (!m_Enabled.Load() || !ShouldLog(message.Level)) return;

                ScopeLock<LockingPolicy> guard(m_Lock);
                SinkIt(message);
            }

            inline void Flush() override final
            {
                ScopeLock<LockingPolicy> guard(m_Lock);
                FlushIt();
            }

            inline void SetPattern(StringView pattern) override final
            {
                ScopeLock<LockingPolicy> guard(m_Lock);
                SetPatternNoLock(pattern);
            }

            isize Write(const LogMessage& message)
            {
                ScopeLock<LockingPolicy> guard(m_Lock);
                isize                    nwritten = WriteNoLock(message.Payload);
                nwritten += WriteNoLock("\n");
                return nwritten;
            }

            void PutChar(u64 c)
            {
                ScopeLock<LockingPolicy> guard(m_Lock);
                WriteNoLock(StringView(reinterpret_cast<const char*>(&c), 1));
            }
            virtual isize WriteNoLock(StringView str)
            {
                LogMessage message;
                message.Payload = str;

                return SinkIt(message);
            }
            virtual isize SinkIt(const LogMessage& message) = 0;
            virtual void  FlushIt() {}
            virtual void  SetPatternNoLock(StringView pattern)
            {
                IgnoreUnused(pattern);
            }

            void           EndOfLine() { WriteNoLock("\n"); }

            constexpr bool IsEnabled() const { return m_Enabled.Load(); }

            inline bool    Enable() { return m_Enabled.Exchange(true); }
            inline bool    Disable() { return m_Enabled.Exchange(false); }

          protected:
            LockingPolicy m_Lock;
            Atomic<bool>  m_Enabled = true;
        };

        class __attribute__((visibility("default"))) StdOutSink final
            : public LogSinkBase
        {
          public:
            virtual ~StdOutSink() override;

#if 1
            virtual void Log(const LogMessage& message) override final;
            virtual void Flush() override final;
            virtual void SetPattern(StringView pattern) override final;
#else
            virtual void Log(const LogMessage& message) override final
            {
                IgnoreUnused(message);
            }
            virtual void Flush() override final {}
            virtual void SetPattern(StringView pattern) override final
            {
                IgnoreUnused(pattern);
            }
#endif
        };
    }; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    using Prism::LogMessage;
    using Prism::LogSink;
    using Prism::LogSinkBase;
    using Prism::StdOutSink;
#endif
}
