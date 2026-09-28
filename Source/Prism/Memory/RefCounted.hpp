/*
 * Created by v1tr10l7 on 17.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/NonCopyable.hpp>
#include <Prism/Core/NonMovable.hpp>
#include <Prism/Debug/Assertions.hpp>
#include <Prism/Utility/Atomic.hpp>

namespace Prism
{
    class RefCountedBase : public NonCopyable<RefCountedBase>,
                           public NonMovable<RefCountedBase>
    {
      public:
        using RefCountType = usize;

        PM_ALWAYS_INLINE void Ref() const PM_NOEXCEPT
        {
            assert(m_RefCount.Load() > 0);
            ++m_RefCount;
        }

        PM_NODISCARD usize RefCount() const PM_NOEXCEPT
        {
            return m_RefCount.Load();
        }

      protected:
        constexpr RefCountedBase() PM_NOEXCEPT = default;

        ~RefCountedBase() { assert(m_RefCount.Load() == 0); }

        PM_ALWAYS_INLINE usize DerefBase() const PM_NOEXCEPT
        {
            assert(m_RefCount.Load() > 0);
            return --m_RefCount;
        }

        mutable Atomic<usize> m_RefCount{1};
    };

#if 0
    template <typename T>
    class RefCounted : public RefCountedBase
    {
      public:
        void Unref() const
        {
            auto* self = const_cast<T*>(static_cast<T const*>(this));

            if (DerefBase() == 0)
            {
                if constexpr (requires { self->WillBeDestroyed(); })
                    self->WillBeDestroyed();
                delete self;
            }
        }
    };
#else
    template <typename T>
    class Ref;
    template <typename T>
    class RefCounted
    {
      public:
        virtual ~RefCounted() { assert(m_RefCount == 0); }

        usize               RefCount() const { return m_RefCount.Load(); }

        void                Ref() { ++m_RefCount; }
        void                Unref() { --m_RefCount; }

        class Prism::Ref<T> RefFromThis()
        {
            return Prism::Ref<T>(static_cast<T*>(this));
        }

        class Prism::Ref<const T> RefFromThis() const
        {
            return Prism::Ref<const T>(static_cast<const T*>(this));
        }

      protected:
        mutable Atomic<usize> m_RefCount = 0;
    };
#endif
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::RefCounted;
#endif
