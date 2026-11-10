/*
 * Created by v1tr10l7 on 27.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Config.hpp>
#include <Prism/Core/TypeTraits.hpp>
#include <Prism/Memory/Ref.hpp>

namespace Prism
{
    template <typename T>
    class WeakRef;

    template <typename T>
    class Weakable;

    class WeakLink : public RefCounted<WeakLink>
    {
        template <typename T>
        friend class Weakable;
        template <typename T>
        friend class WeakRef;

      public:
        template <typename T>
        T* UnsafeRaw() const
        {
            return static_cast<T*>(m_Ptr);
        }

        bool IsNull() const { return m_Ptr == nullptr; }
        void Revoke() { m_Ptr = nullptr; }

      private:
        explicit WeakLink(void* ptr)
            : m_Ptr(ptr)
        {
        }

        mutable void* m_Ptr{nullptr};
    };

    template <typename T>
    class Weakable
    {
      public:
        template <typename U = T>
        WeakRef<U> MakeWeakRef() const
        {
            if (!m_Link)
                m_Link = Ref<WeakLink>(
                    AdoptRef,
                    *new WeakLink(const_cast<T*>(static_cast<const T*>(this))));
            return WeakRef<U>(m_Link);
        }

        /// Invalidates every WeakRef to this object. RefCounted::Unref calls
        /// this before the destructor runs, so a WeakRef can never see a
        /// half-destroyed object.
        void RevokeWeakRefs() const
        {
            if (m_Link)
            {
                m_Link->Revoke();
                m_Link = nullptr;
            }
        }

      protected:
        Weakable() = default;
        // A copy is a different object: it must not share the original's
        // link, or its WeakRefs would point at (and its destruction would
        // revoke) the original.
        Weakable(const Weakable&) {}
        Weakable& operator=(const Weakable&) { return *this; }

        ~Weakable() { RevokeWeakRefs(); }

      private:
        mutable Ref<WeakLink> m_Link;
    };

    template <typename T>
    class WeakRef
    {
        template <typename U>
        friend class WeakRef;

        template <typename U>
        friend class Weakable;

      public:
        constexpr WeakRef() = default;

        constexpr WeakRef(NullType) PM_NOEXCEPT : m_Link(nullptr) {}

        template <typename U,
                  typename
                  = EnableIfType<IsBaseOfV<T, U> && IsBaseOfV<Weakable<U>, U>
                                 && !IsSameV<RemoveCvRef<U>, NullType>>>
        WeakRef(const U& object)
            : m_Link(object.template MakeWeakRef<U>().m_Link)
        {
        }

        template <typename U,
                  typename
                  = EnableIfType<IsBaseOfV<T, U> && IsBaseOfV<Weakable<U>, U>>>
        WeakRef(const U* object)
        {
            if (object) m_Link = object->template MakeWeakRef<U>().m_Link;
        }

        template <typename U,
                  typename
                  = EnableIfType<IsBaseOfV<T, U> && IsBaseOfV<Weakable<U>, U>>>
        WeakRef(const Ref<U>& ref)
        {
            if (ref) m_Link = ref->template MakeWeakRef<U>().m_Link;
        }

        template <typename U, typename = EnableIfType<IsBaseOfV<T, U>>>
        WeakRef(const WeakRef<U>& other)
            : m_Link(other.m_Link)
        {
        }

        template <typename U, typename = EnableIfType<IsBaseOfV<T, U>>>
        WeakRef(WeakRef<U>&& other) PM_NOEXCEPT : m_Link(Move(other.m_Link))
        {
        }

        WeakRef(const WeakRef& ref) PM_NOEXCEPT            = default;
        WeakRef(WeakRef&& ref) PM_NOEXCEPT                 = default;

        WeakRef& operator=(const WeakRef& ref) PM_NOEXCEPT = default;
        WeakRef& operator=(WeakRef&& ref) PM_NOEXCEPT      = default;

        WeakRef& operator=(NullType) PM_NOEXCEPT
        {
            Clear();
            return *this;
        }

        inline T* Raw() const
        {
            return m_Link ? m_Link->template UnsafeRaw<T>() : nullptr;
        }

        T*       operator->() { return Raw(); }
        const T* operator->() const { return Raw(); }

        T&       operator*() { return *Raw(); }
        const T& operator*() const { return *Raw(); }

        operator bool() const { return m_Link ? !m_Link->IsNull() : false; }
        bool IsNull() const { return !m_Link || m_Link->IsNull(); }

        void Clear() { m_Link = nullptr; }

        template <typename U>
        WeakRef<U> As() const
        {
            return WeakRef<U>(m_Link);
        }

        Ref<T> Promote() const
        {
            T* ptr = Raw();
            return ptr ? Ref<T>(ptr) : Ref<T>();
        }

      private:
        explicit WeakRef(Ref<WeakLink> link)
            : m_Link(Move(link))
        {
        }

        Ref<WeakLink> m_Link;
    };
} // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::Weakable;
using Prism::WeakRef;
#endif
