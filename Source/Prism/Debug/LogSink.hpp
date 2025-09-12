/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Log.hpp>

namespace Prism
{
    template <typename LockingPolicy>
    class LogSink
    {
      public:
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

        virtual isize  WriteNoLock(StringView str) = 0;

        void           EndOfLine() { WriteNoLock("\n"); }

        constexpr bool IsEnabled() const { return m_Enabled; }

        bool           Enable() { return Exchange(m_Enabled, true); }
        bool           Disable() { return Exchange(m_Enabled, false); }

      protected:
        LockingPolicy m_Lock;
        bool          m_Enabled = true;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::LogSink;
#endif
