/*
 * Created by v1tr10l7 on 27.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Config.hpp>
#include <Prism/Core/TypeTraits.hpp>
#include <Prism/Memory/RefCounted.hpp>

PRISM_EXPORT namespace Prism
{
    struct AdoptRefTag
    {
    };
    inline constexpr AdoptRefTag AdoptRef{};

    template <typename T>
    class WeakRef;

    template <typename T>
    inline void RefIfNotNonNull(T* ptr)
    {
        if (ptr) ptr->Ref();
    }

    template <typename T>
    inline void UnrefIfNotNonNull(T* ptr)
    {
        if (ptr) ptr->Unref();
    }

    template <typename T>
    class PM_NODISCARD Ref
    {
        template <typename U>
        friend class Ref;
        template <typename U>
        friend class WeakRef;

      public:
        using AdoptTag              = AdoptRefTag;

        constexpr Ref() PM_NOEXCEPT = default;

        constexpr Ref(NullType) PM_NOEXCEPT : m_Instance(nullptr) {}

        constexpr Ref(T const* instance)
            : m_Instance(const_cast<T*>(instance))
        {
            RefIfNotNonNull(m_Instance);
        }

        constexpr Ref(T const& object)
            : m_Instance(const_cast<T*>(&object))
        {
            m_Instance->Ref();
        }

        constexpr Ref(AdoptRefTag, T& object) PM_NOEXCEPT : m_Instance(&object)
        {
        }

        constexpr Ref(AdoptRefTag, T* instance) PM_NOEXCEPT
            : m_Instance(instance)
        {
        }

        template <typename U, typename = EnableIfType<IsConvertibleV<U*, T*>>>
        constexpr Ref(AdoptRefTag, U* instance) PM_NOEXCEPT
            : m_Instance(static_cast<T*>(instance))
        {
        }

        constexpr Ref(Ref&& other) PM_NOEXCEPT : m_Instance(other.Leak()) {}

        constexpr Ref(const Ref& other)
            : m_Instance(other.m_Instance)
        {
            RefIfNotNonNull(m_Instance);
        }

        template <typename U, typename = EnableIfType<IsConvertibleV<U*, T*>>>
        constexpr Ref(const Ref<U>& other)
            : m_Instance(const_cast<T*>(static_cast<T const*>(other.Raw())))
        {
            RefIfNotNonNull(m_Instance);
        }

        template <typename U, typename = EnableIfType<IsConvertibleV<U*, T*>>>
        constexpr Ref(Ref<U>&& other) PM_NOEXCEPT
            : m_Instance(static_cast<T*>(other.Leak()))
        {
        }

        ~Ref() { Clear(); }

        constexpr void Swap(Ref& other) PM_NOEXCEPT
        {
            T* tmp           = m_Instance;
            m_Instance       = other.m_Instance;
            other.m_Instance = tmp;
        }

        constexpr Ref& operator=(NullType) PM_NOEXCEPT
        {
            Clear();
            return *this;
        }

        constexpr Ref& operator=(Ref&& other) PM_NOEXCEPT
        {
            Ref tmp{Move(other)};
            Swap(tmp);
            return *this;
        }

        constexpr Ref& operator=(const Ref& other)
        {
            Ref tmp{other};
            Swap(tmp);
            return *this;
        }

        template <typename U, typename = EnableIfType<IsConvertibleV<U*, T*>>>
        constexpr Ref& operator=(Ref<U>&& other) PM_NOEXCEPT
        {
            Ref tmp{Move(other)};
            Swap(tmp);
            return *this;
        }

        template <typename U, typename = EnableIfType<IsConvertibleV<U*, T*>>>
        constexpr Ref& operator=(const Ref<U>& other)
        {
            Ref tmp{other};
            Swap(tmp);
            return *this;
        }

        constexpr Ref& operator=(T const* ptr)
        {
            Ref tmp{ptr};
            Swap(tmp);
            return *this;
        }

        constexpr Ref& operator=(T const& object)
        {
            Ref tmp{object};
            Swap(tmp);
            return *this;
        }

        /// Takes `other` only if this Ref is currently null.
        /// @return true if the assignment happened.
        constexpr bool AssignIfNull(Ref&& other)
        {
            if (!IsNull() || this == &other) return false;
            *this = Move(other);
            return true;
        }

        template <typename U, typename = EnableIfType<IsConvertibleV<U*, T*>>>
        constexpr bool AssignIfNull(Ref<U>&& other)
        {
            if (!IsNull()) return false;
            *this = Move(other);
            return true;
        }

        constexpr void Clear() PM_NOEXCEPT
        {
            T* ptr = Exchange(m_Instance, nullptr);
            UnrefIfNotNonNull(ptr);
        }

        PM_NODISCARD constexpr T* Leak() PM_NOEXCEPT
        {
            return Exchange(m_Instance, nullptr);
        }

        constexpr bool IsNull() const PM_NOEXCEPT
        {
            return m_Instance == nullptr;
        }

        constexpr bool  operator!() const PM_NOEXCEPT { return !m_Instance; }
        constexpr       operator bool() const PM_NOEXCEPT { return !IsNull(); }

        constexpr       operator T*() const PM_NOEXCEPT { return m_Instance; }

        constexpr T*    operator->() const PM_NOEXCEPT { return m_Instance; }
        constexpr T&    operator*() const PM_NOEXCEPT { return *m_Instance; }

        constexpr T*    Raw() const PM_NOEXCEPT { return m_Instance; }

        constexpr usize RefCount() const PM_NOEXCEPT
        {
            return m_Instance ? m_Instance->RefCount() : 0;
        }

        constexpr void Reset(T* instance = nullptr)
        {
            Ref tmp{instance};
            Swap(tmp);
        }

        template <typename U>
        constexpr Ref<U> As() const
        {
            return Ref<U>(static_cast<U*>(m_Instance));
        }

        template <typename... Args>
        static Ref<T> Create(Args&&... args)
        {
            return Ref<T>(AdoptRef, *new T(Forward<Args>(args)...));
        }

        constexpr bool operator==(NullType) const PM_NOEXCEPT
        {
            return IsNull();
        }
        constexpr bool operator!=(NullType) const PM_NOEXCEPT
        {
            return !IsNull();
        }

        constexpr bool operator==(const Ref& other) const PM_NOEXCEPT
        {
            return m_Instance == other.m_Instance;
        }

        constexpr bool operator!=(const Ref& other) const PM_NOEXCEPT
        {
            return m_Instance != other.m_Instance;
        }

        template <typename RawPtr, typename = EnableIfType<IsPointerV<RawPtr>>>
        constexpr bool operator==(RawPtr other) const PM_NOEXCEPT
        {
            return m_Instance == other;
        }

      private:
        T* m_Instance{nullptr};
    };

    template <typename T>
    inline void Swap(Ref<T>& a, Ref<T>& b) PM_NOEXCEPT
    {
        a.Swap(b);
    }

    template <typename T>
    inline Ref<T> AdoptRefIfNotNonnull(T* object)
    {
        if (object) return Ref<T>(AdoptRef, *object);
        return {};
    }

    template <typename T, typename... Args>
    constexpr Ref<T> CreateRef(Args&&... args)
    {
        return Ref<T>::Create(Forward<Args>(args)...);
    }

    template <typename T, typename U>
    inline Ref<T> StaticPtrCast(Ref<U> const& ptr)
    {
        return Ref<T>(static_cast<T const*>(ptr.Raw()));
    }
} // namespace Prism

#if PRISM_USE_NAMESPACE != 0
PRISM_EXPORT using Prism::AdoptRef;
PRISM_EXPORT using Prism::AdoptRefIfNotNonnull;
PRISM_EXPORT using Prism::CreateRef;
PRISM_EXPORT using Prism::Ref;
PRISM_EXPORT using Prism::StaticPtrCast;
PRISM_EXPORT using Prism::Swap;
#endif
