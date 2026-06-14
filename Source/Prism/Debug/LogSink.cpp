/*
 * Created by v1tr10l7 on 13.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Debug/Log.hpp>
#include <Prism/Debug/LogSink.hpp>

namespace Prism
{

    void StdOutSink::Log(const LogMessage& message)
    {
        Log::Log(message.Level, message.Payload, true);
    }
    void StdOutSink::Flush()
    {
        // FIXME(v1tr10l7): flush
    }
    void StdOutSink::SetPattern(StringView pattern) { IgnoreUnused(pattern); }
}; // namespace Prism
