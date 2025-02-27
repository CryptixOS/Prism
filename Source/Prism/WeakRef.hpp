/*
 * Created by v1tr10l7 on 27.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Ref.hpp>

namespace Prism
{
    template <typename T>
    class WeakRef
    {
      public:
        WeakRef() = default;

        WeakRef(Ref<T> ref) { m_Instance = ref.Raw(); }
        WeakRef(T* instance) { m_Instance = instance; }

        T*       operator->() { return m_Instance; }
        const T* operator->() const { return m_Instance; }

        T&       operator*() { return &m_Instance; }
        const T& operator*() const { return &m_Instance; }

        operator bool() const { return m_Instance != nullptr; }

        template <typename U>
        WeakRef<U> As() const
        {
            return WeakRef<U>(reinterpret_cast<U*>(m_Instance));
        }

      private:
        T* m_Instance = nullptr;
    };
}; // namespace Prism
