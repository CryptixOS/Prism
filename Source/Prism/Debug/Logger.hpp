/*
 * Created by v1tr10l7 on 23.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Debug/LogSink.hpp>
#include <Prism/Memory/Ref.hpp>
#include <Prism/String/String.hpp>

namespace Prism
{
    template <typename LockingPolicy>
    class Logger
    {
      public:
        Logger(StringView name, bool enableStdOut = true)
            : m_Name(name)
        {
            m_StdOut = CreateRef<StdOutSink>();
            AddSink(m_StdOut.Raw());
        }

        void Log(LogLevel level, StringView payload)
        {
            LogMessage message;
            message.Level   = level;
            message.Payload = payload;
            for (auto sink : m_Sinks) sink->Log(message);
        }
        void AddSink(LogSinkBase* sink)
        {
            m_Lock.Lock();
            m_Sinks.PushBack(sink);
            m_Lock.Unlock();
        }

      private:
        String               m_Name;
        Ref<StdOutSink>      m_StdOut;
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
