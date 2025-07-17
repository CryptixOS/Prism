/*
 * Created by v1tr10l7 on 17.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Debug/Assertions.hpp>
#include <Prism/Utility/Atomic.hpp>

namespace Prism
{
    class RefCounted
    {
      public:
        virtual ~RefCounted() { PrismAssert(m_RefCount == 0); }

        usize RefCount() const { return m_RefCount.Load(); }

        void  Ref() { ++m_RefCount; }
        void  Unref() { --m_RefCount; }

      private:
        Atomic<usize> m_RefCount = 0;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::RefCounted;
#endif
