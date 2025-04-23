/*
 * Created by v1tr10l7 on 21.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

namespace Prism
{
    template <typename T, usize ShortCapacity>
    class SSOStorage
    {
      public:
        SSOStorage() = default;

        constexpr usize Size() const { return m_Size; }
        constexpr usize Capacity() const
        {
            return IsLong() ? Long().Capacity() : Short().Capacity;
        }

        constexpr T* Raw() const
        {
            return IsLong() ? Long().Data : Short().Data;
        }
        constexpr T& At(usize index) { return Raw()[index]; }

        void         EnsureCapacity(usize capacity)
        {
            if (Capacity() >= capacity) return;
            if (FitsInSSO() && !IsLong()) ShortInit();

            T* newData = new T[capacity];

            for (usize i = 0; i < Size(); i++) newData[i] = std::move(At(i));
            delete[] Long().Data;

            Long().Data     = newData;
            Long().Capacity = capacity;
        }

      private:
        struct ShortStorage
        {
            T Data[ShortCapacity];
        };
        struct LongStorage
        {
            T*    Data = nullptr;
            usize Capacity;
        };

        union
        {
            ShortStorage Short;
            LongStorage  Long;
        } m_Storage;

        bool  m_UsingSBO = true;
        usize m_Size     = 0;

        void  ShortInit()
        {
            if (IsLong()) delete[] m_Storage.Long.Data;

            void* sbo = &m_Storage;
            std::memset(sbo, 0, *m_Storage);
            m_UsingSBO = true;
            m_Size     = 0;
        }

        constexpr ShortStorage& Short() { return m_Storage.Short; }
        constexpr LongStorage&  Long() { return m_Storage.Long; }
        constexpr bool          IsLong() const { return m_UsingSBO; }
        constexpr bool          FitsInSSO(usize size) const
        {
            return size < ShortCapacity;
        }
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::SSOStorage;
#endif
