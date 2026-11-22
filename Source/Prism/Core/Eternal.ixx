/*
 * Created by v1tr10l7 on 04.01.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>
export module Prism.Core.Eternal;

export import Prism.Core.Core;
export import Prism.Core.TypeTraits;

export extern "C++"
{
    namespace Prism
    {
        namespace Details
        {
            // Not Prism::AlignedStorage (TypeTraits has that one, std-like).
            template <usize Size, usize Align>
            struct alignas(Align) EternalStorage
            {
                constexpr EternalStorage()
                    : Buffer{0}
                {
                }

                u8 Buffer[Size];
            };
        } // namespace Details

        template <typename T>
        class Eternal
        {
          public:
            static_assert(
                IsTriviallyDestructibleV<
                    Details::EternalStorage<sizeof(T), alignof(T)>>,
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
            Details::EternalStorage<sizeof(T), alignof(T)> m_Storage;
        };
    }; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    using Prism::Eternal;
#endif
}
