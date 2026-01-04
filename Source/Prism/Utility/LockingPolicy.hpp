/*
 * Created by v1tr10l7 on 08.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Concepts.hpp>
#include <Prism/Core/NonCopyable.hpp>

namespace Prism
{
    template <typename T>
    concept LockingPolicy = requires(T lock) {
        { lock.Lock() } -> SameAs<void>;
        { lock.Unlock() } -> SameAs<void>;
    };

    struct NoLock
    {
        void Lock() {}
        void Unlock() {}
    };

    template <typename LockType>
    class ScopedLock : public NonCopyable<ScopedLock<LockType>>
    {
      public:
        explicit ScopedLock(LockType& lock)
            : m_Lock(lock)
        {
            m_Lock.Lock();
        }

        ~ScopedLock() { m_Lock.Unlock(); }

      private:
        LockType& m_Lock;
    };
}; // namespace Prism
