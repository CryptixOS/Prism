/*
 * Created by v1tr10l7 on 21.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Concepts.hpp>

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

#include <Prism/Utility/AtomicBuiltins.hpp>

namespace Prism
{
    template <typename T, typename = void>
    struct AtomicArithmetic
    {
        static T Add(T lhs, T rhs) { return lhs + rhs; }
        static T Sub(T lhs, T rhs) { return lhs - rhs; }
    };

    template <typename E>
    struct AtomicArithmetic<E, EnableIfType<IsEnumV<E>>>
    {
        using U = UnderlyingTypeType<E>;
        static auto Add(E lhs, E rhs)
        {
            return static_cast<E>(static_cast<U>(lhs) + static_cast<U>(rhs));
        }
        static auto Add(E lhs, U rhs)
        {
            return static_cast<E>(static_cast<U>(lhs) + rhs);
        }
        static auto Sub(E lhs, E rhs)
        {
            return static_cast<E>(static_cast<U>(lhs) - static_cast<U>(rhs));
        }
        static auto Sub(E lhs, U rhs)
        {
            return static_cast<E>(static_cast<U>(lhs) - rhs);
        }
    };
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

        constexpr void Store(T desired, MemoryOrder order
                                        = MemoryOrder::eSeqCst) PM_NOEXCEPT
        {
            AtomicStore(m_Value, desired, order);
        }
        constexpr void Store(T           desired,
                             MemoryOrder order
                             = MemoryOrder::eSeqCst) volatile PM_NOEXCEPT
        {
            AtomicStore(m_Value, desired, order);
        }

        constexpr T Load(MemoryOrder order
                         = MemoryOrder::eSeqCst) const PM_NOEXCEPT
        {
            return AtomicLoad(m_Value, order);
        }
        constexpr T Load(MemoryOrder order = MemoryOrder::eSeqCst) const
            volatile PM_NOEXCEPT
        {
            return AtomicLoad(m_Value, order);
        }
        constexpr   operator T() const PM_NOEXCEPT { return Load(); }
        constexpr   operator T() const volatile PM_NOEXCEPT { return Load(); }

        constexpr T Exchange(T desired, MemoryOrder order
                                        = MemoryOrder::eSeqCst) PM_NOEXCEPT
        {
            return AtomicExchange(m_Value, desired, order);
        }
        constexpr T Exchange(T           desired,
                             MemoryOrder order
                             = MemoryOrder::eSeqCst) volatile PM_NOEXCEPT
        {
            return AtomicExchange(m_Value, desired, order);
        }
        constexpr bool CompareExchange(T& expected, T desired, bool weak,
                                       MemoryOrder successOrder,
                                       MemoryOrder failureOrder)
        {
            return AtomicCompareExchange(m_Value, expected, desired, weak,
                                         successOrder, failureOrder);
        }

        constexpr static bool IsAlwaysLockFree() PM_NOEXCEPT
        {
            T value;
            return AtomicAlwaysLockFree(sizeof(T), &value);
        }

        constexpr T FetchAdd(T rhs, MemoryOrder order
                                    = MemoryOrder::eSeqCst) PM_NOEXCEPT
        {
            return AtomicFetchAdd(m_Value, rhs, order);
        }
        constexpr T FetchAdd(T rhs, MemoryOrder order
                                    = MemoryOrder::eSeqCst) volatile PM_NOEXCEPT
        {
            return AtomicFetchAdd(m_Value, rhs, order);
        }
        constexpr T FetchSub(T rhs, MemoryOrder order
                                    = MemoryOrder::eSeqCst) PM_NOEXCEPT
        {
            return AtomicFetchSub(m_Value, rhs, order);
        }
        constexpr T FetchSub(T rhs, MemoryOrder order
                                    = MemoryOrder::eSeqCst) volatile PM_NOEXCEPT
        {
            return AtomicFetchSub(m_Value, rhs, order);
        }
        constexpr T FetchAnd(T rhs, MemoryOrder order
                                    = MemoryOrder::eSeqCst) PM_NOEXCEPT
            requires(IsIntegralV<T>)
        {
            return AtomicFetchAnd(m_Value, rhs, order);
        }
        constexpr T FetchAnd(T rhs, MemoryOrder order
                                    = MemoryOrder::eSeqCst) volatile PM_NOEXCEPT
            requires(IsIntegralV<T>)
        {
            return AtomicFetchAnd(m_Value, rhs, order);
        }
        constexpr T FetchOr(T rhs, MemoryOrder order
                                   = MemoryOrder::eSeqCst) PM_NOEXCEPT
            requires(IsIntegralV<T>)
        {
            return AtomicFetchOr(m_Value, rhs, order);
        }
        constexpr T FetchOr(T rhs, MemoryOrder order
                                   = MemoryOrder::eSeqCst) volatile PM_NOEXCEPT
            requires(IsIntegralV<T>)
        {
            return AtomicFetchOr(m_Value, rhs, order);
        }
        constexpr T FetchXor(T rhs, MemoryOrder order
                                    = MemoryOrder::eSeqCst) PM_NOEXCEPT
            requires(IsIntegralV<T>)
        {
            return AtomicFetchXor(m_Value, rhs, order);
        }
        constexpr T FetchXor(T rhs, MemoryOrder order
                                    = MemoryOrder::eSeqCst) volatile PM_NOEXCEPT
            requires(IsIntegralV<T>)
        {
            return AtomicFetchXor(m_Value, rhs, order);
        }

        constexpr T operator++() PM_NOEXCEPT
        {
            T old = FetchAdd(T(1));
            return AtomicArithmetic<T>::Add(old, 1);
        }
        constexpr T operator++() volatile PM_NOEXCEPT
        {
            T old = FetchAdd(T(1));
            return AtomicArithmetic<T>::Add(old, 1);
        }
        constexpr T operator++(int) PM_NOEXCEPT
        {
            T old = FetchAdd(T(1));
            return old;
        }
        constexpr T operator++(int) volatile PM_NOEXCEPT
        {
            T old = FetchAdd(T(1));
            return old;
        }
        constexpr T operator--() PM_NOEXCEPT
        {
            T old = FetchSub(T(1));
            return AtomicArithmetic<T>::Sub(old, 1);
        }
        constexpr T operator--() volatile PM_NOEXCEPT
        {
            T old = FetchSub(T(1));
            return AtomicArithmetic<T>::Sub(old, 1);
        }
        constexpr T operator--(int) PM_NOEXCEPT
        {
            T old = FetchSub(T(1));
            return old;
        }
        constexpr T operator--(int) volatile PM_NOEXCEPT
        {
            T old = FetchSub(T(1));
            return old;
        }

        constexpr T operator+=(T rhs) PM_NOEXCEPT
        {
            T old = FetchAdd(rhs);
            return AtomicArithmetic<T>::Add(old, rhs);
        }
        constexpr T operator+=(T rhs) volatile PM_NOEXCEPT
        {
            T old = FetchAdd(rhs);
            return AtomicArithmetic<T>::Add(old, rhs);
        }
        constexpr T operator-=(T rhs) PM_NOEXCEPT
        {
            T old = FetchSub(rhs);
            return AtomicArithmetic<T>::Sub(old, rhs);
        }
        constexpr T operator-=(T rhs) volatile PM_NOEXCEPT
        {
            T old = FetchSub(rhs);
            return AtomicArithmetic<T>::Sub(old, rhs);
        }

        constexpr T operator&=(T rhs) PM_NOEXCEPT

            requires(IsIntegralV<T>)
        {
            return FetchAnd(rhs) & rhs;
        }
        constexpr T operator&=(T rhs) volatile PM_NOEXCEPT
            requires(IsIntegralV<T>)
        {
            return FetchAnd(rhs) & rhs;
        }
        constexpr T operator|=(T rhs) PM_NOEXCEPT
            requires(IsIntegralV<T>)
        {
            return FetchOr(rhs) | rhs;
        }
        constexpr T operator|=(T rhs) volatile PM_NOEXCEPT
            requires(IsIntegralV<T>)
        {
            return FetchOr(rhs) | rhs;
        }
        constexpr T operator^=(T rhs) PM_NOEXCEPT
            requires(IsIntegralV<T>)
        {
            return FetchXor(rhs) ^ rhs;
        }
        constexpr T operator^=(T rhs) volatile PM_NOEXCEPT
            requires(IsIntegralV<T>)
        {
            return FetchXor(rhs) ^ rhs;
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

#if PRISM_TARGET_CRYPTIX != 0
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
