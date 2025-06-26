/*
 * Created by v1tr10l7 on 23.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Debug/Log.hpp>
#include <Prism/Debug/Logger.hpp>

namespace Prism
{
    Logger::Logger(StringView name, bool endl)
        : m_Name(name)
        , m_PrintNewLine(endl)
    {
    }

    void Logger::operator<<(StringView message) const
    {
        Log::Print(LogLevel::eNone, "[{}{}\e]: ", m_LabelColor, m_Name);
        Log::Print(LogLevel::eNone, "{}{}\e", m_TextColor, message);
        if (m_PrintNewLine) Log::Message("\n");
    }
}; // namespace Prism
