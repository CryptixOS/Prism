/*
 * Created by v1tr10l7 on 15.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

#include <Prism/Memory/Allocator.hpp>
#include <Prism/Memory/SlabAllocator.hpp>

namespace Prism
{
    struct BigAllocMeta
    {
        usize PageCount;
        usize Size;
    };

    template <usize BucketCount, typename PageAllocPolicy, typename LockPolicy>
    class SlabPool : public AllocatorBase
    {
      public:
        constexpr static usize PAGE_SIZE = 0x1000;

        virtual bool           Initialize() override
        {
            for (usize i = 0; i < BucketCount; i++)
                assert(m_Buckets[i].Initialize(8 << i));

            return true;
        }
        virtual void    Shutdown() override {}

        virtual Pointer Allocate(usize bytes, usize alignment = 0) override
        {
            if (bytes > MAX_BUCKET_SIZE)
            {
                auto memory = LargeAllocate(bytes);
                if (memory) return memory;
            }

            for (usize i = 0; i < BucketCount; i++)
            {
                if (bytes > (8zu << i)) continue;

                auto memory = m_Buckets[i].Allocate();
                if (!memory) continue;

                m_TotalAllocated += m_Buckets[i].AllocationSize();
                return memory;
            }
            return nullptr;
        }
        virtual Pointer Callocate(usize bytes, usize alignment = 0) override
        {
            auto memory = Allocate(bytes, alignment);
            if (!memory) return nullptr;

            Memory::Fill(memory, 0, bytes);
            return memory;
        }
        virtual Pointer Reallocate(Pointer memory, usize bytes,
                                   usize alignment = 0) override
        {
            if (!memory) return Allocate(bytes, alignment);
            if (bytes == 0)
            {
                Free(memory);
                return nullptr;
            }

            // Large allocation (page-aligned)
            if ((memory.Raw() & 0xFFF) == 0)
            {
                auto  meta    = memory.Offset<BigAllocMeta*>(-PAGE_SIZE);
                usize oldSize = meta->Size;

                // If still fits in current pages, just update size
                if (Math::DivRoundUp(oldSize, PAGE_SIZE)
                    == Math::DivRoundUp(bytes, PAGE_SIZE))
                {
                    meta->Size = bytes;
                    return memory;
                }

                // Otherwise reallocate
                auto newMemory = Allocate(bytes);
                if (!newMemory) return nullptr;

                Memory::Copy(newMemory, memory, Math::Min(oldSize, bytes));
                Free(memory);
                return newMemory;
            }

            // Slab allocation
            auto allocator
                = Pointer(memory.Raw() & ~0xFFF).As<SlabHeader>()->Slab;
            usize oldSize = allocator->AllocationSize();

            // If still fits, reuse it
            if (bytes <= oldSize) return memory;

            auto newMemory = Allocate(bytes);
            if (!newMemory) return nullptr;

            Memory::Copy(newMemory, memory, oldSize);
            Free(memory);
            return newMemory;
        }
        virtual void Free(Pointer memory) override
        {
            if ((memory.Raw() & 0xfff) == 0) return LargeFree(memory);

            auto allocator = Pointer(memory.Raw() & ~0xfff).As<SlabHeader>();
            allocator->Slab->Free(memory);
            m_TotalFreed += allocator->Slab->AllocationSize();
        }

        Pointer LargeAllocate(usize bytes)
        {
            usize   pageCount = Math::DivRoundUp(bytes, PAGE_SIZE);
            Pointer memory    = PageAllocPolicy::CallocatePages(pageCount + 1);
            if (!memory) return nullptr;

            auto meta       = memory.As<BigAllocMeta>();
            meta->PageCount = pageCount;
            meta->Size      = bytes;

            m_TotalAllocated += meta->Size;
            return memory.Offset(PAGE_SIZE);
        }
        void LargeFree(Pointer memory)
        {
            auto meta = memory.Offset<Pointer>(-PAGE_SIZE).As<BigAllocMeta>();

            m_TotalFreed += meta->Size;
            PageAllocPolicy::FreePages(meta, meta->PageCount + 1);
        }

        virtual usize TotalAllocated() const override
        {
            return m_TotalAllocated;
        }
        virtual usize TotalFreed() const override { return m_TotalFreed; }
        virtual usize Used() const override
        {
            return TotalAllocated() - TotalFreed();
        }

      private:
        constexpr static usize MAX_BUCKET_SIZE = 4 << BucketCount;
        Array<SlabAllocator<PageAllocPolicy, LockPolicy>, BucketCount>
                   m_Buckets;
        LockPolicy m_Lock;

        usize      m_TotalAllocated = 0;
        usize      m_TotalFreed     = 0;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::BigAllocMeta;
using Prism::SlabPool;
#endif
