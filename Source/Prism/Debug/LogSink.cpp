/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Debug/LogSink.hpp>
#include <Prism/String/StringUtils.hpp>

namespace Prism
{
    using namespace StringUtils;

    isize LogSink::Log(LogLevel level, StringView message)
    {
        // std::unique_lock guard(m_Lock);
        isize nwritten = 0;
        
        nwritten += PrintLevel(level);
        nwritten += WriteNoLock(message);

        EndOfLine();
        ++nwritten;
        return nwritten;
    }

    void LogSink::PutChar(u64 c)
    {
        WriteNoLock(reinterpret_cast<const char*>(&c));
    }
    bool LogSink::Enable()
    {
        // std::unique_lock guard(m_Lock);

        return Exchange(m_Enabled, true);
    }
    bool LogSink::Disable()
    {
        // std::unique_lock guard(m_Lock);

        return Exchange(m_Enabled, false);
    }

    isize LogSink::PrintLevel(LogLevel level)
    {

        if (level == LogLevel::eNone) return 0;
        PutChar('[');
        isize nwritten = 1;

        nwritten += WriteNoLock(ToString(level).Substr(1));
        nwritten += WriteNoLock("]: ");
        return nwritten;
    }
}; // namespace Prism
