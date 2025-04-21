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
    };

    /**
     * @brief Performs an atomic load and returns the current value of the
     * atomic variable. Memory is affected according to the value of
     * memoryOrder
     * @param source - source of the operation
     * @param memoryOrder - memory order constraints to enforce, valid values
     * => [eAtomicRelaxed, eAtomicSeqCst, eAtomicRelease]
     *
     * @return The current value of the atomic variable.
     */
    template <typename T>
    PM_ALWAYS_INLINE constexpr ArithmeticEnum<T>
    AtomicLoad(T& source, ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_load_n(reinterpret_cast<volatile u32*>(&source),
                               memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr void
    AtomicLoad(T& source, T& destination,
               ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        __atomic_load(reinterpret_cast<volatile u32*>(&source),
                      reinterpret_cast<u32*>(&destination), memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr void
    AtomicStore(T& destination, ArithmeticEnum<T> value,
                ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        __atomic_store_n(reinterpret_cast<volatile u32*>(&destination), value,
                         memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr void
    AtomicStore(T& destination, T& source,
                ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        __atomic_store(reinterpret_cast<volatile u32*>(&destination),
                       reinterpret_cast<volatile u32*>(&source), memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr ArithmeticEnum<T>
    AtomicExchange(T* target, ArithmeticEnum<T> newValue,
                   ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_exchange_n(reinterpret_cast<volatile u32*>(target),
                                   newValue, memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr void
    AtomicExchange(T* target, ArithmeticEnum<T> newValue, T& oldValue,
                   ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        __atomic_exchange(reinterpret_cast<volatile u32*>(target), newValue,
                          &oldValue, memoryOrder);
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
     *  failureMemoryOrder. This memory order cannot be eAtomicRelease nor
     *  eAtomicAcqRel. It also cannot be a stronger order than that specified by
     *  [successMemoryOrder]
     *
     *
     **/
    template <typename T>
    PM_ALWAYS_INLINE constexpr bool
    AtomicCompareExchange(T* target, T* expected, ArithmeticEnum<T> desired,
                          bool                         weak,
                          ArithmeticEnum<MemoryOrder> successMemoryOrder,
                          ArithmeticEnum<MemoryOrder> failureMemoryOrder)
    {
        return __atomic_compare_exchange_n(
            reinterpret_cast<volatile u32*>(target),
            reinterpret_cast<u32*>(expected), desired, weak, successMemoryOrder,
            failureMemoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr bool
    AtomicCompareExchange(T* target, T* expected, T* desired, bool weak,
                          ArithmeticEnum<MemoryOrder> successMemoryOrder,
                          ArithmeticEnum<MemoryOrder> failureMemoryOrder)
    {
        return __atomic_compare_exchange(
            reinterpret_cast<volatile u32*>(target), expected,
            reinterpret_cast<u32*>(desired), weak, successMemoryOrder,
            failureMemoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr ArithmeticEnum<T>
    AtomicAddFetch(T* target, ArithmeticEnum<T> value,
                   ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_add_fetch(reinterpret_cast<volatile u32*>(target),
                                  value, memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr ArithmeticEnum<T>
    AtomicSubFetch(T* target, ArithmeticEnum<T> value,
                   ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_sub_fetch(reinterpret_cast<volatile u32*>(target),
                                  value, memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr ArithmeticEnum<T>
    AtomicAndFetch(T* target, ArithmeticEnum<T> value,
                   ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_and_fetch(reinterpret_cast<volatile u32*>(target),
                                  value, memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr ArithmeticEnum<T>
    AtomicXorFetch(T* target, ArithmeticEnum<T> value,
                   ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_xor_fetch(reinterpret_cast<volatile u32*>(target),
                                  value, memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr ArithmeticEnum<T>
    AtomicOrFetch(T* target, ArithmeticEnum<T> value,
                  ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_or_fetch(reinterpret_cast<volatile u32*>(target), value,
                                 memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr ArithmeticEnum<T>
    AtomicNandFetch(T* target, ArithmeticEnum<T> value,
                    ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_nand_fetch(reinterpret_cast<volatile u32*>(target),
                                   value, memoryOrder);
    }

    template <typename T>
    PM_ALWAYS_INLINE constexpr ArithmeticEnum<T>
    AtomicFetchAdd(T* target, ArithmeticEnum<T> value,
                   ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_fetch_add(reinterpret_cast<volatile u32*>(target),
                                  value, memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr ArithmeticEnum<T>
    AtomicFetchSub(T* target, ArithmeticEnum<T> value,
                   ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_fetch_sub(reinterpret_cast<volatile u32*>(target),
                                  value, memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr ArithmeticEnum<T>
    AtomicFetchAnd(T* target, ArithmeticEnum<T> value,
                   ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_fetch_and(reinterpret_cast<volatile u32*>(target),
                                  value, memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr ArithmeticEnum<T>
    AtomicFetchXor(T* target, ArithmeticEnum<T> value,
                   ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_fetch_xor(reinterpret_cast<volatile u32*>(target),
                                  value, memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr ArithmeticEnum<T>
    AtomicFetchOr(T* target, ArithmeticEnum<T> value,
                  ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_fetch_or(reinterpret_cast<volatile u32*>(target), value,
                                 memoryOrder);
    }
    template <typename T>
    PM_ALWAYS_INLINE constexpr ArithmeticEnum<T>
    AtomicFetchNand(T* target, ArithmeticEnum<T> value,
                    ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_fetch_nand(reinterpret_cast<volatile u32*>(target),
                                   value, memoryOrder);
    }

    PM_ALWAYS_INLINE inline bool
    AtomicTestAndSet(void* target, ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_test_and_set(reinterpret_cast<volatile u32*>(target),
                                     memoryOrder);
    }
    PM_ALWAYS_INLINE inline void
    AtomicClear(bool* target, ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_clear(reinterpret_cast<volatile u32*>(target),
                              memoryOrder);
    }
#if PRISM_TARGET_CRYPTIX != 1
    PM_ALWAYS_INLINE constexpr void
    AtomicThreadFence(ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_thread_fence((memoryOrder);
    }
    __ATOMIC_HLE_ACQUIRE
    PM_ALWAYS_INLINE constexpr void
    AtomicSignalFence(ArithmeticEnum<MemoryOrder> memoryOrder)
    {
        return __atomic_signal_fence((memoryOrder);
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
