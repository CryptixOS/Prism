/*
 * Created by v1tr10l7 on 27.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Ref.hpp>
#include <Prism/WeakRef.hpp>

namespace Prism
{
    template <typename T>
    struct ScopeDeleter
    {
        void operator()(T* instance) { delete instance; }
    };
    template <typename T, typename Deleter = ScopeDeleter<T>>
    class Scope
    {
      public:
        Scope()               = default;
        Scope(std::nullptr_t) = default;
        Scope(Scope&& other)
            : m_Instance(other.m_Instance)
        {
            other.m_Instance = nullptr;
        }

        template <typename U>
        Scope(Scope<U>&& other)
            : m_Instance(other.Leak())
        {
        }

        Scope(Scope const&) = delete;
        template <typename U>
        Scope(Scope<U> const&)         = delete;
        Scope& operator=(Scope const&) = delete;
        template <typename U>
        Scope& operator=(Scope<U> const&) = delete;

        template <typename U>
        Scope(Ref<U> const&) = delete;
        template <typename U>
        Scope(WeakRef<U> const&) = delete;

        ~Scope() { Reset(); }

        T* Release() PM_NOEXCEPT { return std::exchange(m_Instance, nullptr); }
        void Reset(T* instance = {}) PM_NOEXCEPT
        {
            auto* ptr = std::exchange(m_Instance, instance);
            Deleter{}(ptr);
        }
        template <class U>
        void Reset(U instance) noexcept
        {
            Deleter{}(m_Instance);

            m_Instance = instance;
        }
        void Reset(std::nullptr_t = nullptr) PM_NOEXCEPT
        {
            Deleter{}(m_Instance);
            m_Instance = nullptr;
        }

        void Swap(Scope& other) PM_NOEXCEPT
        {
            std::swap(m_Instance, other.m_Instance);
        }
        template <typename U>
        void Swap(Scope<U>& other)
        {
            std::swap(m_Instance, other.m_Instance);
        }

        T*                 Raw() const PM_NOEXCEPT { return m_Instance; }

        constexpr Deleter& GetDeleter() PM_NOEXCEPT { return Deleter(); }
        constexpr Deleter& GetDeleter() const PM_NOEXCEPT { return Deleter(); }

        explicit           operator bool() const PM_NOEXCEPT
        {
            return m_Instance != nullptr;
        }

        typename std::add_lvalue_reference<T>::type operator*() const
            noexcept(noexcept(*std::declval<T*>()))
        {
            return *m_Instance;
        }

        T* operator->() const PM_NOEXCEPT
        {
            Assert(m_Instance);
            return m_Instance;
        }

        T& operator[](std::size_t i) const { return m_Instance[i]; }

        template <typename U>
        Scope& operator=(Ref<U> const&) = delete;
        template <typename U>
        Scope& operator=(WeakRef<U> const&) = delete;

        Scope& operator=(Scope&& other)
        {
            Scope ptr(std::move(other));
            swap(ptr);
            return *this;
        }

        template <typename U>
        Scope& operator=(Scope<U>&& other)
        {
            Scope ptr(std::move(other));
            Swap(ptr);
            return *this;
        }

        Scope& operator=(T* ptr) = delete;

        Scope& operator=(std::nullptr_t)
        {
            Reset();
            return *this;
        }

        [[nodiscard]]
        T* Leak()
        {
            T* leaked  = m_Instance;
            m_Instance = nullptr;
            return leaked;
        }

        static Scope Lift(T* other) { return Scope{other}; }

      private:
        T* m_Instance = nullptr;
    };
}; // namespace Prism
