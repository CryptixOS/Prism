/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Log.hpp>

#include <mutex>

namespace Prism
{
    class LogSink
    {
      public:
        isize           Log(LogLevel level, StringView message);

        void           PutChar(u64 c);

        virtual isize   WriteNoLock(StringView str) = 0;

        void           EndOfLine() { WriteNoLock("\n"); }

        constexpr bool IsEnabled() const { return m_Enabled; }

        bool           Enable();
        bool           Disable();

      protected:
        // std::mutex m_Lock;
        bool m_Enabled = true;

        isize PrintLevel(LogLevel level);
    };
}; // namespace Prism
