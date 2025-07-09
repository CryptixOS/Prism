/*
 * Created by v1tr10l7 on 10.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Concepts.hpp>

namespace Prism
{
    template <typename Definition, template <typename...> typename Alternate,
              typename... _Args>
    struct DetectedOr
    {
        using Type       = Definition;
        using IsDetected = FalseType;
    };

    template <typename Definition, template <typename...> typename Alternate,
              typename... Args>
        requires requires { typename Alternate<Args...>; }
    struct DetectedOr<Definition, Alternate, Args...>
    {
        using Type       = Alternate<Args...>;
        using IsDetected = TrueType;
    };

    template <typename Default, template <typename...> class Alternate,
              typename... _Args>
    using DetectedOrType =
        typename DetectedOr<Default, Alternate, _Args...>::Type;

    template <typename Allocator>
    struct AllocatorTraits
    {
        using AllocatorType = Allocator;
        using ValueType     = AllocatorType::ValueType;
        using Pointer       = ValueType*;
        using ConstPointer  = const ValueType*;
        using SizeType      = usize;

        static Pointer Allocate(AllocatorType& allocator, SizeType count)
        {
            return new ValueType[count];
        }
        static void Deallocate(AllocatorType& allocator, Pointer addr,
                               SizeType count)
        {
            delete[] addr;
        }
        template <typename T, typename... Args>
        static void Construct(AllocatorType& allocator, T* addr, Args&&... args)
        {
            new (&addr) T(Forward<Args>(args)...);
        }
        template <typename T>
        static void Destroy(AllocatorType& allocator, T* addr)
        {
            addr->~T();
        }
    };
}; // namespace Prism
