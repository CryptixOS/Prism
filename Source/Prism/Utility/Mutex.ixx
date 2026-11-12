/*
 * Created by v1tr10l7 on 09.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

#if PRISM_TARGET_CRYPTIX == 0 && PRISM_TARGET_CARBONC == 0                   \
    && PrismHasInclude(<sched.h>)
    #include <sched.h>
    #define PRISM_MUTEX_CAN_YIELD 1
#else
    #define PRISM_MUTEX_CAN_YIELD 0
#endif

export module Prism.Utility.Mutex;

export extern "C++"
{
    namespace Prism
    {
        namespace Details
        {
            /// Tells the CPU we're spinning (x86 `pause`, ARM `yield`).
            PM_ALWAYS_INLINE void CpuRelax()
            {
#if defined(__x86_64__) || defined(__i386__)
                __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(__arm__)
                __asm__ __volatile__("yield");
#endif
            }
        } // namespace Details

        /**
         * A test-and-test-and-set spinlock. Works everywhere, including the
         * kernel; only hold it for short critical sections.
         */
        class Spinlock
        {
          public:
            constexpr Spinlock()                = default;
            Spinlock(const Spinlock&)           = delete;
            Spinlock& operator=(const Spinlock&) = delete;

            void Lock()
            {
                for (;;)
                {
                    if (!__atomic_exchange_n(&m_Locked, true, __ATOMIC_ACQUIRE))
                        return;
                    while (__atomic_load_n(&m_Locked, __ATOMIC_RELAXED))
                        Details::CpuRelax();
                }
            }
            bool TryLock()
            {
                return !__atomic_load_n(&m_Locked, __ATOMIC_RELAXED)
                    && !__atomic_exchange_n(&m_Locked, true, __ATOMIC_ACQUIRE);
            }
            void Unlock() { __atomic_store_n(&m_Locked, false, __ATOMIC_RELEASE); }

            bool IsLocked() const
            {
                return __atomic_load_n(&m_Locked, __ATOMIC_RELAXED);
            }

          private:
            bool m_Locked = false;
        };

        /**
         * A mutual exclusion lock. On hosted targets a waiting thread spins
         * briefly, then yields its time slice to the scheduler; elsewhere it
         * behaves like a Spinlock. Satisfies LockingPolicy, so it works with
         * ScopedLock.
         */
        class Mutex
        {
          public:
            constexpr Mutex()             = default;
            Mutex(const Mutex&)           = delete;
            Mutex& operator=(const Mutex&) = delete;

            void Lock()
            {
                for (u32 spins = 0;; ++spins)
                {
                    if (m_Lock.TryLock()) return;
                    if (spins < SpinsBeforeYield) Details::CpuRelax();
                    else
                    {
#if PRISM_MUTEX_CAN_YIELD != 0
                        sched_yield();
#else
                        Details::CpuRelax();
#endif
                    }
                }
            }
            bool TryLock() { return m_Lock.TryLock(); }
            void Unlock() { m_Lock.Unlock(); }
            bool IsLocked() const { return m_Lock.IsLocked(); }

          private:
            static constexpr u32 SpinsBeforeYield = 64;

            Spinlock             m_Lock;
        };
    }; // namespace Prism
}
