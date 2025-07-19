/*
 * Created by v1tr10l7 on 18.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Array.hpp>

namespace Prism
{
    constexpr usize RADIX_LEAF_COUNT = Bit(6);
    template <typename Derived, typename IndexType, typename ValueType>
    struct IntrusiveRadixNode
    {
        Derived*   Children[RADIX_LEAF_COUNT];
        bool       IsLeaf = false;
        ValueType* Value  = nullptr;

        Derived*   Self() PM_NOEXCEPT { return static_cast<Derived*>(this); }
    };

    template <typename Key, typename Value>
    class RadixTree
    {
      public:
        constexpr static usize MAP_SIZE = Bit(6);
    };
}; // namespace Prism
