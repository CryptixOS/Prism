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

namespace Prism
{
    template <typename T>
    class Ref
    {
      public:
        constexpr Ref()
            : m_Instance(nullptr)
        {
        }
        constexpr Ref(NullType)
            : m_Instance(nullptr)
        {
        }
        constexpr Ref(T* instance)
            : m_Instance(instance)
        {
            static_assert(IsBaseOfV<RefCounted, T>, "Class is not RefCounted!");

            IncRef();
        }
        template <typename U>
        constexpr Ref(const Ref<U>& other)
        {
            m_Instance = reinterpret_cast<T*>(other.m_Instance);
            IncRef();
        }
        template <typename U>
        constexpr Ref(Ref<U>&& other)
        {
            m_Instance       = reinterpret_cast<T*>(other.m_Instance);
            other.m_Instance = nullptr;
        }
        constexpr Ref(const Ref<T>& other)
            : m_Instance(other.m_Instance)
        {
            IncRef();
        }

        virtual ~Ref() { DecRef(); }

        constexpr Ref& operator=(NullType)
        {
            DecRef();
            m_Instance = nullptr;
            return *this;
        }
        constexpr Ref& operator=(const Ref<T>& other)
        {
            if (this == &other) return *this;
            const_cast<Ref<T>*>(&other)->IncRef();
            DecRef();

            m_Instance = other.m_Instance;
            return *this;
        }
        template <typename U>
        constexpr Ref& operator=(const Ref<U>& other)
        {
            other.IncRef();
            DecRef();

            m_Instance = other.m_Instance;
            return *this;
        }
        template <typename U>
        constexpr Ref& operator=(Ref<U>&& other)
        {
            DecRef();

            m_Instance       = other.m_Instance;
            other.m_Instance = nullptr;
            return *this;
        }

        constexpr    operator bool() const { return m_Instance != nullptr; }

        constexpr T* operator->() { return m_Instance; }
        constexpr const T* operator->() const { return m_Instance; }

        constexpr T&       operator*() { return *m_Instance; }
        constexpr const T& operator*() const { return *m_Instance; }

        constexpr T*       Raw() { return m_Instance; }
        constexpr const T* Raw() const { return m_Instance; }

        constexpr usize    RefCount() const { return m_Instance->RefCount(); }
        constexpr void     Reset(T* instance = nullptr)
        {
            DecRef();
            m_Instance = instance;
        }

        template <typename U>
        constexpr Ref<U> As() const
        {
            return Ref<U>(*this);
        }

        template <typename... Args>
        static Ref<T> Create(Args&&... args)
        {
            return Ref<T>(new T(Forward<Args>(args)...));
        }

        constexpr bool operator==(const Ref<T>& other) const
        {
            return m_Instance == other.m_Instance;
        }
        constexpr bool operator!=(const Ref<T>& other) const
        {
            return m_Instance != other.m_Instance;
        }

      protected:
        template <class U>
        friend class Ref;
        mutable T*     m_Instance = nullptr;

        constexpr void IncRef()
        {
            if (!m_Instance) return;
            m_Instance->Ref();
        }
        constexpr void DecRef()
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
    constexpr Ref<T> CreateRef(Args&&... args)
    {
        return Ref<T>(new T(Forward<Args>(args)...));
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::CreateRef;
using Prism::Ref;
#endif
