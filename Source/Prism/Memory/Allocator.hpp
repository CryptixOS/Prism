/*
 * Created by v1tr10l7 on 14.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Error.hpp>
#include <Prism/Memory/Pointer.hpp>

namespace Prism
{
    class AllocatorBase
    {
      public:
        AllocatorBase()                                             = default;
        virtual ~AllocatorBase()                                    = default;

        virtual bool    Initialize()                                = 0;
        virtual void    Shutdown()                                  = 0;

        virtual Pointer Allocate(usize bytes, usize alignment = 0)  = 0;
        virtual Pointer Callocate(usize bytes, usize alignment = 0) = 0;
        virtual Pointer Reallocate(Pointer memory, usize bytes, usize alignment = 0) = 0;
        virtual void    Free(Pointer address)                       = 0;

        virtual usize   TotalAllocated() const                      = 0;
        virtual usize   TotalFreed() const                          = 0;
        virtual usize   Used() const                                = 0;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::AllocatorBase;
#endif
