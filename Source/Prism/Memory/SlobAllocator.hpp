/*
 * Created by v1tr10l7 on 17.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Error.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Log.hpp>

#include <Prism/Memory/Allocator.hpp>
#include <Prism/Utility/Math.hpp>

namespace Prism
{
    struct SlobBlock
    {
        SlobBlock* Next = nullptr;
        usize      Size = 0;
    };

    class SlobAllocator final : public AllocatorBase
    {
      public:
        constexpr static usize PAGE_SIZE = 0x1000;

        virtual bool           Initialize() override
        {
            if (m_Initialized) return true;

            Pointer base         = 0;
            m_FreeBlocks         = base.As<SlobBlock>();
            m_FreeBlocks->Next   = nullptr;
            m_FreeBlocks->Size   = PAGE_SIZE;

            return m_Initialized = true;
        }
        virtual void    Shutdown() override {}

        virtual Pointer Allocate(usize bytes, usize alignment = 0) override
        {
            SlobBlock* previous = nullptr;
            SlobBlock* current  = m_FreeBlocks;
            while (current)
            {
                previous = current;
                current  = current->Next;

                if (current->Size >= bytes)
                {
                    if (current->Size > bytes + sizeof(SlobBlock))
                    {
                        auto* next = Pointer(current).Offset<SlobBlock*>(bytes);
                        next->Size = current->Size - bytes;
                        next->Next = current->Next;

                        current->Next = nullptr;
                        current->Size = bytes;

                        if (previous) previous->Next = next;
                        else m_FreeBlocks = next;
                        continue;
                    }

                    if (previous) previous->Next = current->Next;
                    else m_FreeBlocks = current->Next;
                }

                return Pointer(current).Offset(sizeof(SlobBlock));
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
            return nullptr;
        }
        virtual void Free(Pointer address) override
        {
            auto block   = address.Offset<SlobBlock*>(-sizeof(SlobBlock));
            block->Next  = m_FreeBlocks;
            m_FreeBlocks = block;
        }

        virtual usize TotalAllocated() const override {}
        virtual usize TotalFreed() const override {}
        virtual usize Used() const override {}

      private:
        bool       m_Initialized = false;

        usize      m_BlockSize   = 0;
        SlobBlock* m_FreeBlocks  = nullptr;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::SlobAllocator;
#endif
