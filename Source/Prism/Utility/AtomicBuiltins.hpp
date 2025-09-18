/*
 * Created by v1tr10l7 on 24.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#pragma once

#include <Prism/Core/Concepts.hpp>

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

namespace Prism
{
    struct Pointer;
    enum class MemoryOrder : u32
    {
        eRelaxed = __ATOMIC_RELAXED,
        eConsume = __ATOMIC_CONSUME,
        eAcquire = __ATOMIC_ACQUIRE,
        eRelease = __ATOMIC_RELEASE,
        eAcqRel  = __ATOMIC_ACQ_REL,
        eSeqCst  = __ATOMIC_SEQ_CST,
    };

    template <typename T>
    struct AtomicUnderlying
    {
        using Type = T;
    };
    template <>
    struct AtomicUnderlying<bool>
    {
        using Type = u8;
    };
    template <typename E>
        requires IsEnumV<E>
    struct AtomicUnderlying<E>
    {
        using Type = UnderlyingTypeType<E>;
    };

    template <typename T>
    using AtomicUnderlyingType = typename AtomicUnderlying<T>::Type;

    // ------------------------ Normalization helpers ------------------------
    template <typename T>
    PM_ALWAYS_INLINE constexpr AtomicUnderlyingType<RemoveCvType<T>>*
    AtomicAddress(T* p) PM_NOEXCEPT
    {
        using Base = AtomicUnderlyingType<RemoveCvType<T>>;
        return reinterpret_cast<Base*>(p);
    }

    template <typename T>
    PM_ALWAYS_INLINE constexpr auto
    AtomicAddress(const T* p) PM_NOEXCEPT -> ConditionalType<
        IsVolatileV<T>, const volatile AtomicUnderlyingType<RemoveCvType<T>>*,
        const AtomicUnderlyingType<RemoveCvType<T>>*>
    {
        using Base        = AtomicUnderlyingType<RemoveCvType<T>>;
        using PointerType = ConditionalType<IsVolatileV<T>,
                                            const volatile Base*, const Base*>;
        return reinterpret_cast<PointerType>(p);
    }

    template <typename T>
    PM_ALWAYS_INLINE constexpr AtomicUnderlyingType<T>
    AtomicValue(T v) PM_NOEXCEPT
    {
        if constexpr (IsEnumV<T>)
            return static_cast<AtomicUnderlyingType<T>>(
                static_cast<UnderlyingTypeType<T>>(v));
        else if constexpr (IsSameV<T, bool>)
            return static_cast<AtomicUnderlyingType<T>>(v ? 1u : 0u);
        else return static_cast<AtomicUnderlyingType<T>>(v);
    }

    template <typename T>
    PM_ALWAYS_INLINE constexpr T
    AtomicCast(AtomicUnderlyingType<T> v) PM_NOEXCEPT
    {
        if constexpr (IsEnumV<T>) return static_cast<T>(v);
        else if constexpr (IsSameV<T, bool>) return static_cast<bool>(v != 0);
        else return static_cast<T>(v);
    }

    // ------------------------ Atomic wrappers ------------------------
    /**
     * @brief Performs an atomic load and returns the current value of the
     * atomic variable. Memory is affected according to the value of
     * memoryOrder
     * @param source - source of the operation
     * @param memoryOrder - memory order constraints to enforce, valid values
     * => [eRelaxed, eSeqCst, eRelease]
     *
     * @return The current value of the atomic variable.
     */
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr T AtomicLoad(const T&    source,
                                            MemoryOrder memoryOrder)
    {
        return AtomicCast<T>(
            __atomic_load_n(AtomicAddress(&source), ToUnderlying(memoryOrder)));
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE void AtomicLoad(const T& source, T& destination,
                                     MemoryOrder memoryOrder)
    {
        U tmp{};
        __atomic_load(AtomicAddress(&source), &tmp, ToUnderlying(memoryOrder));
        destination = AtomicCast<T>(tmp);
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr void AtomicStore(T& destination, T&& source,
                                                MemoryOrder memoryOrder)
    {
        __atomic_store_n(AtomicAddress(&destination), AtomicValue(source),
                         ToUnderlying(memoryOrder));
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr void
    AtomicStore(volatile T& destination, T& source, MemoryOrder memoryOrder)
    {
        __atomic_store(AtomicAddress(const_cast<T*>(&destination)),
                       AtomicAddress(&source), ToUnderlying(memoryOrder));
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr T AtomicExchange(volatile T& target, T newValue,
                                                MemoryOrder memoryOrder)
    {
        return AtomicCast<T>(__atomic_exchange_n(
            AtomicAddress(const_cast<T*>(&target)), AtomicValue(newValue),
            ToUnderlying(memoryOrder)));
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr void
    AtomicExchange(T& target, T& newValue, T& oldValue, MemoryOrder memoryOrder)
    {
        __atomic_exchange(AtomicAddress(&target), AtomicAddress(&newValue),
                          AtomicAddress(&oldValue), ToUnderlying(memoryOrder));
    }
    /*
     * @brief Compares the contents of <*target> with the contents of
     *  [*expected]. If equal, the operation is a read-modify-write operation
     *  that writes desired into [*target]. If they are not equal, the operation
     *  is a read and the current contents of <*target> are written into
     *  [*expected]. weak  is true for weak compare_exchange, which may fail
     *  spuriously, and false for the strong variation, which never fails
     *  spuriously. Many targets only offer the strong variation and ignore the
     *  parameter. When in doubt, use the strong variation.
     *
     *  if [desired] is written int [*target] then true is return and memory is
     *  affected according to the memory order specified by
     *[successMemoryOrder]. The are no restriction on what memory order can be
     *used here.
     *
     *  Otherwise, false is returned and memoy is affected to
     *  failureMemoryOrder. This memory order cannot be eRelease nor
     *  eAcqRel. It also cannot be a stronger order than that specified by
     *  [successMemoryOrder]
     *
     *
     **/
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr bool
    AtomicCompareExchange(T& target, T& expected, T&& desired, bool weak,
                          MemoryOrder successMemoryOrder,
                          MemoryOrder failureMemoryOrder)
    {
        return __atomic_compare_exchange_n(
            AtomicAddress(&target), AtomicAddress(&expected),
            AtomicValue(desired), weak, ToUnderlying(successMemoryOrder),
            ToUnderlying(failureMemoryOrder));
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr bool
    AtomicCompareExchange(T& target, T& expected, T& desired, bool weak,
                          MemoryOrder successMemoryOrder,
                          MemoryOrder failureMemoryOrder)
    {
        return __atomic_compare_exchange(
            AtomicAddress(&target), AtomicAddress(&expected),
            AtomicAddress(&desired), weak, ToUnderlying(successMemoryOrder),
            ToUnderlying(failureMemoryOrder));
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr T AtomicAddFetch(T&                     target,
                                                RemoveReferenceType<T> value,
                                                MemoryOrder memoryOrder)
    {
        return AtomicCast<T>(__atomic_add_fetch(AtomicAddress(&target),
                                                AtomicValue(value),
                                                ToUnderlying(memoryOrder)));
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr T AtomicSubFetch(T&                     target,
                                                RemoveReferenceType<T> value,
                                                MemoryOrder memoryOrder)
    {
        U tmp = __atomic_sub_fetch(AtomicAddress(&target), AtomicValue(value),
                                   ToUnderlying(memoryOrder));
        return AtomicCast<T>(tmp);
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr T AtomicAndFetch(T&                     target,
                                                RemoveReferenceType<T> value,
                                                MemoryOrder memoryOrder)
    {
        U tmp = __atomic_and_fetch(AtomicAddress(&target), AtomicValue(value),
                                   ToUnderlying(memoryOrder));
        return AtomicCast<T>(tmp);
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr T AtomicXorFetch(T&                     target,
                                                RemoveReferenceType<T> value,
                                                MemoryOrder memoryOrder)
    {
        U tmp = __atomic_xor_fetch(AtomicAddress(&target), AtomicValue(value),
                                   ToUnderlying(memoryOrder));
        return AtomicCast<T>(tmp);
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr T AtomicOrFetch(T&                     target,
                                               RemoveReferenceType<T> value,
                                               MemoryOrder memoryOrder)
    {
        U tmp = __atomic_or_fetch(AtomicAddress(&target), AtomicValue(value),
                                  ToUnderlying(memoryOrder));
        return AtomicCast<T>(tmp);
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr T AtomicNandFetch(T&                     target,
                                                 RemoveReferenceType<T> value,
                                                 MemoryOrder memoryOrder)
    {
        U tmp = __atomic_nand_fetch(AtomicAddress(&target), AtomicValue(value),
                                    ToUnderlying(memoryOrder));
        return AtomicCast<T>(tmp);
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr T AtomicFetchAdd(T&                     target,
                                                RemoveReferenceType<T> value,
                                                MemoryOrder memoryOrder)
    {
        U tmp = __atomic_fetch_add(AtomicAddress(&target), AtomicValue(value),
                                   ToUnderlying(memoryOrder));
        return AtomicCast<T>(tmp);
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr T AtomicFetchSub(T&                     target,
                                                RemoveReferenceType<T> value,
                                                MemoryOrder memoryOrder)
    {
        U tmp = __atomic_fetch_sub(AtomicAddress(&target), AtomicValue(value),
                                   ToUnderlying(memoryOrder));
        return AtomicCast<T>(tmp);
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr T AtomicFetchAnd(T&                     target,
                                                RemoveReferenceType<T> value,
                                                MemoryOrder memoryOrder)
    {
        U tmp = __atomic_fetch_and(AtomicAddress(&target), AtomicValue(value),
                                   ToUnderlying(memoryOrder));
        return AtomicCast<T>(tmp);
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr T AtomicFetchXor(T&                     target,
                                                RemoveReferenceType<T> value,
                                                MemoryOrder memoryOrder)
    {
        U tmp = __atomic_fetch_xor(AtomicAddress(&target), AtomicValue(value),
                                   ToUnderlying(memoryOrder));
        return AtomicCast<T>(tmp);
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr T AtomicFetchOr(T&                     target,
                                               RemoveReferenceType<T> value,
                                               MemoryOrder memoryOrder)
    {
        U tmp = __atomic_fetch_or(AtomicAddress(&target), AtomicValue(value),
                                  ToUnderlying(memoryOrder));
        return AtomicCast<T>(tmp);
    }
    template <typename T, typename U = AtomicUnderlyingType<T>>
    PM_ALWAYS_INLINE constexpr T AtomicFetchNand(T&                     target,
                                                 RemoveReferenceType<T> value,
                                                 MemoryOrder memoryOrder)
    {
        U tmp = __atomic_fetch_nand(AtomicAddress(&target), AtomicValue(value),
                                    ToUnderlying(memoryOrder));
        return AtomicCast<T>(value);
    }

    template <Integral T>
        requires(sizeof(T) == 1)
    PM_ALWAYS_INLINE bool AtomicTestAndSet(T& target, MemoryOrder memoryOrder)
    {
        return __atomic_test_and_set(AtomicAddress(&target),
                                     ToUnderlying(memoryOrder));
    }
    template <Integral T>
        requires(sizeof(T) == 1)
    PM_ALWAYS_INLINE void AtomicClear(T& target, MemoryOrder memoryOrder)
    {
        __atomic_clear(AtomicAddress(&target), ToUnderlying(memoryOrder));
    }
    PM_ALWAYS_INLINE void AtomicThreadFence(MemoryOrder memoryOrder)
    {
        __atomic_thread_fence(ToUnderlying(memoryOrder));
    }
    PM_ALWAYS_INLINE void AtomicSignalFence(MemoryOrder memoryOrder)
    {
        __atomic_signal_fence(ToUnderlying(memoryOrder));
    }

    template <NullableType T = void*>
    PM_ALWAYS_INLINE constexpr bool AtomicAlwaysLockFree(usize size,
                                                         T     object = nullptr)
    {
        return __atomic_always_lock_free(size, object);
    }
    template <NullableType T = void*>
    PM_ALWAYS_INLINE constexpr bool AtomicIsLockFree(usize size,
                                                     T     object = nullptr)
    {
        return __atomic_is_lock_free(size, object);
    }
    PM_ALWAYS_INLINE constexpr void AtomicMemoryBarrier() PM_NOEXCEPT
    {
        AtomicSignalFence(MemoryOrder::eAcqRel);
        AtomicThreadFence(MemoryOrder::eAcqRel);
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::MemoryOrder;

using Prism::AtomicLoad;
using Prism::AtomicStore;

using Prism::AtomicCompareExchange;
using Prism::AtomicExchange;

using Prism::AtomicAddFetch;
using Prism::AtomicAndFetch;
using Prism::AtomicNandFetch;
using Prism::AtomicOrFetch;
using Prism::AtomicSubFetch;
using Prism::AtomicXorFetch;

using Prism::AtomicFetchAdd;
using Prism::AtomicFetchAnd;
using Prism::AtomicFetchNand;
using Prism::AtomicFetchOr;
using Prism::AtomicFetchSub;
using Prism::AtomicFetchXor;

using Prism::AtomicAlwaysLockFree;
using Prism::AtomicClear;
using Prism::AtomicMemoryBarrier;
using Prism::AtomicSignalFence;
using Prism::AtomicTestAndSet;
using Prism::AtomicThreadFence;
#endif
