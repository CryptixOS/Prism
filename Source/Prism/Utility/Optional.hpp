/*
 * Created by v1tr10l7 on 04.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Concepts.hpp>
#include <Prism/Core/Error.hpp>
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
    class PM_NODISCARD Optional<T>
    {
        static_assert(!IsLValueReferenceV<T> && !IsRValueReferenceV<T>);

      public:
        using ValueType      = T;

        constexpr Optional() = default;
        template <SameAs<NullOptType> V>
        constexpr Optional(V)
        {
        }
        constexpr Optional(const Optional& other)
            requires(!IsCopyConstructibleV<T>)
        = delete;
        constexpr Optional(const Optional& other) = default;
        constexpr Optional(const Optional& other)
            requires(!IsTriviallyCopyConstructibleV<T>)
            : m_HasValue(other.m_HasValue)
        {
            if (other.HasValue())
                ConstructAt(reinterpret_cast<T*>(m_Storage), T(other.Value()));
        }
        template <typename U>
            requires(IsConstructibleV<T, const U&>
                     && !IsSpecializationOf<T, Optional>::Value
                     && !IsSpecializationOf<U, Optional>::Value
                     && !IsLValueReferenceV<U>)
        constexpr explicit Optional(const Optional<U>& other)
            : m_HasValue(other.m_HasValue)
        {
            if (other.HasValue())
                ConstructAt<RemoveConstType<T>>(&m_Storage, other.Value());
        }

        constexpr Optional(Optional&& other)
            requires(!IsMoveConstructibleV<T>)
        = delete;
        constexpr Optional(Optional&& other)
            : m_HasValue(other.m_HasValue)
        {
            if (other.HasValue()) ConstructAt(&m_Storage, other.ReleaseValue());
        }
        template <typename U>
            requires(IsConstructibleV<T, U &&>
                     && !IsSpecializationOf<T, Optional>::Value
                     && !IsSpecializationOf<U, Optional>::Value
                     && !IsLValueReferenceV<U>)
        constexpr explicit Optional(Optional<U>&& other)
            : m_HasValue(other.m_HasValue)
        {
            if (other.HasValue())
                ConstructAt<RemoveConstType<T>>(&m_Storage,
                                                other.ReleaseValue());
        }
        template <typename U = T>
            requires(!IsSameV<NullOptType, RemoveCvRefType<U>>)
        constexpr explicit(!IsConvertible<U&&, T>::Value) Optional(U&& value)
            requires(!IsSameV<RemoveCvRefType<U>, Optional<T>>
                     && IsConstructibleV<T, U &&>)
            : m_HasValue(true)
        {
            ConstructAt<RemoveConstType<T>>(&m_Storage, Forward<U>(value));
        }

        constexpr ~Optional()
            requires(!IsDestructibleV<T>)
        = delete;
        constexpr ~Optional() = default;
        constexpr ~Optional()
            requires(!IsTriviallyDestructibleV<T>)
        {
            Reset();
        }

        template <SameAs<NullOptType> V>
        Optional& operator=(V)
        {
            Reset();
            return *this;
        }

        Optional& operator=(const Optional& other)
            requires(!IsCopyConstructibleV<T> || !IsDestructibleV<T>)
        = delete;
        Optional&           operator=(const Optional& other) = default;

        constexpr Optional& operator=(Optional const& other)
            requires(!IsTriviallyCopyConstructibleV<T>
                     || !IsTriviallyDestructibleV<T>)
        {
            if (this != &other)
            {
                Reset();
                m_HasValue = other.m_HasValue;
                if (other.HasValue())
                    ConstructAt<RemoveConstType<T>>(&m_Storage, other.Value());
            }
            return *this;
        }

        Optional& operator=(Optional&& other)
            requires(!IsMoveConstructibleV<T> || !IsDestructibleV<T>)
        = delete;
        constexpr Optional& operator=(Optional&& other)
        {
            if (this != &other)
            {
                Reset();
                m_HasValue = other.m_HasValue;
                if (other.HasValue())
                    ConstructAt<RemoveConstType<T>>(&m_Storage,
                                                    other.ReleaseValue());
            }
            return *this;
        }
        PM_ALWAYS_INLINE constexpr Optional& operator=(Optional&& other)
            requires(IsMoveAssignableV<T> && IsMoveConstructibleV<T>
                     && (!IsTriviallyMoveAssignableV<T>
                         || !IsTriviallyMoveConstructibleV<T>
                         || !IsTriviallyDestructibleV<T>))
        {
            if (this == &other) return *this;

            Reset();
            if (other.HasValue())
            {
                if (HasValue()) Value() = other.ReleaseValue();
                else
                {
                    ConstructAt<RemoveConstType<T>>(&m_Storage,
                                                    other.ReleaseValue());
                    m_HasValue = true;
                }

                return *this;
            }

            Value().~T();
            m_HasValue = false;
            return *this;
        }

        constexpr      operator bool() const PM_NOEXCEPT { return HasValue(); }
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
            ConstructAt(reinterpret_cast<T*>(m_Storage),
                        T(Forward<Args>(args)...));
        }

        constexpr bool                   HasValue() const { return m_HasValue; }

        PM_NODISCARD PM_ALWAYS_INLINE T& Value() &
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
        constexpr T ValueOr(const T& fallback) const&
        {
            if (m_HasValue) return Value();
            return fallback;
        }
        constexpr T ReleaseValue()
        {
            assert(m_HasValue);
            T releasedValue = Move(Value());

            Value().~T();
            m_HasValue = false;

            return releasedValue;
        }

        constexpr const T& operator*() const { return Value(); }
        constexpr T&       operator*() { return Value(); }

        constexpr const T* operator->() const { return &Value(); }
        constexpr T*       operator->() { return &Value(); }

      private:
        union
        {
            struct
            {
                alignas(T) u8 _[sizeof(T)];
            } m_Null{};
            RemoveConstType<T> m_Storage;
        };
        bool m_HasValue = false;
    };

    template <typename T>
        requires(IsLValueReferenceV<T>)
    class PM_NODISCARD Optional<T>
    {
        template <typename>
        friend class Optional;

        template <typename U>
        constexpr static bool OptionalCompatible
            = IsSameV<RemoveReferenceType<T>,
                      RemoveReferenceType<AddConstType<U>>>
           && (IsBaseOfV<RemoveCvRefType<T>, RemoveCvRefType<U>>
               || IsSameV<RemoveCvRefType<T>, RemoveCvRefType<U>>);

      public:
        using ValueType                       = T;

        PM_ALWAYS_INLINE constexpr Optional() = default;

        template <SameAs<NullOptType> V>
        PM_ALWAYS_INLINE constexpr Optional(V)
        {
        }

        template <SameAs<NullOptType> V>
        PM_ALWAYS_INLINE constexpr Optional& operator=(V)
        {
            Clear();
            return *this;
        }

        template <typename U = T>
        PM_ALWAYS_INLINE constexpr Optional(U& value)
            requires(OptionalCompatible<U&>)
            : m_Pointer(&value)
        {
        }

        PM_ALWAYS_INLINE constexpr Optional(RemoveReferenceType<T>& value)
            : m_Pointer(&value)
        {
        }

        PM_ALWAYS_INLINE constexpr Optional(Optional const& other) = default;

        PM_ALWAYS_INLINE constexpr Optional(Optional&& other)
            : m_Pointer(other.m_Pointer)
        {
            other.m_Pointer = nullptr;
        }

        template <typename U>
        PM_ALWAYS_INLINE constexpr Optional(Optional<U> const& other)
            requires(OptionalCompatible<U>)
            : m_Pointer(other.m_Pointer)
        {
        }

        template <typename U>
        PM_ALWAYS_INLINE constexpr Optional(Optional<U>&& other)
            requires(OptionalCompatible<U>)
            : m_Pointer(other.m_Pointer)
        {
            other.m_Pointer = nullptr;
        }

        PM_ALWAYS_INLINE constexpr Optional& operator=(Optional const& other)
            = default;
        PM_ALWAYS_INLINE constexpr Optional& operator=(Optional&& other)
        {
            m_Pointer       = other.m_Pointer;
            other.m_Pointer = nullptr;
            return *this;
        }

        template <typename U>
        PM_ALWAYS_INLINE constexpr Optional& operator=(Optional<U> const& other)
            requires(OptionalCompatible<U>)
        {
            m_Pointer = other.m_Pointer;
            return *this;
        }

        template <typename U>
        PM_ALWAYS_INLINE constexpr Optional& operator=(Optional<U>&& other)
            requires(OptionalCompatible<U>)
        {
            m_Pointer       = other.m_Pointer;
            other.m_Pointer = nullptr;
            return *this;
        }

        template <typename U>
            requires(!IsSameV<NullOptType, RemoveCvRefType<U>>)
        PM_ALWAYS_INLINE constexpr Optional& operator=(U&& value)
            requires(OptionalCompatible<U> && IsLValueReferenceV<U>)
        {
            m_Pointer = &value;
            return *this;
        }

        PM_ALWAYS_INLINE constexpr void Clear() { m_Pointer = nullptr; }

        PM_NODISCARD PM_ALWAYS_INLINE constexpr bool HasValue() const
        {
            return m_Pointer != nullptr;
        }

        PM_NODISCARD PM_ALWAYS_INLINE constexpr T Value()
        {
            assert(m_Pointer);
            return *m_Pointer;
        }

        PM_NODISCARD PM_ALWAYS_INLINE constexpr AddConstType<T> Value() const
        {
            assert(m_Pointer);
            return *m_Pointer;
        }

        template <typename U>
            requires(IsBaseOfV<RemoveCvRefType<T>, U>)
        PM_NODISCARD PM_ALWAYS_INLINE constexpr AddConstType<T>
                     ValueOr(U& fallback) const
        {
            if (m_Pointer) return Value();
            return fallback;
        }

        PM_NODISCARD PM_ALWAYS_INLINE constexpr RemoveCvRefType<T>
                     value_or(RemoveCvRefType<T> fallback) const
        {
            if (m_Pointer) return Value();
            return fallback;
        }

        PM_NODISCARD PM_ALWAYS_INLINE constexpr T ReleaseValue()
        {
            return *Exchange(m_Pointer, nullptr);
        }

        template <typename U>
        PM_ALWAYS_INLINE constexpr bool
        operator==(Optional<U> const& other) const
        {
            return HasValue() == other.HasValue()
                && (!HasValue() || Value() == other.Value());
        }

        template <typename U>
        PM_ALWAYS_INLINE constexpr bool operator==(U const& other) const
        {
            return HasValue() && Value() == other;
        }

        PM_ALWAYS_INLINE constexpr AddConstType<T> operator*() const
        {
            return Value();
        }
        PM_ALWAYS_INLINE constexpr T operator*() { return Value(); }

        PM_ALWAYS_INLINE constexpr AddPointerType<
            AddConstType<RemoveReferenceType<T>>>
        operator->() const
        {
            return &Value();
        }
        PM_ALWAYS_INLINE constexpr AddPointerType<RemoveReferenceType<T>>
        operator->()
        {
            return &Value();
        }

        PM_ALWAYS_INLINE constexpr explicit
        operator Optional<RemoveCvRefType<T>>() const
        {
            if (HasValue()) return Optional<RemoveCvRefType<T>>(Value());
            return {};
        }
        PM_ALWAYS_INLINE constexpr Optional<RemoveCvRefType<T>> Copy() const
        {
            return static_cast<Optional<RemoveCvRefType<T>>>(*this);
        }

        template <typename Callback>
        PM_NODISCARD PM_ALWAYS_INLINE constexpr T
        ValueOrLazyEvaluated(Callback callback) const
        {
            if (!m_Pointer) return Value();
            return callback();
        }

        template <typename Callback>
        PM_NODISCARD PM_ALWAYS_INLINE constexpr Optional<T>
                     ValueOrLazyEvaluatedOptional(Callback callback) const
        {
            if (!m_Pointer) return Value();
            return callback();
        }

        template <typename Callback>
        PM_NODISCARD PM_ALWAYS_INLINE constexpr ErrorOr<T>
                     TryValueOrLazyEvaluated(Callback callback) const
        {
            if (!m_Pointer) return Value();
            return callback();
        }

        template <typename Callback>
        PM_NODISCARD PM_ALWAYS_INLINE constexpr ErrorOr<Optional<T>>
                     TryValueOrLazyEvaluatedOptional(Callback callback) const
        {
            if (!m_Pointer) return Value();
            return callback();
        }

      private:
        RemoveReferenceType<T>* m_Pointer{nullptr};
    };
}; // namespace Prism
#if PRISM_TARGET_CRYPTIX != 0
using Prism::NullOpt;
using Prism::Optional;
#endif
