/*
 * Created by v1tr10l7 on 09.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/Utility/Optional.hpp>

namespace Prism
{
    class RangeAllocator
    {
        RangeAllocator(u64 start, u64 end, u64 granularity);
        Optional<u64> Allocate(u64 count);
        Optional<u64> AllocateAt(u64 start, u64 count);

        void          Free(u64 start, u64 count = 1);
        void          Reserve(u64 start, u64 count);

      private:
        u64 start = 0;
        u64 end   = 0;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::RangeAllocator
#endif
