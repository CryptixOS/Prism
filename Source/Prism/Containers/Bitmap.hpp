/*
 * Created by v1tr10l7 on 17.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Assertions.hpp>

namespace Prism
{
    class Bitmap
    {
      public:
        Bitmap() = default;
        Bitmap(u8* data, const usize size, const u8 value = 1);

        void Initialize(u8* data, const usize size, const u8 value = 1);
        void Allocate(usize entryCount);
        void Free();

        constexpr u8*   Raw() const { return m_Data; }
        constexpr usize GetSize() const { return m_EntryCount; }

        void     SetAll(const u8 value);
        void SetIndex(const usize index, const bool value);
        bool GetIndex(const usize index) const;

      private:
        u8*   m_Data;
        usize m_EntryCount = 0;
        usize m_Size       = 0;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::Bitmap;
#endif
