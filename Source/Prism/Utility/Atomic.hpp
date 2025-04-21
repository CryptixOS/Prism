/*
 * Created by v1tr10l7 on 21.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

namespace Prism
{
    enum class MemoryOrder : u32
    {
        eAtomicRelaxed = __ATOMIC_RELAXED,
        eAtomicConsume = __ATOMIC_CONSUME,
        eAtomicAcquire = __ATOMIC_ACQUIRE,
        eAtomicRelease = __ATOMIC_RELEASE,
        eAtomicAcqRel  = __ATOMIC_ACQ_REL,
        eAtomicSeqCst  = __ATOMIC_SEQ_CST,
        // eAtomicLockEvisionStart = __ATOMIC_HLE_ACQUIRE,
        // eAtomicLockEvisionEnd   = __ATOMIC_HLE_RELEASE,
    };

    template <typename T>
    PM_ALWAYS_INLINE constexpr T AtomicLoad(T& source, MemoryOrder memoryOrder)
    {
        return static_cast<T>(
            __atomic_load_n(reinterpret_cast<volatile u32*>(&source),
                            std::to_underlying(memoryOrder)));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr void AtomicLoad(T& source, T& destination,
                                               MemoryOrder memoryOrder)
    {
        __atomic_load(reinterpret_cast<volatile u32*>(&source),
                      reinterpret_cast<u32*>(&destination),
                      std::to_underlying(memoryOrder));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr void AtomicStore(T& destination, T value,
                                                MemoryOrder memoryOrder)
    {
        __atomic_store_n(reinterpret_cast<volatile u32*>(&destination),
                         std::to_underlying(value),
                         std::to_underlying(memoryOrder));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr void AtomicStore(T& destination, T& source,
                                                MemoryOrder memoryOrder)
    {
        __atomic_store(reinterpret_cast<volatile u32*>(&destination),
                       reinterpret_cast<volatile u32*>(&source),
                       std::to_underlying(memoryOrder));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr T AtomicExchange(T* target, T newValue,
                                                MemoryOrder memoryOrder)
    {
        return static_cast<T>(
            __atomic_exchange_n(reinterpret_cast<volatile u32*>(target),
                                newValue, std::to_underlying(memoryOrder)));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr void
    AtomicExchange(T* target, T newValue, T& oldValue, MemoryOrder memoryOrder)
    {
        __atomic_exchange(reinterpret_cast<volatile u32*>(target), newValue,
                          &oldValue, std::to_underlying(memoryOrder));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr bool
    AtomicCompareExchange(T* target, T* expected, T desired, bool weak,
                          MemoryOrder successMemoryOrder,
                          MemoryOrder failureMemoryOrder)
    {
        return __atomic_compare_exchange_n(
            reinterpret_cast<volatile u32*>(target),
            reinterpret_cast<u32*>(expected), std::to_underlying(desired), weak,
            std::to_underlying(successMemoryOrder),
            std::to_underlying(failureMemoryOrder));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr bool
    AtomicCompareExchange(T* target, T* expected, T* desired, bool weak,
                          MemoryOrder successMemoryOrder,
                          MemoryOrder failureMemoryOrder)
    {
        return __atomic_compare_exchange(
            reinterpret_cast<volatile u32*>(target), expected, desired, weak,
            std::to_underlying(successMemoryOrder),
            std::to_underlying(failureMemoryOrder));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr T AtomicAddFetch(T* target, T value,
                                                MemoryOrder memoryOrder)
    {
        return static_cast<T>(
            __atomic_add_fetch(reinterpret_cast<volatile u32*>(target), value,
                               std::to_underlying(memoryOrder)));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr T AtomicSubFetch(T* target, T value,
                                                MemoryOrder memoryOrder)
    {
        return static_cast<T>(
            __atomic_sub_fetch(reinterpret_cast<volatile u32*>(target), value,
                               std::to_underlying(memoryOrder)));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr T AtomicAndFetch(T* target, T value,
                                                MemoryOrder memoryOrder)
    {
        return static_cast<T>(
            __atomic_and_fetch(reinterpret_cast<volatile u32*>(target), value,
                               std::to_underlying(memoryOrder)));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr T AtomicXorFetch(T* target, T value,
                                                MemoryOrder memoryOrder)
    {
        return static_cast<T>(
            __atomic_xor_fetch(reinterpret_cast<volatile u32*>(target), value,
                               std::to_underlying(memoryOrder)));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr T AtomicOrFetch(T* target, T value,
                                               MemoryOrder memoryOrder)
    {
        return static_cast<T>(
            __atomic_or_fetch(reinterpret_cast<volatile u32*>(target), value,
                              std::to_underlying(memoryOrder)));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr T AtomicNandFetch(T* target, T value,
                                                 MemoryOrder memoryOrder)
    {
        return static_cast<T>(
            __atomic_nand_fetch(reinterpret_cast<volatile u32*>(target), value,
                                std::to_underlying(memoryOrder)));
    }

    template <typename T>
    PM_ALWAYS_INLINE constexpr T AtomicFetchAdd(T* target, T value,
                                                MemoryOrder memoryOrder)
    {
        return static_cast<T>(
            __atomic_fetch_add(reinterpret_cast<volatile u32*>(target), value,
                               std::to_underlying(memoryOrder)));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr T AtomicFetchSub(T* target, T value,
                                                MemoryOrder memoryOrder)
    {
        return static_cast<T>(
            __atomic_fetch_sub(reinterpret_cast<volatile u32*>(target), value,
                               std::to_underlying(memoryOrder)));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr T AtomicFetchAnd(T* target, T value,
                                                MemoryOrder memoryOrder)
    {
        return static_cast<T>(
            __atomic_fetch_and(reinterpret_cast<volatile u32*>(target), value,
                               std::to_underlying(memoryOrder)));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr T AtomicFetchXor(T* target, T value,
                                                MemoryOrder memoryOrder)
    {
        return static_cast<T>(
            __atomic_fetch_xor(reinterpret_cast<volatile u32*>(target), value,
                               std::to_underlying(memoryOrder)));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr T AtomicFetchOr(T* target, T value,
                                               MemoryOrder memoryOrder)
    {
        return static_cast<T>(
            __atomic_fetch_or(reinterpret_cast<volatile u32*>(target), value,
                              std::to_underlying(memoryOrder)));
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr T AtomicFetchNand(T* target, T value,
                                                 MemoryOrder memoryOrder)
    {
        return static_cast<T>(
            __atomic_fetch_nand(reinterpret_cast<volatile u32*>(target), value,
                                std::to_underlying(memoryOrder)));
    }

    PM_ALWAYS_INLINE inline bool AtomicTestAndSet(void*       target,
                                                  MemoryOrder memoryOrder)
    {
        return __atomic_test_and_set(reinterpret_cast<volatile u32*>(target),
                                     std::to_underlying(memoryOrder));
    }
    PM_ALWAYS_INLINE inline void AtomicClear(bool*       target,
                                             MemoryOrder memoryOrder)
    {
        return __atomic_clear(reinterpret_cast<volatile u32*>(target),
                              std::to_underlying(memoryOrder));
    }
#if PRISM_TARGET_CRYPTIX != 1
    PM_ALWAYS_INLINE constexpr void AtomicThreadFence(MemoryOrder memoryOrder)
    {
        return __atomic_thread_fence(std::to_underlying(memoryOrder));
    }
    __ATOMIC_HLE_ACQUIRE
    PM_ALWAYS_INLINE constexpr void AtomicSignalFence(MemoryOrder memoryOrder)
    {
        return __atomic_signal_fence(std::to_underlying(memoryOrder));
    }
#endif
    PM_ALWAYS_INLINE constexpr bool AtomicAlwaysLockFree(usize size,
                                                         void* object)
    {
        return __atomic_always_lock_free(size, object);
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
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
using Prism::AtomicTestAndSet;
#endif
