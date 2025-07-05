/*
 * Created by v1tr10l7 on 04.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Concepts.hpp>
#include <Prism/Memory/Pointer.hpp>

namespace Prism
{
    template <typename>
    class Optional;

    struct NullOptType
    {
        explicit NullOptType() = default;
    };
    inline constexpr NullOptType NullOpt{};

    template <typename T>
        requires(!IsLValueReferenceV<T>)
    class [[nodiscard]] Optional<T>
    {
        static_assert(!IsLValueReferenceV<T> && !IsRValueReferenceV<T>);

      public:
        using ValueType      = T;

        constexpr Optional() = default;
        template <SameAs<NullOptType> V>
        constexpr Optional(V)
        {
        }
        constexpr Optional(const Optional& other) = default;
        constexpr Optional(Optional&& other)
            : m_HasValue(other.m_HasValue)
        {
            if (other.HasValue()) new (&m_Storage) T(Move(other.Value()));
        }

        template <SameAs<NullOptType> V>
        Optional& operator=(V)
        {
            Reset();
            return *this;
        }

        Optional(const Optional& other)
            requires(!IsCopyConstructibleV<T>)
        = delete;
        Optional(Optional&& other)
            requires(!IsMoveConstructibleV<T>)
        = delete;

        Optional& operator=(const Optional& other)
            requires(!IsCopyConstructibleV<T> || !IsDestructibleV<T>)
        = delete;
        Optional& operator=(const Optional& other) = default;
        Optional& operator=(Optional&& other)
            requires(!IsMoveConstructibleV<T> || !IsDestructibleV<T>)
        = delete;

        constexpr Optional(const Optional& other)
            requires(!IsTriviallyCopyConstructibleV<T>)
            : m_HasValue(other.m_HasValue)
        {
            if (other.HasValue()) new (&m_Storage) T(other.Value());
        }
        template <typename U>
        // requires(IsConstructibleV<T, const U&>
        // && !IsSpecializationOf<T, Optional>
        // && !IsSpecializationOf<U, Optional>
        // && !IsLValueReferenceV<U>)
        constexpr explicit Optional(const Optional<U>& other)
            : m_HasValue(other.m_HasValue)
        {
            if (other.HasValue()) new (&m_Storage) T(other.Value());
        }

        template <typename U>
        /*   requires(IsConstructibleV<T, U &&>
                    && !IsSpecializationOf<T, Optional>
                    && !IsSpecializationOf<U, Optional>
                    && !IsLValueReferenceV<U>)*/
        constexpr explicit Optional(Optional<U>&& other)
            : m_HasValue(other.m_HasValue)
        {
            if (other.HasValue()) new (&m_Storage) T(Move(other.m_Value));
        }

        template <typename U = T>
            requires(!IsSameV<NullOptType, RemoveCvRefType<U>>)
        constexpr explicit(!IsConvertible<U&&, T>::Value) Optional(U&& value)
            requires(!IsSameV<RemoveCvRefType<U>, Optional<T>>
                     && IsConstructibleV<T, U &&>)
            : m_HasValue(true)
        {
            new (&m_Storage) T(Forward<U>(value));
        }

        constexpr Optional& operator=(Optional const& other)
            requires(!IsTriviallyCopyConstructibleV<T>
                     || !std::is_trivially_destructible_v<T>)
        {
            if (this != &other)
            {
                Reset();
                m_HasValue = other.m_HasValue;
                if (other.HasValue()) new (&m_Storage) T(other.Value());
            }
            return *this;
        }

        constexpr Optional& operator=(Optional&& other)
        {
            if (this != &other)
            {
                Reset();
                m_HasValue = other.m_HasValue;
                if (other.HasValue()) new (&m_Storage) T(Move(other.Value()));
            }
            return *this;
        }

        constexpr bool operator==(NullOptType) const { return !HasValue(); }
        constexpr bool operator!=(NullOptType) const { return HasValue(); }
        template <typename O>
        constexpr bool operator==(Optional<O> const& other) const
        {
            return HasValue() == other.HasValue()
                && (!HasValue() || Value() == other.Value());
        }

        template <typename O>
        constexpr bool operator==(O const& other) const
        {
            return HasValue() && Value() == other;
        }

        constexpr ~Optional()
            requires(!IsDestructibleV<T>)
        = delete;
        constexpr ~Optional() = default;
        constexpr ~Optional()
            requires(!std::is_trivially_destructible_v<T>)
        {
            Reset();
        }

        constexpr void Reset()
        {
            if (!m_HasValue) return;

            Value().~T();
            m_HasValue = false;
        }

        template <typename... Args>
        constexpr void Emplace(Args&&... args)
        {
            Reset();
            m_HasValue = true;
            new (&m_Storage) T(Forward<Args>(args)...);
        }

        constexpr      operator bool() const PM_NOEXCEPT { return HasValue(); }
        constexpr bool HasValue() const { return m_HasValue; }
        [[nodiscard]] PM_ALWAYS_INLINE T& Value() &
        {
            assert(m_HasValue);
            return *Launder(reinterpret_cast<T*>(&m_Storage));
        }

        constexpr T const& Value() const&
        {
            assert(m_HasValue);
            return *Launder(reinterpret_cast<T const*>(&m_Storage));
        }

        constexpr T Value() && { return ReleaseValue(); }

        constexpr T ReleaseValue()
        {
            assert(m_HasValue);
            T releasedValue = Move(Value());

            Value().~T();
            m_HasValue = false;

            return releasedValue;
        }

        constexpr T ValueOr(const T& fallback) const&
        {
            if (m_HasValue) return Value();
            return fallback;
        }

        constexpr const T& operator*() const { return Value(); }
        constexpr T&       operator*() { return Value(); }

        constexpr const T* operator->() const { return &Value(); }
        constexpr T*       operator->() { return &Value(); }

        /*constexpr Optional& operator=(Optional const& other)
            requires(!std::IsTriviallyCopyConstructible<T>
                     || !std::is_trivially_destructible<T>)
        {*/

      private:
        alignas(T) u8 m_Storage[sizeof(T)];
        bool m_HasValue = false;
    };
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX == 1
using Prism::NullOpt;
using Prism::Optional;
#endif
