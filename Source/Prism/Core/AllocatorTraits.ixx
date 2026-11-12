/*
 * Created by v1tr10l7 on 10.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>
export module Prism.Core.AllocatorTraits;

export import Prism.Core.Concepts;

export extern "C++"
{
    namespace Prism
    {
        template <typename Allocator>
        struct AllocatorTraits
        {
            using AllocatorType = Allocator;
            using ValueType     = AllocatorType::ValueType;
            using Pointer       = ValueType*;
            using ConstPointer  = const ValueType*;
            using SizeType      = usize;

            static Pointer Allocate([[maybe_unused]] AllocatorType& allocator, SizeType count)
            {
                return new ValueType[count];
            }
            static void Deallocate([[maybe_unused]] AllocatorType& allocator, Pointer addr,
                                   [[maybe_unused]] SizeType count)
            {
                delete[] addr;
            }
            template <typename T, typename... Args>
            static void Construct([[maybe_unused]] AllocatorType& allocator, T* addr, Args&&... args)
            {
                new (&addr) T(Forward<Args>(args)...);
            }
            template <typename T>
            static void Destroy([[maybe_unused]] AllocatorType& allocator, T* addr)
            {
                addr->~T();
            }
        };
    }; // namespace Prism
}
