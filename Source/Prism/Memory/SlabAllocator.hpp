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

namespace Prism
{
    struct SlabObject
    {
        SlabObject* Next = nullptr;
    };

    constexpr u64 SLAB_CANARY = 0xdeadbeefcafebabe;
    constexpr u64 FREE_POISON = 0xdeaddeaddeaddead;

#ifndef PRISM_PAGE_SIZE
    #define PRISM_PAGE_SIZE 0x1000
#endif
    constexpr usize PAGE_SIZE = PRISM_PAGE_SIZE;

    // TODO(v1tr10l7): Memory poisoning
    // TODO(v1tr10l7): canaries
    // TODO(v1tr10l7): frame tracking
    // TODO(v1tr10l7): alignment, shutdown
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

            PM_UNUSED auto guard = m_Lock.Lock();
            m_ObjectSize         = chunkSize;
            return Grow();
        }
        virtual void    Shutdown() { PrismToDoWarn(); }

        virtual Pointer Allocate()
        {
            // The emptiness check, the refill and the pop must all happen
            // under the lock: otherwise two CPUs can both see one free object
            // left, and one of them pops (and dereferences) nullptr.
            PM_UNUSED auto guard = m_Lock.Lock();
            if (!m_Frame || !m_Frame->NextFree)
                if (!Grow()) return nullptr;

            auto object       = m_Frame->NextFree;
            m_Frame->NextFree = object->Next;

            // assert(VerifyCanary(object));

            m_TotalAllocated += m_ObjectSize;
            return object;
        }
        virtual void Free(Pointer memory) override
        {
            if (!memory) return;

            PM_UNUSED auto guard  = m_Lock.Lock();

            auto           object = memory.As<SlabObject>();
            // assert(VerifyCanary(object));
            // assert(!IsPoisoned(object));

            // Poison(object);
            object->Next          = m_Frame->NextFree;
            m_Frame->NextFree     = object;

            m_TotalFreed += m_ObjectSize;
            return;
        }

        virtual usize AllocationSize() override { return m_ObjectSize; }

        virtual usize TotalAllocated() const { return m_TotalAllocated; }
        virtual usize TotalFreed() const { return m_TotalFreed; }
        virtual usize Used() const { return TotalAllocated() - TotalFreed(); }

      private:
        // Carves a fresh slab into objects. Caller must hold m_Lock.
        ErrorOr<void> Grow()
        {
            // One page per slab: the SlabFrame header lives at the start of
            // the page and Free()/Reallocate() find it by masking an object's
            // address down to the page, so objects can't spill into a second
            // page.
            constexpr usize SLAB_SIZE = PAGE_SIZE;

            Pointer         base = PageAllocPolicy::CallocatePages(1);
            if (!base) return Error(ENOMEM);

            usize overhead = 0;
            for (; overhead < sizeof(SlabFrame); overhead += m_ObjectSize);

            auto frame         = new (base.As<void>()) SlabFrame;
            frame->Data        = base.Offset<Pointer>(overhead);
            frame->DataLength  = SLAB_SIZE - overhead;
            frame->Base        = base;
            frame->TotalLength = SLAB_SIZE;
            frame->Allocator   = this;

            // Objects of older slabs that get freed are pushed onto the
            // current frame's list, so keep whatever is left on it.
            SlabObject* previous = m_Frame ? m_Frame->NextFree : nullptr;
            for (usize offset = 0; offset + m_ObjectSize <= frame->DataLength;
                 offset += m_ObjectSize)
            {
                auto object  = frame->Data.Offset<Pointer>(offset).As<SlabObject>();
                object->Next = previous;
                previous     = object;
            }
            frame->NextFree = previous;
            m_Frame         = frame;
            return {};
        }

        LockPolicy             m_Lock;

        SlabFrame*             m_Frame          = nullptr;
        usize                  m_ObjectSize     = 0;

        usize                  m_TotalAllocated = 0;
        usize                  m_TotalFreed     = 0;

        static constexpr usize CANARY_SIZE      = sizeof(u64);
        PM_UNUSED inline void  WriteCanary(Pointer object)
        {
            *object.As<u64>()                                = SLAB_CANARY;
            *object.Offset<u64*>(m_ObjectSize - CANARY_SIZE) = SLAB_CANARY;
        }
        PM_UNUSED inline bool VerifyCanary(Pointer object)
        {
            return *object.As<u64>() == SLAB_CANARY
                && *object.Offset<u64*>(m_ObjectSize - CANARY_SIZE)
                       == SLAB_CANARY;
        }

        PM_UNUSED inline void Poison(Pointer object)
        {
            Memory::Fill(object, 0xde, m_ObjectSize - 2 * CANARY_SIZE);
        }
        PM_UNUSED inline bool IsPoisoned(Pointer object)
        {
            for (usize i = 0;
                 i < (m_ObjectSize - 2 * CANARY_SIZE) / sizeof(u64); i++)
                if (object.As<u8>()[i] != FREE_POISON) return false;

            return true;
        }
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::SlabAllocator;
using Prism::SlabAllocatorBase;
using Prism::SlabObject;
#endif
