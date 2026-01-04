/*
 * Created by v1tr10l7 on 04.01.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Core.hpp>
#include <Prism/Core/TypeTraits.hpp>

namespace Prism
{
    template <usize Size, usize Align>
    struct alignas(Align) AlignedStorage
    {
        constexpr AlignedStorage()
            : Buffer{0}
        {
        }

        u8 Buffer[Size];
    };

    template <typename... T>
    using AlignedUnion
        = AlignedStorage<Max({sizeof(T)...}), Max({alignof(T)...})>;

    template <typename T>
    class Eternal
    {
      public:
        static_assert(
#if defined(__clang__) && __clang_major__ >= 15
            IsTriviallyDestructibleV<AlignedStorage<sizeof(T), alignof(T)>>,
#else
            HasTrivialDestructorV<AlignedStorage<sizeof(T), alignof(T)>>,
#endif
            "Eternal<T> should have a trivial destructor");

        template <typename... Args>
        Eternal(Args&&... args)
        {
            new (&m_Storage) T(Forward<Args>(args)...);
        }

        T& Instance() { return *reinterpret_cast<T*>(&m_Storage); }

        T& operator*() { return *reinterpret_cast<T*>(&m_Storage); }
        T* operator->() { return reinterpret_cast<T*>(&m_Storage); }

      private:
        AlignedStorage<sizeof(T), alignof(T)> m_Storage;
    };
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::Eternal;
#endif
