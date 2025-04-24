/*
 * Created by v1tr10l7 on 04.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

namespace Prism
{
    template <typename>
    class Optional;

    struct NullOpt
    {
        explicit NullOpt() = default;
    };

    template <typename T>
        requires(!std::is_lvalue_reference<T>())
    class [[nodiscard]] Optional<T>
    {
        static_assert(!std::is_lvalue_reference<T>()
                      && !std::is_rvalue_reference<T>());

      public:
        using ValueType      = T;

        constexpr Optional() = default;
        template <std::same_as<NullOpt> V>
        Optional(V)
        {
        }

        template <std::same_as<NullOpt> V>
        Optional& operator=(V)
        {
            Reset();
            return *this;
        }

        Optional(const Optional& other)
            requires(!std::is_copy_constructible_v<T>)
        = delete;
        Optional(const Optional& other) = default;
        Optional(Optional&& other)
            requires(!std::is_move_constructible_v<T>)
        = delete;

        Optional& operator=(const Optional& other)
            requires(!std::is_copy_constructible_v<T>
                     || !std::is_destructible_v<T>)
        = delete;
        Optional& operator=(const Optional& other) = default;
        Optional& operator=(Optional&& other)
            requires(!std::is_move_constructible_v<T>
                     || !std::is_destructible_v<T>)
        = delete;

        constexpr Optional(const Optional& other)
            requires(!std::is_trivially_copy_constructible_v<T>)
            : m_HasValue(other.m_HasValue)
        {
            if (other.HasValue()) new (&m_Storage) T(other.Value());
        }
        constexpr Optional(Optional&& other)
            : m_HasValue(other.m_HasValue)
        {
            if (other.HasValue()) new (&m_Storage) T(std::move(other.Value()));
        }

        template <typename U>
        // requires(std::is_constructible_v<T, const U&>
        // && !std::is_specialization_of<T, Optional>
        // && !std::is_specialization_of<U, Optional>
        // && !std::is_lvalue_reference_v<U>)
        constexpr explicit Optional(const Optional<U>& other)
            : m_HasValue(other.m_HasValue)
        {
            if (other.HasValue()) new (&m_Storage) T(other.Value());
        }

        template <typename U>
        /*   requires(std::is_constructible_v<T, U &&>
                    && !std::is_specialization_of<T, Optional>
                    && !std::is_specialization_of<U, Optional>
                    && !std::is_lvalue_reference_v<U>)*/
        constexpr explicit Optional(Optional<U>&& other)
            : m_HasValue(other.m_HasValue)
        {
            if (other.HasValue()) new (&m_Storage) T(std::move(other.m_Value));
        }

        template <typename U = T>
            requires(!std::is_same<NullOpt, std::remove_cvref_t<U>>())
        constexpr explicit(!std::is_convertible_v<U&&, T>) Optional(U&& value)
            requires(!std::is_same<std::remove_cvref<U>, Optional<T>>()
                     && std::is_constructible_v<T, U &&>)
            : m_HasValue(true)
        {
            new (&m_Storage) T(std::forward<U>(value));
        }

        constexpr Optional& operator=(Optional const& other)
            requires(!std::is_trivially_copy_constructible_v<T>
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
                if (other.HasValue())
                    new (&m_Storage) T(std::move(other.Value()));
            }
            return *this;
        }

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
            new (&m_Storage) T(std::forward<Args>(args)...);
        }

        constexpr bool HasValue() const { return m_HasValue; }
        [[nodiscard]] PM_ALWAYS_INLINE T& Value() &
        {
            assert(m_HasValue);
            return *__builtin_launder(reinterpret_cast<T*>(&m_Storage));
        }

        constexpr T const& Value() const&
        {
            assert(m_HasValue);
            return *__builtin_launder(reinterpret_cast<T const*>(&m_Storage));
        }

        constexpr T Value() && { return ReleaseValue(); }

        constexpr T ReleaseValue()
        {
            assert(m_HasValue);
            T releasedValue = std::move(Value());

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
            requires(!std::is_trivially_copy_constructible<T>
                     || !std::is_trivially_destructible<T>)
        {*/

      private:
        alignas(T) u8 m_Storage[sizeof(T)];
        bool m_HasValue = false;
    };
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX == 1
using Prism::Optional;
#endif
