/*
 * Created by v1tr10l7 on 21.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Concepts.hpp>

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

#include <Prism/Utility/AtomicBuiltins.hpp>

namespace Prism
{
    template <typename T>
    class Atomic
    {
      public:
        using ValueType                = T;

        constexpr Atomic() PM_NOEXCEPT = default;
        constexpr Atomic(T desired) PM_NOEXCEPT : m_Value(desired) {}
        Atomic(const Atomic& other) = delete;

        inline constexpr T operator=(T desired) PM_NOEXCEPT
        {
            T value = Load();
            Store(desired);

            return value;
        }
        inline constexpr T operator=(T desired) volatile PM_NOEXCEPT
        {
            T value = Load();
            Store(desired);

            return value;
        }
        Atomic&               operator=(const Atomic&)          = delete;
        Atomic&               operator=(const Atomic&) volatile = delete;

        inline constexpr bool IsLockFree() const PM_NOEXCEPT
        {
            return AtomicAlwaysLockFree(sizeof(T), &m_Value);
        }
        inline constexpr bool IsLockFree() const volatile PM_NOEXCEPT
        {
            return AtomicAlwaysLockFree(sizeof(T), &m_Value);
        }

        constexpr void Store(T           desired,
                             MemoryOrder order
                             = MemoryOrder::eAtomicSeqCst) PM_NOEXCEPT
        {
            AtomicStore(m_Value, desired, order);
        }
        constexpr void Store(T           desired,
                             MemoryOrder order
                             = MemoryOrder::eAtomicSeqCst) volatile PM_NOEXCEPT
        {
            AtomicStore(m_Value, desired, order);
        }

        constexpr T Load(MemoryOrder order
                         = MemoryOrder::eAtomicSeqCst) const PM_NOEXCEPT
        {
            return AtomicLoad<T>(m_Value, order);
        }
        constexpr T Load(MemoryOrder order = MemoryOrder::eAtomicSeqCst) const
            volatile PM_NOEXCEPT
        {
            return AtomicLoad<T>(m_Value, order);
        }
        constexpr   operator T() const PM_NOEXCEPT { return Load(); }
        constexpr   operator T() const volatile PM_NOEXCEPT { return Load(); }

        constexpr T Exchange(T           desired,
                             MemoryOrder order
                             = MemoryOrder::eAtomicSeqCst) PM_NOEXCEPT
        {
            return AtomicExchange<T, T>(m_Value, desired, order);
        }
        constexpr T Exchange(T           desired,
                             MemoryOrder order
                             = MemoryOrder::eAtomicSeqCst) volatile PM_NOEXCEPT
        {
            return AtomicExchange<T, T>(m_Value, desired, order);
        }
        constexpr bool CompareExchange(T& expected, T desired, bool weak,
                                       MemoryOrder successOrder,
                                       MemoryOrder failureOrder)
        {
            return AtomicCompareExchange<T, T>(m_Value, expected, desired, weak,
                                               successOrder, failureOrder);
        }

        constexpr static bool IsAlwaysLockFree() PM_NOEXCEPT
        {
            T value;
            return AtomicAlwaysLockFree(sizeof(T), &value);
        }

        constexpr T FetchAdd(T rhs, MemoryOrder order
                                    = MemoryOrder::eAtomicSeqCst) PM_NOEXCEPT
        {
            return AtomicFetchAdd<T, T>(m_Value, rhs, order);
        }
        constexpr T FetchAdd(T           rhs,
                             MemoryOrder order
                             = MemoryOrder::eAtomicSeqCst) volatile PM_NOEXCEPT
        {
            return AtomicFetchAdd<T, T>(m_Value, rhs, order);
        }
        constexpr T FetchSub(T rhs, MemoryOrder order
                                    = MemoryOrder::eAtomicSeqCst) PM_NOEXCEPT
        {
            return AtomicFetchSub<T, T>(m_Value, rhs, order);
        }
        constexpr T FetchSub(T           rhs,
                             MemoryOrder order
                             = MemoryOrder::eAtomicSeqCst) volatile PM_NOEXCEPT
        {
            return AtomicFetchSub<T, T>(m_Value, rhs, order);
        }
        constexpr T FetchAnd(T rhs, MemoryOrder order
                                    = MemoryOrder::eAtomicSeqCst) PM_NOEXCEPT
            requires(IsIntegral<T>)
        {
            return AtomicFetchAnd<T, T>(m_Value, rhs, order);
        }
        constexpr T FetchAnd(T           rhs,
                             MemoryOrder order
                             = MemoryOrder::eAtomicSeqCst) volatile PM_NOEXCEPT
            requires(IsIntegral<T>)
        {
            return AtomicFetchAnd<T, T>(m_Value, rhs, order);
        }
        constexpr T FetchOr(T rhs, MemoryOrder order
                                   = MemoryOrder::eAtomicSeqCst) PM_NOEXCEPT
            requires(IsIntegral<T>)
        {
            return AtomicFetchOr<T, T>(m_Value, rhs, order);
        }
        constexpr T FetchOr(T           rhs,
                            MemoryOrder order
                            = MemoryOrder::eAtomicSeqCst) volatile PM_NOEXCEPT
            requires(IsIntegral<T>)
        {
            return AtomicFetchOr<T, T>(m_Value, rhs, order);
        }
        constexpr T FetchXor(T rhs, MemoryOrder order
                                    = MemoryOrder::eAtomicSeqCst) PM_NOEXCEPT
            requires(IsIntegral<T>)
        {
            return AtomicFetchXor<T, T>(m_Value, rhs, order);
        }
        constexpr T FetchXor(T           rhs,
                             MemoryOrder order
                             = MemoryOrder::eAtomicSeqCst) volatile PM_NOEXCEPT
            requires(IsIntegral<T>)
        {
            return AtomicFetchXor<T, T>(m_Value, rhs, order);
        }

        constexpr T operator++() PM_NOEXCEPT
        {
            T increment = static_cast<T>(1);
            T discarded = FetchAdd(increment);

            (void)discarded;
            return Load();
        }
        constexpr T operator++(int) PM_NOEXCEPT
        {
            T increment = static_cast<T>(1);
            return FetchAdd(increment);
        }
        constexpr T operator--() PM_NOEXCEPT
        {
            T decrement = static_cast<T>(1);
            FetchSub(decrement);
            return Load();
        }
        constexpr T operator--(int) PM_NOEXCEPT
        {
            T decrement = static_cast<T>(1);
            return FetchSub(decrement);
        }

        constexpr T operator+=(T rhs) PM_NOEXCEPT { return FetchAdd(rhs); }
        constexpr T operator+=(T rhs) volatile PM_NOEXCEPT
        {
            return FetchAdd(rhs);
        }
        constexpr T operator-=(T rhs) PM_NOEXCEPT { return FetchSub(rhs); }
        constexpr T operator-=(T rhs) volatile PM_NOEXCEPT
        {
            return FetchSub(rhs);
        }

        constexpr T operator&=(T rhs) PM_NOEXCEPT

            requires(IsIntegral<T>)
        {
            return FetchAnd(rhs);
        }
        constexpr T operator&=(T rhs) volatile PM_NOEXCEPT
            requires(IsIntegral<T>)
        {
            return FetchAnd(rhs);
        }
        constexpr T operator|=(T rhs) PM_NOEXCEPT
            requires(IsIntegral<T>)
        {
            return FetchOr(rhs);
        }
        constexpr T operator|=(T rhs) volatile PM_NOEXCEPT
            requires(IsIntegral<T>)
        {
            return FetchOr(rhs);
        }
        constexpr T operator^=(T rhs) PM_NOEXCEPT
            requires(IsIntegral<T>)
        {
            return FetchXor(rhs);
        }
        constexpr T operator^=(T rhs) volatile PM_NOEXCEPT
            requires(IsIntegral<T>)
        {
            return FetchXor(rhs);
        }

      private:
        T m_Value{};
    };

    using AtomicBool         = Atomic<bool>;
    using AtomicChar         = Atomic<char>;
    using AtomicSignedChar   = Atomic<signed char>;
    using AtomicUnsignedChar = Atomic<unsigned char>;

    using AtomicI8           = Atomic<i8>;
    using AtomicI16          = Atomic<i16>;
    using AtomicI32          = Atomic<i32>;
    using AtomicI64          = Atomic<i64>;

    using AtomicU8           = Atomic<u8>;
    using AtomicU16          = Atomic<u16>;
    using AtomicU32          = Atomic<u32>;
    using AtomicU64          = Atomic<u64>;

    using AtomicPointer      = Atomic<Pointer>;
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::Atomic;

using Prism::AtomicBool;
using Prism::AtomicChar;
using Prism::AtomicSignedChar;
using Prism::AtomicUnsignedChar;

using Prism::AtomicI16;
using Prism::AtomicI32;
using Prism::AtomicI64;
using Prism::AtomicI8;

using Prism::AtomicU16;
using Prism::AtomicU32;
using Prism::AtomicU64;
using Prism::AtomicU8;

using Prism::AtomicPointer;
#endif
