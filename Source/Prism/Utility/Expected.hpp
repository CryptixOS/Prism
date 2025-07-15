/*
 * Created by v1tr10l7 on 07.12.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <cassert>
#include <memory>
#include <utility>

#include <Prism/Core/Core.hpp>

namespace Prism
{
    template <typename T, typename E>
    class Expected;

    template <typename E>
    class Unexpected;

    struct UnexpectedBase
    {
        explicit UnexpectedBase() = default;
    };

    inline constexpr UnexpectedBase unexpect{};

    namespace Detail
    {
        template <typename T>
        constexpr bool IsExpected = false;
        template <typename T, typename E>
        constexpr bool IsExpected<Expected<T, E>> = true;

        template <typename T>
        constexpr bool IsUnexpected = false;
        template <typename T>
        constexpr bool IsUnexpected<Unexpected<T>> = true;

        template <typename F, typename T>
        using Result = RemoveCvRefType<InvokeResultType<F&&, T&&>>;
        template <typename F, typename T>
        using ResultFrom = RemoveCvType<InvokeResultType<F&&, T&&>>;
        template <typename F>
        using Result0 = RemoveCvRefType<InvokeResultType<F&&>>;
        template <typename F>
        using Result0From = RemoveCvType<InvokeResultType<F&&>>;

        template <typename E>
        concept CanBeUnexpected
            = IsObjectV<E> && (!IsArrayV<E>) && (!IsUnexpected<E>)
           && (!IsConstV<E>) && (!IsVolatileV<E>);

        struct InPlaceInvocation
        {
        };
        struct UnexpectedInvocation
        {
        };
    } // namespace Detail

    template <typename E>
    class Unexpected
    {
        static_assert(Detail::CanBeUnexpected<E>);

      public:
        constexpr Unexpected(const Unexpected&) = default;
        constexpr Unexpected(Unexpected&&)      = default;

        template <typename Err = E>
            requires(!IsSameV<RemoveCvRefType<Err>, Unexpected>)
                 && (!IsSameV<RemoveCvRefType<Err>, InPlaceT>)
                 && IsConstructibleV<E, Err>
        constexpr explicit Unexpected(Err&& e) noexcept
            : m_Unexpected(Forward<Err>(e))
        {
        }

        template <typename... Args>
            requires IsConstructibleV<E, Args...>
        constexpr explicit Unexpected(InPlaceT, Args&&... args) noexcept
            : m_Unexpected(Forward<Args>(args)...)
        {
        }

        template <typename U, typename... Args>
            requires IsConstructibleV<E, std::initializer_list<U>&, Args...>
        constexpr explicit Unexpected(InPlaceT, std::initializer_list<U> list,
                                      Args&&... args) noexcept
            : m_Unexpected(list, Forward<Args>(args)...)
        {
        }

        constexpr Unexpected& operator=(const Unexpected&) = default;
        constexpr Unexpected& operator=(Unexpected&&)      = default;

        [[nodiscard]]
        constexpr const E& Error() const& noexcept
        {
            return m_Unexpected;
        }

        [[nodiscard]]
        constexpr E& Error() & noexcept
        {
            return m_Unexpected;
        }

        [[nodiscard]]
        constexpr const E&& Error() const&& noexcept
        {
            return Move(m_Unexpected);
        }

        [[nodiscard]]
        constexpr E&& Error() && noexcept
        {
            return Move(m_Unexpected);
        }

        [[nodiscard]]
        constexpr const E& error() const& noexcept
        {
            return m_Unexpected;
        }

        [[nodiscard]]
        constexpr E& error() & noexcept
        {
            return m_Unexpected;
        }

        [[nodiscard]]
        constexpr const E&& error() const&& noexcept
        {
            return Move(m_Unexpected);
        }

        [[nodiscard]]
        constexpr E&& error() && noexcept
        {
            return Move(m_Unexpected);
        }

        constexpr void Swap(Unexpected& other) noexcept
            requires IsSwappableV<E>
        {
            Swap(m_Unexpected, other.m_Unexpected);
        }

        template <typename Err>
        [[nodiscard]]
        friend constexpr bool operator==(const Unexpected&      lhs,
                                         const Unexpected<Err>& rhs)
        {
            return lhs.m_Unexpected == rhs.Error();
        }

        friend constexpr void
        swap(Unexpected& lhs, Unexpected& rhs) noexcept(noexcept(lhs.Swap(rhs)))
            requires IsSwappableV<E>
        {
            lhs.Swap(rhs);
        }

      private:
        E m_Unexpected;
    };

    template <typename E>
    Unexpected(E) -> Unexpected<E>;

    namespace Detail
    {
        template <typename T>
        struct Guard
        {
            constexpr explicit Guard(T& lhs)
                : m_Guarded(AddressOf(lhs))
                , m_Temp(Move(lhs))
            {
                std::destroy_at(m_Guarded);
            }

            constexpr ~Guard()
            {
                if (m_Guarded) [[unlikely]]
                    std::construct_at(m_Guarded, Move(m_Temp));
            }

            Guard(const Guard&)                   = delete;
            Guard&        operator=(const Guard&) = delete;

            constexpr T&& Release() noexcept
            {
                m_Guarded = nullptr;
                return Move(m_Temp);
            }

          private:
            T* m_Guarded;
            T  m_Temp;
        };

        template <typename T, typename U, typename V>
        constexpr void Reinitialize(T* newValue, U* oldValue, V&& arg) noexcept
        {
            std::destroy_at(oldValue);
            std::construct_at(newValue, Forward<V>(arg));
        }
    } // namespace Detail

    template <typename T, typename E>
    class Expected
    {
        static_assert(!IsReferenceV<T>);
        static_assert(!IsFunctionV<T>);
        static_assert(!IsSameV<RemoveCvType<T>, InPlaceT>);
        static_assert(!IsSameV<RemoveCvType<T>, UnexpectedBase>);
        static_assert(!Detail::IsUnexpected<RemoveCvType<T>>);
        static_assert(Detail::CanBeUnexpected<E>);

        template <typename U, typename Err, typename Un = Unexpected<E>>
        static constexpr bool IsCostructibleFromExpected
            = DisjunctionV<IsConstructible<T, Expected<U, Err>&>,
                           IsConstructible<T, Expected<U, Err>>,
                           IsConstructible<T, const Expected<U, Err>&>,
                           IsConstructible<T, const Expected<U, Err>>,
                           IsConvertible<Expected<U, Err>&, T>,
                           IsConvertible<Expected<U, Err>, T>,
                           IsConvertible<const Expected<U, Err>&, T>,
                           IsConvertible<const Expected<U, Err>, T>,
                           IsConstructible<Un, Expected<U, Err>&>,
                           IsConstructible<Un, Expected<U, Err>>,
                           IsConstructible<Un, const Expected<U, Err>&>,
                           IsConstructible<Un, const Expected<U, Err>>>;

        template <typename U, typename Err>
        constexpr static bool IsExplicitlyConstructible
            = !IsConvertible<U, T>::Value || !IsConvertible<Err, E>::Value;

        template <typename U>
        static constexpr bool SameValue = IsSameV<typename U::ValueType, T>;

        template <typename U>
        static constexpr bool SameError = IsSameV<typename U::ErrorType, E>;

      public:
        using ValueType      = T;
        using ErrorType      = E;
        using UnexpectedType = Unexpected<E>;

        template <typename U>
        using Rebind = Expected<U, ErrorType>;

        constexpr Expected() noexcept
            requires IsDefaultConstructibleV<T>
            : m_Value()
            , m_HasValue(true)
        {
        }

        Expected(const Expected&) = default;
        constexpr Expected(const Expected& lhs) noexcept
            requires IsCopyConstructibleV<T> && IsCopyConstructibleV<E>
                  && (!IsTriviallyCopyConstructibleV<T>
                      || !IsTriviallyCopyConstructibleV<E>)
            : m_HasValue(lhs.m_HasValue)
        {
            if (m_HasValue) std::construct_at(AddressOf(m_Value), lhs.m_Value);
            else std::construct_at(AddressOf(m_Unexpected), lhs.m_Unexpected);
        }

        Expected(Expected&&) = default;

        constexpr Expected(Expected&& lhs) noexcept
            requires IsMoveConstructibleV<T> && IsMoveConstructibleV<E>
                  && (!IsTriviallyMoveConstructibleV<T>
                      || !IsTriviallyMoveConstructibleV<E>)
            : m_HasValue(lhs.m_HasValue)
        {
            if (m_HasValue)
                std::construct_at(AddressOf(m_Value), Move(lhs).m_Value);
            else
                std::construct_at(AddressOf(m_Unexpected),
                                  Move(lhs).m_Unexpected);
        }

        template <typename U, typename UnexpectedResult>
            requires IsConstructibleV<T, const U&>
                  && IsConstructibleV<E, const UnexpectedResult&>
                  && (!IsCostructibleFromExpected<U, UnexpectedResult>)
        constexpr explicit Expected(
            const Expected<U, UnexpectedResult>& lhs) noexcept
            : m_HasValue(lhs.m_HasValue)
        {
            if (m_HasValue) std::construct_at(AddressOf(m_Value), lhs.m_Value);
            else std::construct_at(AddressOf(m_Unexpected), lhs.m_Unexpected);
        }
        template <typename U, typename = EnableIfType<IsConvertibleV<U, T>>>
        Expected(const Expected<U, E>& other)
            : m_HasValue(other.m_HasValue)
        {
            if (m_HasValue)
                std::construct_at(AddressOf(m_Value), other.m_Value);
            else std::construct_at(AddressOf(m_Unexpected), other.m_Unexpected);
        }

        template <typename U, typename UnexpectedResult>
            requires IsConstructibleV<T, U>
                  && IsConstructibleV<E, UnexpectedResult>
                  && (!IsCostructibleFromExpected<U, UnexpectedResult>)
        constexpr explicit Expected(
            Expected<U, UnexpectedResult>&& lhs) noexcept
            : m_HasValue(lhs.m_HasValue)
        {
            if (m_HasValue)
                std::construct_at(AddressOf(m_Value), Move(lhs).m_Value);
            else
                std::construct_at(AddressOf(m_Unexpected),
                                  Move(lhs).m_Unexpected);
        }
        template <typename U,
                  typename = std::enable_if_t<std::is_convertible_v<U, T>>>
        Expected(Expected<U, E>&& other)
            : m_HasValue(other.m_HasValue)
        {
            if (m_HasValue)
                std::construct_at(AddressOf(m_Value), Move(other).m_Value);
            else
                std::construct_at(AddressOf(m_Unexpected),
                                  Move(other).m_Unexpected);
        }

        template <typename U = T>
            requires(!IsSameV<RemoveCvRefType<U>, Expected>)
                     && (!IsSameV<RemoveCvRefType<U>, InPlaceT>)
                     && (!Detail::IsUnexpected<RemoveCvRefType<U>>)
                     && IsConstructibleV<T, U>
        constexpr explicit(!IsConvertibleV<U, T>) Expected(U&& value) noexcept
            : m_Value(Forward<U>(value))
            , m_HasValue(true)
        {
        }

        template <typename UnexpectedResult = E>
            requires IsConstructibleV<E, const UnexpectedResult&>
        constexpr explicit(!IsConvertibleV<const UnexpectedResult&, E>)
            Expected(const Unexpected<UnexpectedResult>& unexpect) noexcept
            : m_Unexpected(unexpect.Error())
            , m_HasValue(false)
        {
        }

        template <typename UnexpectedResult = E>
            requires IsConstructibleV<E, UnexpectedResult>
        constexpr explicit(!IsConvertibleV<UnexpectedResult, E>)
            Expected(Unexpected<UnexpectedResult>&& unexpect) noexcept
            : m_Unexpected(Move(unexpect).Error())
            , m_HasValue(false)
        {
        }

        template <typename... Args>
            requires IsConstructibleV<T, Args...>
        constexpr explicit Expected(InPlaceT, Args&&... args) noexcept
            : m_Value(Forward<Args>(args)...)
            , m_HasValue(true)
        {
        }

        template <typename U, typename... Args>
            requires IsConstructibleV<T, std::initializer_list<U>&, Args...>
        constexpr explicit Expected(InPlaceT, std::initializer_list<U> ilist,
                                    Args&&... args) noexcept
            : m_Value(ilist, Forward<Args>(args)...)
            , m_HasValue(true)
        {
        }

        template <typename... Args>
            requires IsConstructibleV<E, Args...>
        constexpr explicit Expected(UnexpectedBase, Args&&... args) noexcept
            : m_Unexpected(Forward<Args>(args)...)
            , m_HasValue(false)
        {
        }

        template <typename U, typename... Args>
            requires IsConstructibleV<E, std::initializer_list<U>&, Args...>
        constexpr explicit Expected(UnexpectedBase,
                                    std::initializer_list<U> ilist,
                                    Args&&... args) noexcept
            : m_Unexpected(ilist, Forward<Args>(args)...)
            , m_HasValue(false)
        {
        }

        constexpr ~Expected() = default;

        constexpr ~Expected()
            requires(!std::is_trivially_destructible_v<T>)
                 || (!std::is_trivially_destructible_v<E>)
        {
            if (m_HasValue) std::destroy_at(AddressOf(m_Value));
            else std::destroy_at(AddressOf(m_Unexpected));
        }

        Expected&           operator=(const Expected&) = delete;
        constexpr Expected& operator=(const Expected& lhs) noexcept
            requires IsCopyAssignableV<T> && IsCopyConstructibleV<T>
                  && IsCopyAssignableV<E> && IsCopyConstructibleV<E>
        {
            if (lhs.m_HasValue) this->AssignValue(lhs.m_Value);
            else this->AssignUnexpected(lhs.m_Unexpected);
            return *this;
        }

        constexpr Expected& operator=(Expected&& lhs) noexcept
            requires IsMoveAssignableV<T> && IsMoveConstructibleV<T>
                  && IsMoveAssignableV<E> && IsMoveConstructibleV<E>
        {
            if (lhs.m_HasValue) AssignValue(Move(lhs.m_Value));
            else AssignUnexpected(Move(lhs.m_Unexpected));
            return *this;
        }

        template <typename U = T>
            requires(!IsSameV<Expected, RemoveCvRefType<U>>)
                 && (!Detail::IsUnexpected<RemoveCvRefType<U>>)
                 && IsConstructibleV<T, U> && IsAssignableV<T&, U>
        constexpr Expected& operator=(U&& value)
        {
            AssignValue(Forward<U>(value));
            return *this;
        }

        template <typename U,
                  typename = std::enable_if_t<std::is_convertible_v<U, T>>>
        Expected& operator=(const Expected<U, E>& other)
        {
            if (other.m_HasValue) AssignValue(Move(other.m_Value));
            else AssignUnexpected(Move(other.m_Unexpected));
            return *this;
        }

        template <typename UnexpectedResult>
            requires IsConstructibleV<E, const UnexpectedResult&>
                  && IsAssignableV<E&, const UnexpectedResult&>
        constexpr Expected& operator=(const Unexpected<UnexpectedResult>& e)
        {
            AssignUnexpected(e.Error());
            return *this;
        }

        template <typename UnexpectedResult>
            requires IsConstructibleV<E, UnexpectedResult>
                  && IsAssignableV<E&, UnexpectedResult>
        constexpr Expected& operator=(Unexpected<UnexpectedResult>&& e)
        {
            AssignUnexpected(Move(e).Error());
            return *this;
        }

        template <typename... Args>
        constexpr T& Emplace(Args&&... args) noexcept
        {
            if (m_HasValue) std::destroy_at(AddressOf(m_Value));
            else
            {
                std::destroy_at(AddressOf(m_Unexpected));
                m_HasValue = true;
            }
            std::construct_at(AddressOf(m_Value), Forward<Args>(args)...);
            return m_Value;
        }

        template <typename U, typename... Args>
        constexpr T& Emplace(std::initializer_list<U> ilist,
                             Args&&... args) noexcept
        {
            if (m_HasValue) std::destroy_at(AddressOf(m_Value));
            else
            {
                std::destroy_at(AddressOf(m_Unexpected));
                m_HasValue = true;
            }
            std::construct_at(AddressOf(m_Value), ilist,
                              Forward<Args>(args)...);
            return m_Value;
        }

        constexpr void Swap(Expected& lhs) noexcept
            requires IsSwappableV<T> && IsSwappableV<E>
                  && IsMoveConstructibleV<T> && IsMoveConstructibleV<E>
        {
            if (m_HasValue)
            {
                if (lhs.m_HasValue) Swap(m_Value, lhs.m_Value);
                else SwapValueWithUnexpected(lhs);
            }
            else
            {
                if (lhs.m_HasValue) lhs.SwapValueWithUnexpected(*this);
                else Swap(m_Unexpected, lhs.m_Unexpected);
            }
        }

        // observers

        [[nodiscard]]
        constexpr const T* operator->() const noexcept
        {
            assert(m_HasValue);
            return AddressOf(m_Value);
        }

        [[nodiscard]]
        constexpr T* operator->() noexcept
        {
            assert(m_HasValue);
            return AddressOf(m_Value);
        }

        [[nodiscard]]
        constexpr const T& operator*() const& noexcept
        {
            assert(m_HasValue);
            return m_Value;
        }

        [[nodiscard]]
        constexpr T& operator*() & noexcept
        {
            assert(m_HasValue);
            return m_Value;
        }

        [[nodiscard]]
        constexpr const T&& operator*() const&& noexcept
        {
            assert(m_HasValue);
            return Move(m_Value);
        }

        [[nodiscard]]
        constexpr T&& operator*() && noexcept
        {
            assert(m_HasValue);
            return Move(m_Value);
        }

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return m_HasValue;
        }

        [[nodiscard]]
        constexpr bool HasValue() const noexcept
        {
            return m_HasValue;
        }
        [[nodiscard]]
        constexpr bool has_value() const noexcept
        {
            return m_HasValue;
        }

        constexpr const T& value() const&
        {
            if (m_HasValue) [[likely]]
                return m_Value;
            assert(false);
        }

        constexpr T& value() &
        {
            if (m_HasValue) [[likely]]
                return m_Value;
            assert(false);
        }

        constexpr const T&& value() const&&
        {
            if (m_HasValue) [[likely]]
                return Move(m_Value);
            assert(false);
        }

        constexpr T&& value() &&
        {
            if (m_HasValue) [[likely]]
                return Move(m_Value);
            assert(false);
        }

        constexpr const E& Error() const& noexcept
        {
            assert(!m_HasValue);
            return m_Unexpected;
        }

        constexpr E& Error() & noexcept
        {
            assert(!m_HasValue);
            return m_Unexpected;
        }

        constexpr const E&& Error() const&& noexcept
        {
            assert(!m_HasValue);
            return Move(m_Unexpected);
        }

        constexpr E&& Error() && noexcept
        {
            assert(!m_HasValue);
            return Move(m_Unexpected);
        }

        constexpr const E& error() const& noexcept
        {
            assert(!m_HasValue);
            return m_Unexpected;
        }

        constexpr E& error() & noexcept
        {
            assert(!m_HasValue);
            return m_Unexpected;
        }

        constexpr const E&& error() const&& noexcept
        {
            assert(!m_HasValue);
            return Move(m_Unexpected);
        }

        constexpr E&& error() && noexcept
        {
            assert(!m_HasValue);
            return Move(m_Unexpected);
        }

        template <typename U>
        constexpr T ValueOr(U&& value) const& noexcept
        {
            static_assert(IsCopyConstructibleV<T>);
            static_assert(IsConvertibleV<U, T>);

            if (m_HasValue) return m_Value;
            return static_cast<T>(Forward<U>(value));
        }

        template <typename U>
        constexpr T ValueOr(U&& value) && noexcept
        {
            static_assert(IsMoveConstructibleV<T>);
            static_assert(IsConvertibleV<U, T>);

            if (m_HasValue) return Move(m_Value);
            return static_cast<T>(Forward<U>(value));
        }

        template <typename U>
        constexpr T value_or(U&& value) const& noexcept
        {
            static_assert(IsCopyConstructibleV<T>);
            static_assert(IsConvertibleV<U, T>);

            if (m_HasValue) return m_Value;
            return static_cast<T>(Forward<U>(value));
        }

        template <typename U>
        constexpr T value_or(U&& value) && noexcept
        {
            static_assert(IsMoveConstructibleV<T>);
            static_assert(IsConvertibleV<U, T>);

            if (m_HasValue) return Move(m_Value);
            return static_cast<T>(Forward<U>(value));
        }

        template <typename UnexpectedResult = E>
        constexpr E ErrorOr(UnexpectedResult&& e) const&
        {
            static_assert(IsCopyConstructibleV<E>);
            static_assert(IsConvertibleV<UnexpectedResult, E>);

            if (m_HasValue) return Forward<UnexpectedResult>(e);
            return m_Unexpected;
        }

        template <typename UnexpectedResult = E>
        constexpr E ErrorOr(UnexpectedResult&& e) &&
        {
            static_assert(IsMoveConstructibleV<E>);
            static_assert(IsConvertibleV<UnexpectedResult, E>);

            if (m_HasValue) return Forward<UnexpectedResult>(e);
            return Move(m_Unexpected);
        }

        // monadic operations

        template <typename F>
            requires IsConstructibleV<E, E&>
        constexpr auto AndThen(F&& f) &
        {
            using U = Detail::Result<F, T&>;
            static_assert(Detail::IsExpected<U>,
                          "the function passed to std::Expected<T, E>::AndThen "
                          "must return a std::Expected");
            static_assert(
                IsSameV<typename U::ErrorType, E>,
                "the function passed to std::Expected<T, E>::AndThen "
                "must return a std::Expected with the same ErrorType");

            if (HasValue()) return Invoke(Forward<F>(f), m_Value);
            else return U(unexpect, m_Unexpected);
        }

        template <typename F>
            requires IsConstructibleV<E, const E&>
        constexpr auto AndThen(F&& f) const&
        {
            using U = Detail::Result<F, const T&>;
            static_assert(Detail::IsExpected<U>,
                          "the function passed to std::Expected<T, E>::AndThen "
                          "must return a std::Expected");
            static_assert(
                IsSameV<typename U::ErrorType, E>,
                "the function passed to std::Expected<T, E>::AndThen "
                "must return a std::Expected with the same ErrorType");

            if (HasValue()) return Invoke(Forward<F>(f), m_Value);
            else return U(unexpect, m_Unexpected);
        }

        template <typename F>
            requires IsConstructibleV<E, E>
        constexpr auto AndThen(F&& f) &&
        {
            using U = Detail::Result<F, T&&>;
            static_assert(Detail::IsExpected<U>,
                          "the function passed to std::Expected<T, E>::AndThen "
                          "must return a std::Expected");
            static_assert(
                IsSameV<typename U::ErrorType, E>,
                "the function passed to std::Expected<T, E>::AndThen "
                "must return a std::Expected with the same ErrorType");

            if (HasValue()) return Invoke(Forward<F>(f), Move(m_Value));
            else return U(unexpect, Move(m_Unexpected));
        }

        template <typename F>
            requires IsConstructibleV<E, const E>
        constexpr auto AndThen(F&& f) const&&
        {
            using U = Detail::Result<F, const T&&>;
            static_assert(Detail::IsExpected<U>,
                          "the function passed to std::Expected<T, E>::AndThen "
                          "must return a std::Expected");
            static_assert(
                IsSameV<typename U::ErrorType, E>,
                "the function passed to std::Expected<T, E>::AndThen "
                "must return a std::Expected with the same ErrorType");

            if (HasValue()) return Invoke(Forward<F>(f), Move(m_Value));
            else return U(unexpect, Move(m_Unexpected));
        }

        template <typename F>
            requires IsConstructibleV<T, T&>
        constexpr auto OrElse(F&& f) &
        {
            using UnexpectedResult = Detail::Result<F, E&>;
            static_assert(Detail::IsExpected<UnexpectedResult>,
                          "the function passed to std::Expected<T, E>::OrElse "
                          "must return a std::Expected");
            static_assert(
                IsSameV<typename UnexpectedResult::ValueType, T>,
                "the function passed to std::Expected<T, E>::OrElse "
                "must return a std::Expected with the same ValueType");

            if (HasValue()) return UnexpectedResult(std::in_place, m_Value);
            else return Invoke(Forward<F>(f), m_Unexpected);
        }

        template <typename F>
            requires IsConstructibleV<T, const T&>
        constexpr auto OrElse(F&& f) const&
        {
            using UnexpectedResult = Detail::Result<F, const E&>;
            static_assert(Detail::IsExpected<UnexpectedResult>,
                          "the function passed to std::Expected<T, E>::OrElse "
                          "must return a std::Expected");
            static_assert(
                IsSameV<typename UnexpectedResult::ValueType, T>,
                "the function passed to std::Expected<T, E>::OrElse "
                "must return a std::Expected with the same ValueType");

            if (HasValue()) return UnexpectedResult(std::in_place, m_Value);
            else return Invoke(Forward<F>(f), m_Unexpected);
        }

        template <typename F>
            requires IsConstructibleV<T, T>
        constexpr auto OrElse(F&& f) &&
        {
            using UnexpectedResult = Detail::Result<F, E&&>;
            static_assert(Detail::IsExpected<UnexpectedResult>,
                          "the function passed to std::Expected<T, E>::OrElse "
                          "must return a std::Expected");
            static_assert(
                IsSameV<typename UnexpectedResult::ValueType, T>,
                "the function passed to std::Expected<T, E>::OrElse "
                "must return a std::Expected with the same ValueType");

            if (HasValue())
                return UnexpectedResult(std::in_place, Move(m_Value));
            else return Invoke(Forward<F>(f), Move(m_Unexpected));
        }

        template <typename F>
            requires IsConstructibleV<T, const T>
        constexpr auto OrElse(F&& f) const&&
        {
            using UnexpectedResult = Detail::Result<F, const E&&>;
            static_assert(Detail::IsExpected<UnexpectedResult>,
                          "the function passed to std::Expected<T, E>::OrElse "
                          "must return a std::Expected");
            static_assert(
                IsSameV<typename UnexpectedResult::ValueType, T>,
                "the function passed to std::Expected<T, E>::OrElse "
                "must return a std::Expected with the same ValueType");

            if (HasValue())
                return UnexpectedResult(std::in_place, Move(m_Value));
            else return Invoke(Forward<F>(f), Move(m_Unexpected));
        }

        template <typename F>
            requires IsConstructibleV<E, E&>
        constexpr auto Transform(F&& f) &
        {
            using U          = Detail::ResultFrom<F, T&>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(InPlaceInvocation{}, [&]()
                                  { return Invoke(Forward<F>(f), m_Value); });
            else return ResultType(unexpect, m_Unexpected);
        }

        template <typename F>
            requires IsConstructibleV<E, const E&>
        constexpr auto Transform(F&& f) const&
        {
            using U          = Detail::ResultFrom<F, const T&>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(InPlaceInvocation{}, [&]()
                                  { return Invoke(Forward<F>(f), m_Value); });
            else return ResultType(unexpect, m_Unexpected);
        }

        template <typename F>
            requires IsConstructibleV<E, E>
        constexpr auto Transform(F&& f) &&
        {
            using U          = Detail::ResultFrom<F, T>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(
                    InPlaceInvocation{},
                    [&]() { return Invoke(Forward<F>(f), Move(m_Value)); });
            else return ResultType(unexpect, Move(m_Unexpected));
        }

        template <typename F>
            requires IsConstructibleV<E, const E>
        constexpr auto Transform(F&& f) const&&
        {
            using U          = Detail::ResultFrom<F, const T>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(
                    InPlaceInvocation{},
                    [&]() { return Invoke(Forward<F>(f), Move(m_Value)); });
            else return ResultType(unexpect, Move(m_Unexpected));
        }

        template <typename F>
            requires IsConstructibleV<T, T&>
        constexpr auto TransformError(F&& f) &
        {
            using UnexpectedResult = Detail::ResultFrom<F, E&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue()) return ResultType(std::in_place, m_Value);
            else
                return ResultType(
                    UnexpectedInvocation{},
                    [&]() { return Invoke(Forward<F>(f), m_Unexpected); });
        }

        template <typename F>
            requires IsConstructibleV<T, const T&>
        constexpr auto TransformError(F&& f) const&
        {
            using UnexpectedResult = Detail::ResultFrom<F, const E&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue()) return ResultType(std::in_place, m_Value);
            else
                return ResultType(
                    UnexpectedInvocation{},
                    [&]() { return Invoke(Forward<F>(f), m_Unexpected); });
        }

        template <typename F>
            requires IsConstructibleV<T, T>
        constexpr auto TransformError(F&& f) &&
        {
            using UnexpectedResult = Detail::ResultFrom<F, E&&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue()) return ResultType(std::in_place, Move(m_Value));
            else
                return ResultType(
                    UnexpectedInvocation{}, [&]()
                    { return Invoke(Forward<F>(f), Move(m_Unexpected)); });
        }

        template <typename F>
            requires IsConstructibleV<T, const T>
        constexpr auto TransformError(F&& f) const&&
        {
            using UnexpectedResult = Detail::ResultFrom<F, const E&&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue()) return ResultType(std::in_place, Move(m_Value));
            else
                return ResultType(
                    UnexpectedInvocation{}, [&]()
                    { return Invoke(Forward<F>(f), Move(m_Unexpected)); });
        }

        template <typename U, typename E2>
            requires(!IsVoidV<U>)
        friend constexpr bool
        operator==(const Expected& lhs, const Expected<U, E2>& rhs) noexcept(
            noexcept(bool(*lhs == *rhs))
            && noexcept(bool(lhs.Error() == rhs.Error())))
        {
            if (lhs.HasValue()) return rhs.HasValue() && bool(*lhs == *rhs);
            else return !rhs.HasValue() && bool(lhs.Error() == rhs.Error());
        }

        template <typename U>
        friend constexpr bool
        operator==(const Expected& lhs,
                   const U& value) noexcept(noexcept(bool(*lhs == value)))
        {
            return lhs.HasValue() && bool(*lhs == value);
        }

        template <typename E2>
        friend constexpr bool
        operator==(const Expected& lhs, const Unexpected<E2>& e) noexcept(
            noexcept(bool(lhs.Error() == e.Error())))
        {
            return !lhs.HasValue() && bool(lhs.Error() == e.Error());
        }

        friend constexpr void
        Swap(Expected& lhs, Expected& rhs) noexcept(noexcept(lhs.Swap(rhs)))
            requires requires { lhs.Swap(rhs); }
        {
            lhs.Swap(rhs);
        }

      private:
        template <typename, typename>
        friend class Expected;

        template <typename V>
        constexpr void AssignValue(V&& value)
        {
            if (m_HasValue) m_Value = Forward<V>(value);
            else
            {
                Detail::Reinitialize(AddressOf(m_Value),
                                     AddressOf(m_Unexpected),
                                     Forward<V>(value));
                m_HasValue = true;
            }
        }

        template <typename V>
        constexpr void AssignUnexpected(V&& value)
        {
            if (m_HasValue)
            {
                Detail::Reinitialize(AddressOf(m_Unexpected),
                                     AddressOf(m_Value), Forward<V>(value));
                m_HasValue = false;
                return;
            }
            m_Unexpected = Forward<V>(value);
        }

        constexpr void SwapValueWithUnexpected(Expected& rhs) noexcept
        {
            if constexpr (IsMoveConstructibleV<E>)
            {
                Detail::Guard<E> __guard(rhs.m_Unexpected);
                std::construct_at(AddressOf(rhs.m_Value), Move(m_Value));
                rhs.m_HasValue = true;
                std::destroy_at(AddressOf(m_Value));
                std::construct_at(AddressOf(m_Unexpected), __guard.release());
                m_HasValue = false;
            }
            else
            {
                Detail::Guard<T> guard(m_Value);
                std::construct_at(AddressOf(m_Unexpected),
                                  Move(rhs.m_Unexpected));
                m_HasValue = false;
                std::destroy_at(AddressOf(rhs.m_Unexpected));
                std::construct_at(AddressOf(rhs.m_Value), guard.release());
                rhs.m_HasValue = true;
            }
        }

        using InPlaceInvocation    = Detail::InPlaceInvocation;
        using UnexpectedInvocation = Detail::UnexpectedInvocation;

        template <typename F>
        explicit constexpr Expected(InPlaceInvocation, F&& f)
            : m_Value(Forward<F>(f)())
            , m_HasValue(true)
        {
        }

        template <typename F>
        explicit constexpr Expected(UnexpectedInvocation, F&& f)
            : m_Unexpected(Forward<F>(f)())
            , m_HasValue(false)
        {
        }

        union
        {
            T m_Value;
            E m_Unexpected;
        };

        bool m_HasValue;
    };

    template <typename T, typename E>
        requires IsVoidV<T>
    class Expected<T, E>
    {
        static_assert(Detail::CanBeUnexpected<E>);

        template <typename U, typename Err, typename Unex = Unexpected<E>>
        static constexpr bool IsCostructibleFromExpected
            = DisjunctionV<IsConstructible<Unex, Expected<U, Err>&>,
                           IsConstructible<Unex, Expected<U, Err>>,
                           IsConstructible<Unex, const Expected<U, Err>&>,
                           IsConstructible<Unex, const Expected<U, Err>>>;

        template <typename U>
        static constexpr bool SameValue = IsSameV<typename U::ValueType, T>;

        template <typename U>
        static constexpr bool SameError = IsSameV<typename U::ErrorType, E>;

      public:
        using ValueType      = T;
        using ErrorType      = E;
        using UnexpectedType = Unexpected<E>;

        template <typename U>
        using Rebind = Expected<U, ErrorType>;

        constexpr Expected() noexcept
            : m_Void()
            , m_HasValue(true)
        {
        }

        Expected(const Expected&) = default;

        constexpr Expected(const Expected& lhs) noexcept
            requires IsCopyConstructibleV<E>
                      && (!IsTriviallyCopyConstructibleV<E>)
            : m_Void()
            , m_HasValue(lhs.m_HasValue)
        {
            if (!m_HasValue)
                std::construct_at(AddressOf(m_Unexpected), lhs.m_Unexpected);
        }

        Expected(Expected&&) = default;

        constexpr Expected(Expected&& lhs) noexcept
            requires IsMoveConstructibleV<E>
                      && (!IsTriviallyMoveConstructibleV<E>)
            : m_Void()
            , m_HasValue(lhs.m_HasValue)
        {
            if (!m_HasValue)
                std::construct_at(AddressOf(m_Unexpected),
                                  Move(lhs).m_Unexpected);
        }

        template <typename U, typename UnexpectedResult>
            requires IsVoidV<U> && IsConstructibleV<E, const UnexpectedResult&>
                      && (!IsCostructibleFromExpected<U, UnexpectedResult>)
        constexpr explicit(!IsConvertibleV<const UnexpectedResult&, E>)
            Expected(const Expected<U, UnexpectedResult>& lhs) noexcept
            : m_Void()
            , m_HasValue(lhs.m_HasValue)
        {
            if (!m_HasValue)
                std::construct_at(AddressOf(m_Unexpected), lhs.m_Unexpected);
        }

        template <typename U, typename UnexpectedResult>
            requires IsVoidV<U> && IsConstructibleV<E, UnexpectedResult>
                      && (!IsCostructibleFromExpected<U, UnexpectedResult>)
        constexpr explicit(!IsConvertibleV<UnexpectedResult, E>)
            Expected(Expected<U, UnexpectedResult>&& lhs) noexcept
            : m_Void()
            , m_HasValue(lhs.m_HasValue)
        {
            if (!m_HasValue)
                std::construct_at(AddressOf(m_Unexpected),
                                  Move(lhs).m_Unexpected);
        }

        template <typename UnexpectedResult = E>
            requires IsConstructibleV<E, const UnexpectedResult&>
        constexpr explicit(!IsConvertibleV<const UnexpectedResult&, E>)
            Expected(const Unexpected<UnexpectedResult>& unexpect) noexcept
            : m_Unexpected(unexpect.Error())
            , m_HasValue(false)
        {
        }

        template <typename UnexpectedResult = E>
            requires IsConstructibleV<E, UnexpectedResult>
        constexpr explicit(!IsConvertibleV<UnexpectedResult, E>)
            Expected(Unexpected<UnexpectedResult>&& unexpect) noexcept
            : m_Unexpected(Move(unexpect).Error())
            , m_HasValue(false)
        {
        }

        constexpr explicit Expected(InPlaceT) noexcept
            : Expected()
        {
        }

        template <typename... Args>
            requires IsConstructibleV<E, Args...>
        constexpr explicit Expected(UnexpectedBase, Args&&... args) noexcept
            : m_Unexpected(Forward<Args>(args)...)
            , m_HasValue(false)
        {
        }

        template <typename U, typename... Args>
            requires IsConstructibleV<E, std::initializer_list<U>&, Args...>
        constexpr explicit Expected(UnexpectedBase,
                                    std::initializer_list<U> ilist,
                                    Args&&... args) noexcept
            : m_Unexpected(ilist, Forward<Args>(args)...)
            , m_HasValue(false)
        {
        }

        constexpr ~Expected() = default;

        constexpr ~Expected()
            requires(!std::is_trivially_destructible_v<E>)
        {
            if (!m_HasValue) std::destroy_at(AddressOf(m_Unexpected));
        }

        Expected&           operator=(const Expected&) = delete;

        constexpr Expected& operator=(const Expected& lhs) noexcept
            requires IsCopyConstructibleV<E> && IsCopyAssignableV<E>
        {
            if (lhs.m_HasValue) Emplace();
            else AssignUnexpected(lhs.m_Unexpected);
            return *this;
        }

        constexpr Expected& operator=(Expected&& lhs) noexcept
            requires IsMoveConstructibleV<E> && IsMoveAssignableV<E>
        {
            if (lhs.m_HasValue) Emplace();
            else AssignUnexpected(Move(lhs.m_Unexpected));
            return *this;
        }

        template <typename UnexpectedResult>
            requires IsConstructibleV<E, const UnexpectedResult&>
                  && IsAssignableV<E&, const UnexpectedResult&>
        constexpr Expected& operator=(const Unexpected<UnexpectedResult>& e)
        {
            AssignUnexpected(e.Error());
            return *this;
        }

        template <typename UnexpectedResult>
            requires IsConstructibleV<E, UnexpectedResult>
                  && IsAssignableV<E&, UnexpectedResult>
        constexpr Expected& operator=(Unexpected<UnexpectedResult>&& e)
        {
            AssignUnexpected(Move(e.Error()));
            return *this;
        }

        constexpr void Emplace() noexcept
        {
            if (!m_HasValue)
            {
                std::destroy_at(AddressOf(m_Unexpected));
                m_HasValue = true;
            }
        }

        constexpr void Swap(Expected& lhs) noexcept
            requires IsSwappableV<E> && IsMoveConstructibleV<E>
        {
            if (m_HasValue)
            {
                if (!lhs.m_HasValue)
                {
                    std::construct_at(AddressOf(m_Unexpected),
                                      Move(lhs.m_Unexpected));
                    std::destroy_at(AddressOf(lhs.m_Unexpected));
                    m_HasValue     = false;
                    lhs.m_HasValue = true;
                }
            }
            else
            {
                if (lhs.m_HasValue)
                {
                    std::construct_at(AddressOf(lhs.m_Unexpected),
                                      Move(m_Unexpected));
                    std::destroy_at(AddressOf(m_Unexpected));
                    m_HasValue     = true;
                    lhs.m_HasValue = false;
                }
                else Swap(m_Unexpected, lhs.m_Unexpected);
            }
        }

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return m_HasValue;
        }

        [[nodiscard]]
        constexpr bool HasValue() const noexcept
        {
            return m_HasValue;
        }
        [[nodiscard]]
        constexpr bool has_value() const noexcept
        {
            return m_HasValue;
        }

        constexpr void operator*() const noexcept { assert(m_HasValue); }

        constexpr void Value() const&
        {
            if (m_HasValue) [[likely]]
                return;
            assert(false);
        }

        constexpr void Value() &&
        {
            if (m_HasValue) [[likely]]
                return;
            assert(false);
        }
        constexpr void value() const&
        {
            if (m_HasValue) [[likely]]
                return;
            assert(false);
        }

        constexpr void value() &&
        {
            if (m_HasValue) [[likely]]
                return;
            assert(false);
        }

        constexpr const E& Error() const& noexcept
        {
            assert(!m_HasValue);
            return m_Unexpected;
        }

        constexpr E& Error() & noexcept
        {
            assert(!m_HasValue);
            return m_Unexpected;
        }

        constexpr const E&& Error() const&& noexcept
        {
            assert(!m_HasValue);
            return Move(m_Unexpected);
        }

        constexpr E&& Error() && noexcept
        {
            assert(!m_HasValue);
            return Move(m_Unexpected);
        }

        constexpr const E& error() const& noexcept
        {
            assert(!m_HasValue);
            return m_Unexpected;
        }

        constexpr E& error() & noexcept
        {
            assert(!m_HasValue);
            return m_Unexpected;
        }

        constexpr const E&& error() const&& noexcept
        {
            assert(!m_HasValue);
            return Move(m_Unexpected);
        }

        constexpr E&& error() && noexcept
        {
            assert(!m_HasValue);
            return Move(m_Unexpected);
        }

        template <typename UnexpectedResult = E>
        constexpr E ErrorOr(UnexpectedResult&& e) const&
        {
            static_assert(IsCopyConstructibleV<E>);
            static_assert(IsConvertibleV<UnexpectedResult, E>);

            if (m_HasValue) return Forward<UnexpectedResult>(e);
            return m_Unexpected;
        }

        template <typename UnexpectedResult = E>
        constexpr E ErrorOr(UnexpectedResult&& e) &&
        {
            static_assert(IsMoveConstructibleV<E>);
            static_assert(IsConvertibleV<UnexpectedResult, E>);

            if (m_HasValue) return Forward<UnexpectedResult>(e);
            return Move(m_Unexpected);
        }

        template <typename F>
            requires IsConstructibleV<E, E&>
        constexpr auto AndThen(F&& f) &
        {
            using U = Detail::Result0<F>;
            static_assert(Detail::IsExpected<U>);
            static_assert(IsSameV<typename U::ErrorType, E>);

            if (HasValue()) return Invoke(Forward<F>(f));
            else return U(unexpect, m_Unexpected);
        }

        template <typename F>
            requires IsConstructibleV<E, const E&>
        constexpr auto AndThen(F&& f) const&
        {
            using U = Detail::Result0<F>;
            static_assert(Detail::IsExpected<U>);
            static_assert(IsSameV<typename U::ErrorType, E>);

            if (HasValue()) return Invoke(Forward<F>(f));
            else return U(unexpect, m_Unexpected);
        }

        template <typename F>
            requires IsConstructibleV<E, E>
        constexpr auto AndThen(F&& f) &&
        {
            using U = Detail::Result0<F>;
            static_assert(Detail::IsExpected<U>);
            static_assert(IsSameV<typename U::ErrorType, E>);

            if (HasValue()) return Invoke(Forward<F>(f));
            else return U(unexpect, Move(m_Unexpected));
        }

        template <typename F>
            requires IsConstructibleV<E, const E>
        constexpr auto AndThen(F&& f) const&&
        {
            using U = Detail::Result0<F>;
            static_assert(Detail::IsExpected<U>);
            static_assert(IsSameV<typename U::ErrorType, E>);

            if (HasValue()) return Invoke(Forward<F>(f));
            else return U(unexpect, Move(m_Unexpected));
        }

        template <typename F>
        constexpr auto OrElse(F&& f) &
        {
            using UnexpectedResult = Detail::Result<F, E&>;
            static_assert(Detail::IsExpected<UnexpectedResult>);
            static_assert(IsSameV<typename UnexpectedResult::ValueType, T>);

            if (HasValue()) return UnexpectedResult();
            else return Invoke(Forward<F>(f), m_Unexpected);
        }

        template <typename F>
        constexpr auto OrElse(F&& f) const&
        {
            using UnexpectedResult = Detail::Result<F, const E&>;
            static_assert(Detail::IsExpected<UnexpectedResult>);
            static_assert(IsSameV<typename UnexpectedResult::ValueType, T>);

            if (HasValue()) return UnexpectedResult();
            else return Invoke(Forward<F>(f), m_Unexpected);
        }

        template <typename F>
        constexpr auto OrElse(F&& f) &&
        {
            using UnexpectedResult = Detail::Result<F, E&&>;
            static_assert(Detail::IsExpected<UnexpectedResult>);
            static_assert(IsSameV<typename UnexpectedResult::ValueType, T>);

            if (HasValue()) return UnexpectedResult();
            else return Invoke(Forward<F>(f), Move(m_Unexpected));
        }

        template <typename F>
        constexpr auto OrElse(F&& f) const&&
        {
            using UnexpectedResult = Detail::Result<F, const E&&>;
            static_assert(Detail::IsExpected<UnexpectedResult>);
            static_assert(IsSameV<typename UnexpectedResult::ValueType, T>);

            if (HasValue()) return UnexpectedResult();
            else return Invoke(Forward<F>(f), Move(m_Unexpected));
        }

        template <typename F>
            requires IsConstructibleV<E, E&>
        constexpr auto Transform(F&& f) &
        {
            using U          = Detail::Result0From<F>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(InPlaceInvocation{}, Forward<F>(f));
            else return ResultType(unexpect, m_Unexpected);
        }

        template <typename F>
            requires IsConstructibleV<E, const E&>
        constexpr auto Transform(F&& f) const&
        {
            using U          = Detail::Result0From<F>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(InPlaceInvocation{}, Forward<F>(f));
            else return ResultType(unexpect, m_Unexpected);
        }

        template <typename F>
            requires IsConstructibleV<E, E>
        constexpr auto Transform(F&& f) &&
        {
            using U          = Detail::Result0From<F>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(InPlaceInvocation{}, Forward<F>(f));
            else return ResultType(unexpect, Move(m_Unexpected));
        }

        template <typename F>
            requires IsConstructibleV<E, const E>
        constexpr auto Transform(F&& f) const&&
        {
            using U          = Detail::Result0From<F>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(InPlaceInvocation{}, Forward<F>(f));
            else return ResultType(unexpect, Move(m_Unexpected));
        }

        template <typename F>
        constexpr auto TransformError(F&& f) &
        {
            using UnexpectedResult = Detail::ResultFrom<F, E&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue()) return ResultType();
            else
                return ResultType(
                    UnexpectedInvocation{},
                    [&]() { return Invoke(Forward<F>(f), m_Unexpected); });
        }

        template <typename F>
        constexpr auto TransformError(F&& f) const&
        {
            using UnexpectedResult = Detail::ResultFrom<F, const E&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue()) return ResultType();
            else
                return ResultType(
                    UnexpectedInvocation{},
                    [&]() { return Invoke(Forward<F>(f), m_Unexpected); });
        }

        template <typename F>
        constexpr auto TransformError(F&& f) &&
        {
            using UnexpectedResult = Detail::ResultFrom<F, E&&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue()) return ResultType();
            else
                return ResultType(
                    UnexpectedInvocation{}, [&]()
                    { return Invoke(Forward<F>(f), Move(m_Unexpected)); });
        }

        template <typename F>
        constexpr auto TransformError(F&& f) const&&
        {
            using UnexpectedResult = Detail::ResultFrom<F, const E&&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue()) return ResultType();
            else
                return ResultType(
                    UnexpectedInvocation{}, [&]()
                    { return Invoke(Forward<F>(f), Move(m_Unexpected)); });
        }

        template <typename U, typename E2>
            requires IsVoidV<U>
        friend constexpr bool operator==(const Expected&        lhs,
                                         const Expected<U, E2>& rhs)
        {
            if (lhs.HasValue()) return rhs.HasValue();
            else return !rhs.HasValue() && bool(lhs.Error() == rhs.Error());
        }

        template <typename Er2>
        friend constexpr bool operator==(const Expected&        lhs,
                                         const Unexpected<Er2>& e)
        {
            return !lhs.HasValue() && bool(lhs.Error() == e.Error());
        }

        friend constexpr void
        Swap(Expected& lhs, Expected& rhs) noexcept(noexcept(lhs.Swap(rhs)))
            requires requires { lhs.Swap(rhs); }
        {
            lhs.Swap(rhs);
        }

      private:
        template <typename, typename>
        friend class Expected;

        template <typename V>
        constexpr void AssignUnexpected(V&& value)
        {
            if (m_HasValue)
            {
                std::construct_at(AddressOf(m_Unexpected), Forward<V>(value));
                m_HasValue = false;
                return;
            }
            m_Unexpected = Forward<V>(value);
        }

        using InPlaceInvocation    = Detail::InPlaceInvocation;
        using UnexpectedInvocation = Detail::UnexpectedInvocation;

        template <typename F>
        explicit constexpr Expected(InPlaceInvocation, F&& f)
            : m_Void()
            , m_HasValue(true)
        {
            Forward<F>(f)();
        }

        template <typename F>
        explicit constexpr Expected(UnexpectedInvocation, F&& f)
            : m_Unexpected(Forward<F>(f)())
            , m_HasValue(false)
        {
        }

        union
        {
            struct
            {
            } m_Void;
            E m_Unexpected;
        };

        bool m_HasValue;
    };
} // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Expected;
using Prism::Unexpected;
#endif
