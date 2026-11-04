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

    template <typename T>
    class Ref;

    /**
     * @brief Intrusive reference count, Serenity-style.
     *
     * A new object starts with a count of 1, owned by whoever created it.
     * Hand it to a Ref with AdoptRef (Ref::Create / CreateRef do this). The
     * last Unref() deletes the object, calling T::WillBeDestroyed() first if
     * T has one. Objects managed this way must be heap-allocated with `new`.
     */
    template <typename T>
    class RefCounted
    {
      public:
        using RefCountType = usize;

        RefCounted(const RefCounted&)            = delete;
        RefCounted& operator=(const RefCounted&) = delete;

        PM_NODISCARD usize RefCount() const PM_NOEXCEPT
        {
            return m_RefCount.Load();
        }

        void Ref() const PM_NOEXCEPT
        {
            assert(m_RefCount.Load() > 0 && "Ref() on a dead object");
            ++m_RefCount;
        }
        /// @return true if this call destroyed the object.
        bool Unref() const
        {
            assert(m_RefCount.Load() > 0 && "Unref() underflow");
            if (--m_RefCount != 0) return false;

            auto* self = const_cast<T*>(static_cast<const T*>(this));
            if constexpr (requires { self->RevokeWeakRefs(); })
                self->RevokeWeakRefs();
            if constexpr (requires { self->WillBeDestroyed(); })
                self->WillBeDestroyed();
            delete self;
            return true;
        }

        class Prism::Ref<T> RefFromThis()
        {
            return Prism::Ref<T>(static_cast<T*>(this));
        }
        class Prism::Ref<const T> RefFromThis() const
        {
            return Prism::Ref<const T>(static_cast<const T*>(this));
        }

      protected:
        RefCounted() PM_NOEXCEPT = default;
        // Virtual so a Ref<Base> can delete a Derived.
        virtual ~RefCounted()
        {
            assert(m_RefCount.Load() == 0
                   && "Destroying an object that still has references");
        }

        mutable Atomic<usize> m_RefCount{1};
    };
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::RefCounted;
#endif
