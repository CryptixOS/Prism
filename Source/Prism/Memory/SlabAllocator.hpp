/*
 * Created by v1tr10l7 on 17.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Log.hpp>
#include <Prism/Utility/Math.hpp>

#ifndef ToDoWarn
#define ToDoWarn(...) PrismWarn("{}[{}:{}]: {} is not implemented", PM_FILENAME, PM_LINE, PM_COLUMN, PM_FUNCTION_NAME)
#endif

namespace Prism
{
    struct SlabFrame
    {
        SlabFrame* Next = nullptr;
    };

    constexpr u64   SLAB_CANARY = 0xDEADBEEFCAFEBABE;
    constexpr u64   FREE_POISON = 0xDEADDEADDEADDEAD;
    constexpr usize PAGE_SIZE   = 0x1000;

    class SlabAllocatorBase
    {
      public:
        virtual usize AllocationSize()     = 0;
        virtual void  Free(Pointer memory) = 0;
    };
    struct SlabHeader
    {
        SlabAllocatorBase* Slab = nullptr;
    };

    template <typename PageAllocPolicy, typename LockPolicy>
    class SlabAllocator : public SlabAllocatorBase
    {
      public:
        SlabAllocator() {}

        virtual ErrorOr<void> Initialize(usize chunkSize)
        {
            assert(Math::IsPowerOfTwo(chunkSize));
            m_ChunkSize     = chunkSize;

            usize pageCount = Math::DivRoundUp(chunkSize, PAGE_SIZE);
            m_FirstFree     = PageAllocPolicy::CallocatePages(pageCount);
            if (!m_FirstFree) return Error(ENOMEM);

            auto firstSlab  = m_FirstFree.As<SlabHeader>();
            firstSlab->Slab = this;

            usize usableSize
                = PAGE_SIZE - Math::AlignUp(sizeof(SlabFrame), m_ChunkSize);
            PM_UNUSED usize   usableCount = usableSize / m_ChunkSize;

            PM_UNUSED Pointer frameBase   = m_FirstFree.Offset(
                Math::AlignUp(sizeof(SlabFrame), m_ChunkSize));
            PM_UNUSED SlabFrame* previous = nullptr;

            for (usize i = 0; i < usableCount; i++)
            {
                auto frame         = frameBase.Offset<SlabFrame*>(i *
                m_ChunkSize);
                // frame->Header.Slab = this;
                frame->Next        = previous;
                previous           = frame;
            }
            m_FirstFree = previous;
            return {};

            auto                   available
                = PAGE_SIZE - Math::AlignUp(sizeof(SlabHeader), m_ChunkSize);

            PM_UNUSED auto guard = m_Lock.Lock();
            m_FirstFree += Math::AlignUp(sizeof(SlabHeader), m_ChunkSize);

            auto       count = available / m_ChunkSize;
            SlabFrame* prev  = nullptr;
            for (usize i = 0; i < count; i++)
            {
                auto frame  = m_FirstFree.Offset<SlabFrame*>(i * m_ChunkSize);
                frame->Next = prev;
                prev        = frame;
            }
            m_FirstFree = prev;
            return {};
        }
        virtual void    Shutdown() { ToDoWarn(); }

        virtual Pointer Allocate()
        {
            if (!m_FirstFree && !Initialize(m_ChunkSize)) return nullptr;

            PM_UNUSED auto guard  = m_Lock.Lock();
            auto frame  = m_FirstFree.As<SlabFrame>();
            m_FirstFree = frame->Next;

            m_TotalAllocated += m_ChunkSize;
            return frame;
        }
        virtual void Free(Pointer memory) override
        {
            if (!memory) return;

            PM_UNUSED auto guard  = m_Lock.Lock();
            auto frame  = memory.As<SlabFrame>();
            frame->Next = m_FirstFree;
            m_FirstFree = frame;

            m_TotalFreed += m_ChunkSize;
            return;
        }

        virtual usize AllocationSize() override { return m_ChunkSize; }

        virtual usize TotalAllocated() const { return m_TotalAllocated; }
        virtual usize TotalFreed() const { return m_TotalFreed; }
        virtual usize Used() const { return TotalAllocated() - TotalFreed(); }

      private:
        LockPolicy m_Lock;
        Pointer    m_FirstFree = 0;
        usize      m_ChunkSize = 0;

        usize      m_TotalAllocated = 0;
        usize      m_TotalFreed = 0;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::SlabAllocator;
using Prism::SlabAllocatorBase;
using Prism::SlabFrame;
using Prism::SlabHeader;
#endif
