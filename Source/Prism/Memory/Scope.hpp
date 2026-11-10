/*
 * Created by v1tr10l7 on 27.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Concepts.hpp>
#include <Prism/Core/Core.hpp>

#include <Prism/Memory/Ref.hpp>
#include <Prism/Memory/WeakRef.hpp>

namespace Prism
{
    template <typename T>
    struct DefaultDelete
    {
        constexpr DefaultDelete() PM_NOEXCEPT = default;

        template <typename U>
            requires ConvertibleTo<U*, T*>
        constexpr DefaultDelete(const DefaultDelete<U>&) PM_NOEXCEPT
        {
        }

        constexpr void operator()(T* ptr) const
        {
            static_assert(!IsVoid<T>::Value,
                          "can't delete pointer to incomplete type");
            static_assert(sizeof(T), "can't delete pointer to incomplete type");
            delete ptr;
        }
    };
    template <typename T>
    struct DefaultDelete<T[]>
    {
      public:
        constexpr DefaultDelete() PM_NOEXCEPT = default;

        template <typename U>
            requires ConvertibleTo<U*, T*>
        constexpr DefaultDelete(const DefaultDelete<U[]>&) PM_NOEXCEPT
        {
        }

        template <typename U>
        constexpr EnableIfType<IsConvertible<U (*)[], T (*)[]>::Value>
        operator()(U* ptr) const
        {
            static_assert(sizeof(T), "can't delete pointer to incomplete type");
            delete[] ptr;
        }
    };

    template <typename T, typename Deleter = DefaultDelete<T>>
    class Scope
    {
      public:
        using ElementType = T;
        using DeleterType = Deleter;

        constexpr Scope() PM_NOEXCEPT = default;
        constexpr Scope(NullType) PM_NOEXCEPT {}
        constexpr Scope(T* instance) PM_NOEXCEPT : m_Instance(instance) {}
        constexpr Scope(T* instance, Deleter deleter) PM_NOEXCEPT
            : m_Instance(instance),
              m_Deleter(Move(deleter))
        {
        }

        constexpr Scope(Scope&& other) PM_NOEXCEPT
            : m_Instance(other.Release()),
              m_Deleter(Move(other.m_Deleter))
        {
        }
        template <typename U, typename E>
            requires(!IsArrayV<U> && IsConvertibleV<U*, T*>
                     && IsConvertibleV<E, Deleter>)
        constexpr Scope(Scope<U, E>&& other) PM_NOEXCEPT
            : m_Instance(other.Release()),
              m_Deleter(Move(other.GetDeleter()))
        {
        }

        Scope(Scope const&)            = delete;
        Scope& operator=(Scope const&) = delete;

        template <typename U>
        Scope(Ref<U> const&) = delete;
        template <typename U>
        Scope(WeakRef<U> const&) = delete;

        ~Scope() { Reset(); }

        PM_NODISCARD constexpr T* Release() PM_NOEXCEPT
        {
            return Exchange(m_Instance, nullptr);
        }
        constexpr void Reset(T* instance = nullptr) PM_NOEXCEPT
        {
            T* old = Exchange(m_Instance, instance);
            if (old) m_Deleter(old);
        }

        constexpr void Swap(Scope& other) PM_NOEXCEPT
        {
            Prism::Swap(m_Instance, other.m_Instance);
            Prism::Swap(m_Deleter, other.m_Deleter);
        }

        constexpr T*       Raw() const PM_NOEXCEPT { return m_Instance; }

        constexpr Deleter& GetDeleter() PM_NOEXCEPT { return m_Deleter; }
        constexpr const Deleter& GetDeleter() const PM_NOEXCEPT
        {
            return m_Deleter;
        }

        constexpr explicit operator bool() const PM_NOEXCEPT
        {
            return m_Instance != nullptr;
        }

        constexpr AddLValueReferenceType<T> operator*() const
            PM_NOEXCEPT(PM_NOEXCEPT(*DeclVal<T*>()))
        {
            assert(m_Instance);
            return *m_Instance;
        }
        constexpr T* operator->() const PM_NOEXCEPT
        {
            assert(m_Instance);
            return m_Instance;
        }
        constexpr T& operator[](usize i) const { return m_Instance[i]; }

        template <typename U>
        Scope& operator=(Ref<U> const&) = delete;
        template <typename U>
        Scope& operator=(WeakRef<U> const&) = delete;
        Scope& operator=(T* ptr)            = delete;

        constexpr Scope& operator=(Scope&& other) PM_NOEXCEPT
        {
            if (this != &other)
            {
                Reset(other.Release());
                m_Deleter = Move(other.m_Deleter);
            }
            return *this;
        }
        template <typename U, typename E>
            requires(!IsArrayV<U> && IsConvertibleV<U*, T*>
                     && IsConvertibleV<E, Deleter>)
        constexpr Scope& operator=(Scope<U, E>&& other) PM_NOEXCEPT
        {
            Reset(other.Release());
            m_Deleter = Move(other.GetDeleter());
            return *this;
        }
        constexpr Scope& operator=(NullType) PM_NOEXCEPT
        {
            Reset();
            return *this;
        }

        constexpr bool operator==(NullType) const PM_NOEXCEPT
        {
            return m_Instance == nullptr;
        }

        PM_NODISCARD constexpr T* Leak() PM_NOEXCEPT { return Release(); }

        static Scope               Lift(T* other) { return Scope{other}; }

      private:
        T*                         m_Instance = nullptr;
        PM_NO_UNIQUE_ADDRESS Deleter m_Deleter{};
    };

    template <typename T, typename Deleter>
    class Scope<T[], Deleter>
    {
      public:
        using ElementType = T;
        using DeleterType = Deleter;

        constexpr Scope() PM_NOEXCEPT = default;
        constexpr Scope(NullType) PM_NOEXCEPT {}
        constexpr Scope(T* instance) PM_NOEXCEPT : m_Instance(instance) {}

        constexpr Scope(Scope&& other) PM_NOEXCEPT
            : m_Instance(other.Release()),
              m_Deleter(Move(other.m_Deleter))
        {
        }
        // Only qualification conversions (e.g. T[] -> const T[]) are safe for
        // arrays: indexing a Derived[] through a Base* is undefined behavior.
        template <typename U, typename E>
            requires(IsConvertibleV<U (*)[], T (*)[]>
                     && IsConvertibleV<E, Deleter>)
        constexpr Scope(Scope<U[], E>&& other) PM_NOEXCEPT
            : m_Instance(other.Release()),
              m_Deleter(Move(other.GetDeleter()))
        {
        }

        Scope(Scope const&)            = delete;
        Scope& operator=(Scope const&) = delete;

        template <typename U>
        Scope(Ref<U> const&) = delete;
        template <typename U>
        Scope(WeakRef<U> const&) = delete;

        ~Scope() { Reset(); }

        PM_NODISCARD constexpr T* Release() PM_NOEXCEPT
        {
            return Exchange(m_Instance, nullptr);
        }
        constexpr void Reset(T* instance = nullptr) PM_NOEXCEPT
        {
            T* old = Exchange(m_Instance, instance);
            if (old) m_Deleter(old);
        }

        constexpr void Swap(Scope& other) PM_NOEXCEPT
        {
            Prism::Swap(m_Instance, other.m_Instance);
            Prism::Swap(m_Deleter, other.m_Deleter);
        }

        constexpr T*       Raw() const PM_NOEXCEPT { return m_Instance; }

        constexpr Deleter& GetDeleter() PM_NOEXCEPT { return m_Deleter; }
        constexpr const Deleter& GetDeleter() const PM_NOEXCEPT
        {
            return m_Deleter;
        }

        constexpr explicit operator bool() const PM_NOEXCEPT
        {
            return m_Instance != nullptr;
        }

        constexpr T& operator[](usize i) const
        {
            assert(m_Instance);
            return m_Instance[i];
        }

        template <typename U>
        Scope& operator=(Ref<U> const&) = delete;
        template <typename U>
        Scope& operator=(WeakRef<U> const&) = delete;
        Scope& operator=(T* ptr)            = delete;

        constexpr Scope& operator=(Scope&& other) PM_NOEXCEPT
        {
            if (this != &other)
            {
                Reset(other.Release());
                m_Deleter = Move(other.m_Deleter);
            }
            return *this;
        }
        template <typename U, typename E>
            requires(IsConvertibleV<U (*)[], T (*)[]>
                     && IsConvertibleV<E, Deleter>)
        constexpr Scope& operator=(Scope<U[], E>&& other) PM_NOEXCEPT
        {
            Reset(other.Release());
            m_Deleter = Move(other.GetDeleter());
            return *this;
        }
        constexpr Scope& operator=(NullType) PM_NOEXCEPT
        {
            Reset();
            return *this;
        }

        constexpr bool operator==(NullType) const PM_NOEXCEPT
        {
            return m_Instance == nullptr;
        }

        PM_NODISCARD constexpr T* Leak() PM_NOEXCEPT { return Release(); }

        static Scope               Lift(T* other) { return Scope{other}; }

      private:
        T*                         m_Instance = nullptr;
        PM_NO_UNIQUE_ADDRESS Deleter m_Deleter{};
    };

    template <typename T, typename... Args>
        requires(!IsArrayV<T>)
    constexpr Scope<T> CreateScope(Args&&... args)
    {
        return Scope<T>(new T(Forward<Args>(args)...));
    }

    template <typename T>
        requires IsArrayV<T> && (ExtentV<T> == 0)
    constexpr Scope<T> CreateScope(usize size)
    {
        using ElementType = RemoveExtentType<T>;
        return Scope<T>(new ElementType[size]);
    }
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::CreateScope;
using Prism::Scope;
#endif
