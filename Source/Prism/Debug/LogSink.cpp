/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Debug/LogSink.hpp>

#include <magic_enum/magic_enum.hpp>
#include <utility>

namespace Prism
{
    void LogSink::Log(LogLevel level, std::string_view message)
    {
        std::unique_lock guard(m_Lock);

        PrintLevel(level);
        WriteNoLock(message);

        EndOfLine();
    }

    void LogSink::PutChar(u64 c)
    {
        WriteNoLock(reinterpret_cast<const char*>(c));
    }
    bool LogSink::Enable()
    {
        std::unique_lock guard(m_Lock);

        return std::exchange(m_Enabled, true);
    }
    bool LogSink::Disable()
    {
        std::unique_lock guard(m_Lock);

        return std::exchange(m_Enabled, false);
    }

    void LogSink::PrintLevel(LogLevel level)
    {

        if (level == LogLevel::eNone) return;
        PutChar('[');

        WriteNoLock(magic_enum::enum_name(level).data() + 1);
        WriteNoLock("]: ");
    }
}; // namespace Prism
