/*
 * Created by v1tr10l7 on 27.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Config.hpp>
#include <Prism/Core/TypeTraits.hpp>
#include <Prism/Debug/Assertions.hpp>
#include <Prism/Utility/Atomic.hpp>

namespace Prism
{
    class RefCounted
    {
      public:
        virtual ~RefCounted() { PrismAssert(m_RefCount == 0); }

        usize RefCount() const { return m_RefCount.Load(); }

        void  Ref() { ++m_RefCount; }
        void  Unref() { --m_RefCount; }

      private:
        Atomic<usize> m_RefCount = 0;
    };

    template <typename T>
    class Ref
    {
      public:
        Ref()
            : m_Instance(nullptr)
        {
        }
        Ref(std::nullptr_t)
            : m_Instance(nullptr)
        {
        }
        Ref(T* instance)
            : m_Instance(instance)
        {
            static_assert(IsBaseOfV<RefCounted, T>, "Class is not RefCounted!");

            IncRef();
        }
        template <typename U>
        Ref(const Ref<U>& other)
        {
            m_Instance = reinterpret_cast<T*>(other.m_Instance);
            IncRef();
        }
        template <typename U>
        Ref(Ref<U>&& other)
        {
            m_Instance       = reinterpret_cast<T*>(other.m_Instance);
            other.m_Instance = nullptr;
        }
        Ref(const Ref<T>& other)
            : m_Instance(other.m_Instance)
        {
            IncRef();
        }

        virtual ~Ref() { DecRef(); }

        Ref& operator=(std::nullptr_t)
        {
            DecRef();
            m_Instance = nullptr;
            return *this;
        }
        Ref& operator=(const Ref<T>& other)
        {
            if (this == &other) return *this;
            const_cast<Ref<T>&>(other).IncRef();
            DecRef();

            m_Instance = other.m_Instance;
            return *this;
        }
        template <typename U>
        Ref& operator=(const Ref<U>& other)
        {
            other.IncRef();
            DecRef();

            m_Instance = other.m_Instance;
            return *this;
        }
        template <typename U>
        Ref& operator=(Ref<U>&& other)
        {
            DecRef();

            m_Instance       = other.m_Instance;
            other.m_Instance = nullptr;
            return *this;
        }

        operator bool() const { return m_Instance != nullptr; }

        T*              operator->() { return m_Instance; }
        const T*        operator->() const { return m_Instance; }

        T&              operator*() { return *m_Instance; }
        const T&        operator*() const { return *m_Instance; }

        T*              Raw() { return m_Instance; }
        const T*        Raw() const { return m_Instance; }

        constexpr usize RefCount() const { return m_Instance->RefCount(); }
        void            Reset(T* instance = nullptr)
        {
            DecRef();
            m_Instance = instance;
        }

        template <typename U>
        Ref<U> As() const
        {
            return Ref<U>(*this);
        }

        template <typename... Args>
        static Ref<T> Create(Args&&... args)
        {
            return Ref<T>(new T(Forward<Args>(args)...));
        }

        bool operator==(const Ref<T>& other) const
        {
            return m_Instance == other.m_Instance;
        }
        bool operator!=(const Ref<T>& other) const
        {
            return m_Instance != other.m_Instance;
        }

      private:
        T*   m_Instance = nullptr;

        void IncRef()
        {
            if (!m_Instance) return;
            m_Instance->Ref();
        }
        void DecRef()
        {
            if (!m_Instance) return;
            m_Instance->Unref();

            if (m_Instance->RefCount() == 0)
            {
                delete m_Instance;
                m_Instance = nullptr;
            }
        }
    };

    template <typename T, typename... Args>
    Ref<T> CreateRef(Args&&... args)
    {
        return Ref<T>(new T(Forward<Args>(args)...));
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::CreateRef;
using Prism::Ref;
using Prism::RefCounted;
#endif
