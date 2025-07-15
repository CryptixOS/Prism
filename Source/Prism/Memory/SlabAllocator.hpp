/*
 * Created by v1tr10l7 on 17.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Error.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Log.hpp>
#include <Prism/Utility/Math.hpp>

#ifndef ToDoWarn
    #define ToDoWarn(...)                                                      \
        PrismWarn("{}[{}:{}]: {} is not implemented", PM_FILENAME, PM_LINE,    \
                  PM_COLUMN, PM_FUNCTION_NAME)
#endif

namespace Prism
{
    struct SlabObject
    {
        SlabObject* Next = nullptr;
    };

    constexpr u64   SLAB_CANARY = 0xDEADBEEFCAFEBABE;
    constexpr u64   FREE_POISON = 0xDEADDEADDEADDEAD;
    constexpr usize PAGE_SIZE   = 0x1000;

    //TODO(v1tr10l7): Memory poisoning
    //TODO(v1tr10l7): canaries
    //TODO(v1tr10l7): frame tracking
    //TODO(v1tr10l7): alignment, shutdown
    class SlabAllocatorBase
    {
      public:
        virtual usize AllocationSize()     = 0;
        virtual void  Free(Pointer memory) = 0;
    };
    struct SlabFrame
    {
        Pointer            Base                 = nullptr;
        usize              TotalLength          = 0;
        Pointer            Data                 = nullptr;
        usize              DataLength           = 0;

        usize              AllocatedObjectCount = 0;
        SlabObject*        NextFree             = nullptr;
        SlabAllocatorBase* Allocator            = nullptr;
    };

    template <typename PageAllocPolicy, typename LockPolicy>
    class SlabAllocator : public SlabAllocatorBase
    {
      public:
        SlabAllocator() {}

        virtual ErrorOr<void> Initialize(usize chunkSize)
        {
            assert(Math::IsPowerOfTwo(chunkSize));
            m_ObjectSize                 = chunkSize;

            constexpr usize SLAB_SIZE    = 0x1000;
            constexpr usize SLAB_SB_SIZE = 0x1000;

            usize           totalSize    = SLAB_SIZE + SLAB_SB_SIZE;
            usize           pageCount = Math::DivRoundUp(totalSize, PAGE_SIZE);

            Pointer         base = PageAllocPolicy::CallocatePages(pageCount);
            if (!base) return Error(ENOMEM);

            auto  address  = Math::AlignUp(base, SLAB_SB_SIZE);
            usize overhead = 0;
            for (; overhead < sizeof(SlabFrame); overhead += m_ObjectSize);

            auto frame         = new (base.As<void>()) SlabFrame;
            frame->Data        = address + overhead;
            frame->DataLength  = SLAB_SIZE - overhead;
            frame->Base        = base;
            frame->TotalLength = totalSize;
            m_Frame            = frame;
            m_Frame->NextFree  = base;
            m_Frame->Allocator = this;

            SlabObject* previous = nullptr;
            for (usize offset = 0; offset < m_Frame->DataLength;
                 offset += m_ObjectSize)
            {
                auto object  = frame->Data.Offset<SlabObject*>(offset);
                object->Next = previous;
                previous     = object;
            }
            m_Frame->NextFree = previous;
            return {};
        }
        virtual void    Shutdown() { ToDoWarn(); }

        virtual Pointer Allocate()
        {
            if (!m_Frame->NextFree && !Initialize(m_ObjectSize)) return nullptr;

            PM_UNUSED auto guard = m_Lock.Lock();
            auto           frame = Pointer(m_Frame->NextFree).As<SlabObject>();
            m_Frame->NextFree    = frame->Next;

            m_TotalAllocated += m_ObjectSize;
            return frame;
        }
        virtual void Free(Pointer memory) override
        {
            if (!memory) return;

            PM_UNUSED auto guard = m_Lock.Lock();
            auto           frame = memory.As<SlabObject>();
            frame->Next          = m_Frame->NextFree;
            m_Frame->NextFree    = frame;

            m_TotalFreed += m_ObjectSize;
            return;
        }

        virtual usize AllocationSize() override { return m_ObjectSize; }

        virtual usize TotalAllocated() const { return m_TotalAllocated; }
        virtual usize TotalFreed() const { return m_TotalFreed; }
        virtual usize Used() const { return TotalAllocated() - TotalFreed(); }

      private:
        LockPolicy m_Lock;

        SlabFrame* m_Frame          = nullptr;
        usize      m_ObjectSize     = 0;

        usize      m_TotalAllocated = 0;
        usize      m_TotalFreed     = 0;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::SlabAllocator;
using Prism::SlabAllocatorBase;
using Prism::SlabObject;
#endif
