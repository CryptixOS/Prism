/*
 * Created by v1tr10l7 on 15.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Memory/Allocator.hpp>
#include <Prism/Memory/Memory.hpp>
#include <Prism/Utility/Math.hpp>

namespace Prism
{
    template <typename PageAllocPolicy, typename LockPolicy>
    class BumpAllocator : public AllocatorBase
    {
      public:
        constexpr static usize PAGE_SIZE    = 0x1000;
        constexpr static usize DEFAULT_SIZE = 64_mib;

        virtual bool           Initialize() override
        {
            usize pageCount = Math::DivRoundUp(DEFAULT_SIZE, PAGE_SIZE);
            m_Base          = PageAllocPolicy::AllocatePages(pageCount);
            m_Current       = m_Base;
            m_Size = m_InitialSize = DEFAULT_SIZE;

            return true;
        }
        virtual void Shutdown() override
        {
            usize pageCount = Math::DivRoundUp(m_InitialSize, PAGE_SIZE);
            PageAllocPolicy::FreePages(m_Base, pageCount);

            m_Base = m_Current = m_Size = m_InitialSize = 0;
        }

        virtual Pointer Allocate(usize bytes, usize alignment = 0) override
        {
            auto memory = m_Current;
            if (m_Size < bytes) return nullptr;

            m_Current += bytes;
            m_Size -= bytes;
            m_TotalAllocated += bytes;

            return memory;
        }
        virtual Pointer Callocate(usize bytes, usize alignment = 0) override
        {
            auto memory = Allocate(bytes, alignment);
            Memory::Fill(memory, 0, bytes);

            return memory;
        }
        virtual Pointer Reallocate(Pointer memory, usize size,
                                   usize alignment = 0) override
        {
        }

        virtual void  Free(Pointer memory) override {}

        virtual usize TotalAllocated() const override
        {
            return m_TotalAllocated;
        }
        virtual usize TotalFreed() const override { return 0; }
        virtual usize Used() const override
        {
            return TotalAllocated() - TotalFreed();
        }

      private:
        Pointer    m_Base        = nullptr;
        Pointer    m_Current     = nullptr;
        usize      m_InitialSize = 0;
        usize      m_Size        = 0;
        LockPolicy m_Lock;

        usize      m_TotalAllocated = 0;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::BumpAllocator;
#endif
