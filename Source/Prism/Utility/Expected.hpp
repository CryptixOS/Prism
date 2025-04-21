/*
 * Created by v1tr10l7 on 07.12.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

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
        using Result = std::remove_cvref_t<std::invoke_result_t<F&&, T&&>>;
        template <typename F, typename T>
        using ResultFrom = std::remove_cv_t<std::invoke_result_t<F&&, T&&>>;
        template <typename F>
        using Result0 = std::remove_cvref_t<std::invoke_result_t<F&&>>;
        template <typename F>
        using Result0From = std::remove_cv_t<std::invoke_result_t<F&&>>;

        template <typename E>
        concept CanBeUnexpected
            = std::is_object_v<E> && (!std::is_array_v<E>) && (!IsUnexpected<E>)
           && (!std::is_const_v<E>) && (!std::is_volatile_v<E>);

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
            requires(!std::is_same_v<std::remove_cvref_t<Err>, Unexpected>)
                 && (!std::is_same_v<std::remove_cvref_t<Err>, std::in_place_t>)
                 && std::is_constructible_v<E, Err>
        constexpr explicit Unexpected(Err&& e) noexcept
            : m_Unexpected(std::forward<Err>(e))
        {
        }

        template <typename... Args>
            requires std::is_constructible_v<E, Args...>
        constexpr explicit Unexpected(std::in_place_t, Args&&... args) noexcept
            : m_Unexpected(std::forward<Args>(args)...)
        {
        }

        template <typename U, typename... Args>
            requires std::is_constructible_v<E, std::initializer_list<U>&,
                                             Args...>
        constexpr explicit Unexpected(std::in_place_t,
                                      std::initializer_list<U> list,
                                      Args&&... args) noexcept
            : m_Unexpected(list, std::forward<Args>(args)...)
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
            return std::move(m_Unexpected);
        }

        [[nodiscard]]
        constexpr E&& Error() && noexcept
        {
            return std::move(m_Unexpected);
        }

        constexpr void Swap(Unexpected& other) noexcept
            requires std::is_swappable_v<E>
        {
            std::swap(m_Unexpected, other.m_Unexpected);
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
            requires std::is_swappable_v<E>
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
                : m_Guarded(std::addressof(lhs))
                , m_Temp(std::move(lhs))
            {
                std::destroy_at(m_Guarded);
            }

            constexpr ~Guard()
            {
                if (m_Guarded) [[unlikely]]
                    std::construct_at(m_Guarded, std::move(m_Temp));
            }

            Guard(const Guard&)                   = delete;
            Guard&        operator=(const Guard&) = delete;

            constexpr T&& Release() noexcept
            {
                m_Guarded = nullptr;
                return std::move(m_Temp);
            }

          private:
            T* m_Guarded;
            T  m_Temp;
        };

        template <typename T, typename U, typename V>
        constexpr void Reinitialize(T* newValue, U* oldValue, V&& arg) noexcept
        {
            std::destroy_at(oldValue);
            std::construct_at(newValue, std::forward<V>(arg));
        }
    } // namespace Detail

    template <typename T, typename E>
    class Expected
    {
        static_assert(!std::is_reference_v<T>);
        static_assert(!std::is_function_v<T>);
        static_assert(!std::is_same_v<std::remove_cv_t<T>, std::in_place_t>);
        static_assert(!std::is_same_v<std::remove_cv_t<T>, UnexpectedBase>);
        static_assert(!Detail::IsUnexpected<std::remove_cv_t<T>>);
        static_assert(Detail::CanBeUnexpected<E>);

        template <typename U, typename Err, typename Un = Unexpected<E>>
        static constexpr bool IsCostructibleFromExpected = std::disjunction_v<
            std::is_constructible<T, Expected<U, Err>&>,
            std::is_constructible<T, Expected<U, Err>>,
            std::is_constructible<T, const Expected<U, Err>&>,
            std::is_constructible<T, const Expected<U, Err>>,
            std::is_convertible<Expected<U, Err>&, T>,
            std::is_convertible<Expected<U, Err>, T>,
            std::is_convertible<const Expected<U, Err>&, T>,
            std::is_convertible<const Expected<U, Err>, T>,
            std::is_constructible<Un, Expected<U, Err>&>,
            std::is_constructible<Un, Expected<U, Err>>,
            std::is_constructible<Un, const Expected<U, Err>&>,
            std::is_constructible<Un, const Expected<U, Err>>>;

        template <typename U, typename Err>
        constexpr static bool IsExplicitlyConstructible
            = !std::is_convertible<U, T>::value
           || !std::is_convertible<Err, E>::value;

        template <typename U>
        static constexpr bool SameValue
            = std::is_same_v<typename U::ValueType, T>;

        template <typename U>
        static constexpr bool SameError
            = std::is_same_v<typename U::ErrorType, E>;

      public:
        using ValueType      = T;
        using ErrorType      = E;
        using UnexpectedType = Unexpected<E>;

        template <typename U>
        using Rebind = Expected<U, ErrorType>;

        constexpr Expected() noexcept
            requires std::is_default_constructible_v<T>
            : m_Value()
            , m_HasValue(true)
        {
        }

        Expected(const Expected&) = default;

        constexpr Expected(const Expected& lhs) noexcept
            requires std::is_copy_constructible_v<T>
                  && std::is_copy_constructible_v<E>
                  && (!std::is_trivially_copy_constructible_v<T>
                      || !std::is_trivially_copy_constructible_v<E>)
            : m_HasValue(lhs.m_HasValue)
        {
            if (m_HasValue)
                std::construct_at(std::addressof(m_Value), lhs.m_Value);
            else
                std::construct_at(std::addressof(m_Unexpected),
                                  lhs.m_Unexpected);
        }

        Expected(Expected&&) = default;

        constexpr Expected(Expected&& lhs) noexcept
            requires std::is_move_constructible_v<T>
                  && std::is_move_constructible_v<E>
                  && (!std::is_trivially_move_constructible_v<T>
                      || !std::is_trivially_move_constructible_v<E>)
            : m_HasValue(lhs.m_HasValue)
        {
            if (m_HasValue)
                std::construct_at(std::addressof(m_Value),
                                  std::move(lhs).m_Value);
            else
                std::construct_at(std::addressof(m_Unexpected),
                                  std::move(lhs).m_Unexpected);
        }

        template <typename U, typename UnexpectedResult>
            requires std::is_constructible_v<T, const U&>
                  && std::is_constructible_v<E, const UnexpectedResult&>
                  && (!IsCostructibleFromExpected<U, UnexpectedResult>)
        constexpr explicit Expected(
            const Expected<U, UnexpectedResult>& lhs) noexcept
            : m_HasValue(lhs.m_HasValue)
        {
            if (m_HasValue)
                std::construct_at(std::addressof(m_Value), lhs.m_Value);
            else
                std::construct_at(std::addressof(m_Unexpected),
                                  lhs.m_Unexpected);
        }

        template <typename U, typename UnexpectedResult>
            requires std::is_constructible_v<T, U>
                  && std::is_constructible_v<E, UnexpectedResult>
                  && (!IsCostructibleFromExpected<U, UnexpectedResult>)
        constexpr explicit Expected(
            Expected<U, UnexpectedResult>&& lhs) noexcept
            : m_HasValue(lhs.m_HasValue)
        {
            if (m_HasValue)
                std::construct_at(std::addressof(m_Value),
                                  std::move(lhs).m_Value);
            else
                std::construct_at(std::addressof(m_Unexpected),
                                  std::move(lhs).m_Unexpected);
        }

        template <typename U = T>
            requires(!std::is_same_v<std::remove_cvref_t<U>, Expected>)
                     && (!std::is_same_v<std::remove_cvref_t<U>,
                                         std::in_place_t>)
                     && (!Detail::IsUnexpected<std::remove_cvref_t<U>>)
                     && std::is_constructible_v<T, U>
        constexpr explicit(!std::is_convertible_v<U, T>)
            Expected(U&& value) noexcept
            : m_Value(std::forward<U>(value))
            , m_HasValue(true)
        {
        }

        template <typename UnexpectedResult = E>
            requires std::is_constructible_v<E, const UnexpectedResult&>
        constexpr explicit(!std::is_convertible_v<const UnexpectedResult&, E>)
            Expected(const Unexpected<UnexpectedResult>& __u) noexcept
            : m_Unexpected(__u.Error())
            , m_HasValue(false)
        {
        }

        template <typename UnexpectedResult = E>
            requires std::is_constructible_v<E, UnexpectedResult>
        constexpr explicit(!std::is_convertible_v<UnexpectedResult, E>)
            Expected(Unexpected<UnexpectedResult>&& __u) noexcept
            : m_Unexpected(std::move(__u).Error())
            , m_HasValue(false)
        {
        }

        template <typename... Args>
            requires std::is_constructible_v<T, Args...>
        constexpr explicit Expected(std::in_place_t, Args&&... args) noexcept
            : m_Value(std::forward<Args>(args)...)
            , m_HasValue(true)
        {
        }

        template <typename U, typename... Args>
            requires std::is_constructible_v<T, std::initializer_list<U>&,
                                             Args...>
        constexpr explicit Expected(std::in_place_t,
                                    std::initializer_list<U> ilist,
                                    Args&&... args) noexcept
            : m_Value(ilist, std::forward<Args>(args)...)
            , m_HasValue(true)
        {
        }

        template <typename... Args>
            requires std::is_constructible_v<E, Args...>
        constexpr explicit Expected(UnexpectedBase, Args&&... args) noexcept
            : m_Unexpected(std::forward<Args>(args)...)
            , m_HasValue(false)
        {
        }

        template <typename U, typename... Args>
            requires std::is_constructible_v<E, std::initializer_list<U>&,
                                             Args...>
        constexpr explicit Expected(UnexpectedBase,
                                    std::initializer_list<U> ilist,
                                    Args&&... args) noexcept
            : m_Unexpected(ilist, std::forward<Args>(args)...)
            , m_HasValue(false)
        {
        }

        constexpr ~Expected() = default;

        constexpr ~Expected()
            requires(!std::is_trivially_destructible_v<T>)
                 || (!std::is_trivially_destructible_v<E>)
        {
            if (m_HasValue) std::destroy_at(std::addressof(m_Value));
            else std::destroy_at(std::addressof(m_Unexpected));
        }

        Expected&           operator=(const Expected&) = delete;
        constexpr Expected& operator=(const Expected& lhs) noexcept
            requires std::is_copy_assignable_v<T>
                  && std::is_copy_constructible_v<T>
                  && std::is_copy_assignable_v<E>
                  && std::is_copy_constructible_v<E>
        {
            if (lhs.m_HasValue) this->_assign_value(lhs.m_Value);
            else this->AssignUnexpected(lhs.m_Unexpected);
            return *this;
        }

        constexpr Expected& operator=(Expected&& lhs) noexcept
            requires std::is_move_assignable_v<T>
                  && std::is_move_constructible_v<T>
                  && std::is_move_assignable_v<E>
                  && std::is_move_constructible_v<E>
        {
            if (lhs.m_HasValue) AssignValue(std::move(lhs.m_Value));
            else AssignUnexpected(std::move(lhs.m_Unexpected));
            return *this;
        }

        template <typename U = T>
            requires(!std::is_same_v<Expected, std::remove_cvref_t<U>>)
                 && (!Detail::IsUnexpected<std::remove_cvref_t<U>>)
                 && std::is_constructible_v<T, U> && std::is_assignable_v<T&, U>
        constexpr Expected& operator=(U&& value)
        {
            AssignValue(std::forward<U>(value));
            return *this;
        }

        template <typename UnexpectedResult>
            requires std::is_constructible_v<E, const UnexpectedResult&>
                  && std::is_assignable_v<E&, const UnexpectedResult&>
        constexpr Expected& operator=(const Unexpected<UnexpectedResult>& e)
        {
            AssignUnexpected(e.Error());
            return *this;
        }

        template <typename UnexpectedResult>
            requires std::is_constructible_v<E, UnexpectedResult>
                  && std::is_assignable_v<E&, UnexpectedResult>
        constexpr Expected& operator=(Unexpected<UnexpectedResult>&& e)
        {
            AssignUnexpected(std::move(e).Error());
            return *this;
        }

        template <typename... Args>
        constexpr T& Emplace(Args&&... args) noexcept
        {
            if (m_HasValue) std::destroy_at(std::addressof(m_Value));
            else
            {
                std::destroy_at(std::addressof(m_Unexpected));
                m_HasValue = true;
            }
            std::construct_at(std::addressof(m_Value),
                              std::forward<Args>(args)...);
            return m_Value;
        }

        template <typename U, typename... Args>
        constexpr T& Emplace(std::initializer_list<U> ilist,
                             Args&&... args) noexcept
        {
            if (m_HasValue) std::destroy_at(std::addressof(m_Value));
            else
            {
                std::destroy_at(std::addressof(m_Unexpected));
                m_HasValue = true;
            }
            std::construct_at(std::addressof(m_Value), ilist,
                              std::forward<Args>(args)...);
            return m_Value;
        }

        constexpr void Swap(Expected& lhs) noexcept
            requires std::is_swappable_v<T> && std::is_swappable_v<E>
                  && std::is_move_constructible_v<T>
                  && std::is_move_constructible_v<E>
        {
            if (m_HasValue)
            {
                if (lhs.m_HasValue) std::swap(m_Value, lhs.m_Value);
                else SwapValueWithUnexpected(lhs);
            }
            else
            {
                if (lhs.m_HasValue) lhs.SwapValueWithUnexpected(*this);
                else std::swap(m_Unexpected, lhs.m_Unexpected);
            }
        }

        // observers

        [[nodiscard]]
        constexpr const T* operator->() const noexcept
        {
            assert(m_HasValue);
            return std::addressof(m_Value);
        }

        [[nodiscard]]
        constexpr T* operator->() noexcept
        {
            assert(m_HasValue);
            return std::addressof(m_Value);
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
            return std::move(m_Value);
        }

        [[nodiscard]]
        constexpr T&& operator*() && noexcept
        {
            assert(m_HasValue);
            return std::move(m_Value);
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
                return std::move(m_Value);
            assert(false);
        }

        constexpr T&& value() &&
        {
            if (m_HasValue) [[likely]]
                return std::move(m_Value);
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
            return std::move(m_Unexpected);
        }

        constexpr E&& Error() && noexcept
        {
            assert(!m_HasValue);
            return std::move(m_Unexpected);
        }

        template <typename U>
        constexpr T ValueOr(U&& value) const& noexcept
        {
            static_assert(std::is_copy_constructible_v<T>);
            static_assert(std::is_convertible_v<U, T>);

            if (m_HasValue) return m_Value;
            return static_cast<T>(std::forward<U>(value));
        }

        template <typename U>
        constexpr T ValueOr(U&& value) && noexcept
        {
            static_assert(std::is_move_constructible_v<T>);
            static_assert(std::is_convertible_v<U, T>);

            if (m_HasValue) return std::move(m_Value);
            return static_cast<T>(std::forward<U>(value));
        }

        template <typename UnexpectedResult = E>
        constexpr E ErrorOr(UnexpectedResult&& e) const&
        {
            static_assert(std::is_copy_constructible_v<E>);
            static_assert(std::is_convertible_v<UnexpectedResult, E>);

            if (m_HasValue) return std::forward<UnexpectedResult>(e);
            return m_Unexpected;
        }

        template <typename UnexpectedResult = E>
        constexpr E ErrorOr(UnexpectedResult&& e) &&
        {
            static_assert(std::is_move_constructible_v<E>);
            static_assert(std::is_convertible_v<UnexpectedResult, E>);

            if (m_HasValue) return std::forward<UnexpectedResult>(e);
            return std::move(m_Unexpected);
        }

        // monadic operations

        template <typename F>
            requires std::is_constructible_v<E, E&>
        constexpr auto AndThen(F&& f) &
        {
            using U = Detail::Result<F, T&>;
            static_assert(Detail::IsExpected<U>,
                          "the function passed to std::Expected<T, E>::AndThen "
                          "must return a std::Expected");
            static_assert(
                std::is_same_v<typename U::ErrorType, E>,
                "the function passed to std::Expected<T, E>::AndThen "
                "must return a std::Expected with the same ErrorType");

            if (HasValue()) return std::invoke(std::forward<F>(f), m_Value);
            else return U(unexpect, m_Unexpected);
        }

        template <typename F>
            requires std::is_constructible_v<E, const E&>
        constexpr auto AndThen(F&& f) const&
        {
            using U = Detail::Result<F, const T&>;
            static_assert(Detail::IsExpected<U>,
                          "the function passed to std::Expected<T, E>::AndThen "
                          "must return a std::Expected");
            static_assert(
                std::is_same_v<typename U::ErrorType, E>,
                "the function passed to std::Expected<T, E>::AndThen "
                "must return a std::Expected with the same ErrorType");

            if (HasValue()) return std::invoke(std::forward<F>(f), m_Value);
            else return U(unexpect, m_Unexpected);
        }

        template <typename F>
            requires std::is_constructible_v<E, E>
        constexpr auto AndThen(F&& f) &&
        {
            using U = Detail::Result<F, T&&>;
            static_assert(Detail::IsExpected<U>,
                          "the function passed to std::Expected<T, E>::AndThen "
                          "must return a std::Expected");
            static_assert(
                std::is_same_v<typename U::ErrorType, E>,
                "the function passed to std::Expected<T, E>::AndThen "
                "must return a std::Expected with the same ErrorType");

            if (HasValue())
                return std::invoke(std::forward<F>(f), std::move(m_Value));
            else return U(unexpect, std::move(m_Unexpected));
        }

        template <typename F>
            requires std::is_constructible_v<E, const E>
        constexpr auto AndThen(F&& f) const&&
        {
            using U = Detail::Result<F, const T&&>;
            static_assert(Detail::IsExpected<U>,
                          "the function passed to std::Expected<T, E>::AndThen "
                          "must return a std::Expected");
            static_assert(
                std::is_same_v<typename U::ErrorType, E>,
                "the function passed to std::Expected<T, E>::AndThen "
                "must return a std::Expected with the same ErrorType");

            if (HasValue())
                return std::invoke(std::forward<F>(f), std::move(m_Value));
            else return U(unexpect, std::move(m_Unexpected));
        }

        template <typename F>
            requires std::is_constructible_v<T, T&>
        constexpr auto OrElse(F&& f) &
        {
            using UnexpectedResult = Detail::Result<F, E&>;
            static_assert(Detail::IsExpected<UnexpectedResult>,
                          "the function passed to std::Expected<T, E>::OrElse "
                          "must return a std::Expected");
            static_assert(
                std::is_same_v<typename UnexpectedResult::ValueType, T>,
                "the function passed to std::Expected<T, E>::OrElse "
                "must return a std::Expected with the same ValueType");

            if (HasValue()) return UnexpectedResult(std::in_place, m_Value);
            else return std::invoke(std::forward<F>(f), m_Unexpected);
        }

        template <typename F>
            requires std::is_constructible_v<T, const T&>
        constexpr auto OrElse(F&& f) const&
        {
            using UnexpectedResult = Detail::Result<F, const E&>;
            static_assert(Detail::IsExpected<UnexpectedResult>,
                          "the function passed to std::Expected<T, E>::OrElse "
                          "must return a std::Expected");
            static_assert(
                std::is_same_v<typename UnexpectedResult::ValueType, T>,
                "the function passed to std::Expected<T, E>::OrElse "
                "must return a std::Expected with the same ValueType");

            if (HasValue()) return UnexpectedResult(std::in_place, m_Value);
            else return std::invoke(std::forward<F>(f), m_Unexpected);
        }

        template <typename F>
            requires std::is_constructible_v<T, T>
        constexpr auto OrElse(F&& f) &&
        {
            using UnexpectedResult = Detail::Result<F, E&&>;
            static_assert(Detail::IsExpected<UnexpectedResult>,
                          "the function passed to std::Expected<T, E>::OrElse "
                          "must return a std::Expected");
            static_assert(
                std::is_same_v<typename UnexpectedResult::ValueType, T>,
                "the function passed to std::Expected<T, E>::OrElse "
                "must return a std::Expected with the same ValueType");

            if (HasValue())
                return UnexpectedResult(std::in_place, std::move(m_Value));
            else
                return std::invoke(std::forward<F>(f), std::move(m_Unexpected));
        }

        template <typename F>
            requires std::is_constructible_v<T, const T>
        constexpr auto OrElse(F&& f) const&&
        {
            using UnexpectedResult = Detail::Result<F, const E&&>;
            static_assert(Detail::IsExpected<UnexpectedResult>,
                          "the function passed to std::Expected<T, E>::OrElse "
                          "must return a std::Expected");
            static_assert(
                std::is_same_v<typename UnexpectedResult::ValueType, T>,
                "the function passed to std::Expected<T, E>::OrElse "
                "must return a std::Expected with the same ValueType");

            if (HasValue())
                return UnexpectedResult(std::in_place, std::move(m_Value));
            else
                return std::invoke(std::forward<F>(f), std::move(m_Unexpected));
        }

        template <typename F>
            requires std::is_constructible_v<E, E&>
        constexpr auto Transform(F&& f) &
        {
            using U          = Detail::ResultFrom<F, T&>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(
                    InPlaceInvocation{},
                    [&]() { return std::invoke(std::forward<F>(f), m_Value); });
            else return ResultType(unexpect, m_Unexpected);
        }

        template <typename F>
            requires std::is_constructible_v<E, const E&>
        constexpr auto Transform(F&& f) const&
        {
            using U          = Detail::ResultFrom<F, const T&>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(
                    InPlaceInvocation{},
                    [&]() { return std::invoke(std::forward<F>(f), m_Value); });
            else return ResultType(unexpect, m_Unexpected);
        }

        template <typename F>
            requires std::is_constructible_v<E, E>
        constexpr auto Transform(F&& f) &&
        {
            using U          = Detail::ResultFrom<F, T>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(InPlaceInvocation{},
                                  [&]()
                                  {
                                      return std::invoke(std::forward<F>(f),
                                                         std::move(m_Value));
                                  });
            else return ResultType(unexpect, std::move(m_Unexpected));
        }

        template <typename F>
            requires std::is_constructible_v<E, const E>
        constexpr auto Transform(F&& f) const&&
        {
            using U          = Detail::ResultFrom<F, const T>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(InPlaceInvocation{},
                                  [&]()
                                  {
                                      return std::invoke(std::forward<F>(f),
                                                         std::move(m_Value));
                                  });
            else return ResultType(unexpect, std::move(m_Unexpected));
        }

        template <typename F>
            requires std::is_constructible_v<T, T&>
        constexpr auto TransformError(F&& f) &
        {
            using UnexpectedResult = Detail::ResultFrom<F, E&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue()) return ResultType(std::in_place, m_Value);
            else
                return ResultType(
                    UnexpectedInvocation{}, [&]()
                    { return std::invoke(std::forward<F>(f), m_Unexpected); });
        }

        template <typename F>
            requires std::is_constructible_v<T, const T&>
        constexpr auto TransformError(F&& f) const&
        {
            using UnexpectedResult = Detail::ResultFrom<F, const E&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue()) return ResultType(std::in_place, m_Value);
            else
                return ResultType(
                    UnexpectedInvocation{}, [&]()
                    { return std::invoke(std::forward<F>(f), m_Unexpected); });
        }

        template <typename F>
            requires std::is_constructible_v<T, T>
        constexpr auto TransformError(F&& f) &&
        {
            using UnexpectedResult = Detail::ResultFrom<F, E&&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue())
                return ResultType(std::in_place, std::move(m_Value));
            else
                return ResultType(UnexpectedInvocation{},
                                  [&]()
                                  {
                                      return std::invoke(
                                          std::forward<F>(f),
                                          std::move(m_Unexpected));
                                  });
        }

        template <typename F>
            requires std::is_constructible_v<T, const T>
        constexpr auto TransformError(F&& f) const&&
        {
            using UnexpectedResult = Detail::ResultFrom<F, const E&&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue())
                return ResultType(std::in_place, std::move(m_Value));
            else
                return ResultType(UnexpectedInvocation{},
                                  [&]()
                                  {
                                      return std::invoke(
                                          std::forward<F>(f),
                                          std::move(m_Unexpected));
                                  });
        }

        template <typename U, typename E2>
            requires(!std::is_void_v<U>)
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
            if (m_HasValue) m_Value = std::forward<V>(value);
            else
            {
                Detail::Reinitialize(std::addressof(m_Value),
                                     std::addressof(m_Unexpected),
                                     std::forward<V>(value));
                m_HasValue = true;
            }
        }

        template <typename V>
        constexpr void AssignUnexpected(V&& value)
        {
            if (m_HasValue)
            {
                Detail::Reinitialize(std::addressof(m_Unexpected),
                                     std::addressof(m_Value),
                                     std::forward<V>(value));
                m_HasValue = false;
                return;
            }
            m_Unexpected = std::forward<V>(value);
        }

        constexpr void SwapValueWithUnexpected(Expected& rhs) noexcept
        {
            if constexpr (std::is_move_constructible_v<E>)
            {
                Detail::Guard<E> __guard(rhs.m_Unexpected);
                std::construct_at(std::addressof(rhs.m_Value),
                                  std::move(m_Value));
                rhs.m_HasValue = true;
                std::destroy_at(std::addressof(m_Value));
                std::construct_at(std::addressof(m_Unexpected),
                                  __guard.release());
                m_HasValue = false;
            }
            else
            {
                Detail::Guard<T> guard(m_Value);
                std::construct_at(std::addressof(m_Unexpected),
                                  std::move(rhs.m_Unexpected));
                m_HasValue = false;
                std::destroy_at(std::addressof(rhs.m_Unexpected));
                std::construct_at(std::addressof(rhs.m_Value), guard.release());
                rhs.m_HasValue = true;
            }
        }

        using InPlaceInvocation    = Detail::InPlaceInvocation;
        using UnexpectedInvocation = Detail::UnexpectedInvocation;

        template <typename F>
        explicit constexpr Expected(InPlaceInvocation, F&& f)
            : m_Value(std::forward<F>(f)())
            , m_HasValue(true)
        {
        }

        template <typename F>
        explicit constexpr Expected(UnexpectedInvocation, F&& f)
            : m_Unexpected(std::forward<F>(f)())
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
        requires std::is_void_v<T>
    class Expected<T, E>
    {
        static_assert(Detail::CanBeUnexpected<E>);

        template <typename U, typename Err, typename Unex = Unexpected<E>>
        static constexpr bool IsCostructibleFromExpected = std::disjunction_v<
            std::is_constructible<Unex, Expected<U, Err>&>,
            std::is_constructible<Unex, Expected<U, Err>>,
            std::is_constructible<Unex, const Expected<U, Err>&>,
            std::is_constructible<Unex, const Expected<U, Err>>>;

        template <typename U>
        static constexpr bool SameValue
            = std::is_same_v<typename U::ValueType, T>;

        template <typename U>
        static constexpr bool SameError
            = std::is_same_v<typename U::ErrorType, E>;

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
            requires std::is_copy_constructible_v<E>
                      && (!std::is_trivially_copy_constructible_v<E>)
            : m_Void()
            , m_HasValue(lhs.m_HasValue)
        {
            if (!m_HasValue)
                std::construct_at(std::addressof(m_Unexpected),
                                  lhs.m_Unexpected);
        }

        Expected(Expected&&) = default;

        constexpr Expected(Expected&& lhs) noexcept
            requires std::is_move_constructible_v<E>
                      && (!std::is_trivially_move_constructible_v<E>)
            : m_Void()
            , m_HasValue(lhs.m_HasValue)
        {
            if (!m_HasValue)
                std::construct_at(std::addressof(m_Unexpected),
                                  std::move(lhs).m_Unexpected);
        }

        template <typename U, typename UnexpectedResult>
            requires std::is_void_v<U>
                      && std::is_constructible_v<E, const UnexpectedResult&>
                      && (!IsCostructibleFromExpected<U, UnexpectedResult>)
        constexpr explicit(!std::is_convertible_v<const UnexpectedResult&, E>)
            Expected(const Expected<U, UnexpectedResult>& lhs) noexcept
            : m_Void()
            , m_HasValue(lhs.m_HasValue)
        {
            if (!m_HasValue)
                std::construct_at(std::addressof(m_Unexpected),
                                  lhs.m_Unexpected);
        }

        template <typename U, typename UnexpectedResult>
            requires std::is_void_v<U>
                      && std::is_constructible_v<E, UnexpectedResult>
                      && (!IsCostructibleFromExpected<U, UnexpectedResult>)
        constexpr explicit(!std::is_convertible_v<UnexpectedResult, E>)
            Expected(Expected<U, UnexpectedResult>&& lhs) noexcept
            : m_Void()
            , m_HasValue(lhs.m_HasValue)
        {
            if (!m_HasValue)
                std::construct_at(std::addressof(m_Unexpected),
                                  std::move(lhs).m_Unexpected);
        }

        template <typename UnexpectedResult = E>
            requires std::is_constructible_v<E, const UnexpectedResult&>
        constexpr explicit(!std::is_convertible_v<const UnexpectedResult&, E>)
            Expected(const Unexpected<UnexpectedResult>& __u) noexcept
            : m_Unexpected(__u.Error())
            , m_HasValue(false)
        {
        }

        template <typename UnexpectedResult = E>
            requires std::is_constructible_v<E, UnexpectedResult>
        constexpr explicit(!std::is_convertible_v<UnexpectedResult, E>)
            Expected(Unexpected<UnexpectedResult>&& __u) noexcept
            : m_Unexpected(std::move(__u).Error())
            , m_HasValue(false)
        {
        }

        constexpr explicit Expected(std::in_place_t) noexcept
            : Expected()
        {
        }

        template <typename... Args>
            requires std::is_constructible_v<E, Args...>
        constexpr explicit Expected(UnexpectedBase, Args&&... args) noexcept
            : m_Unexpected(std::forward<Args>(args)...)
            , m_HasValue(false)
        {
        }

        template <typename U, typename... Args>
            requires std::is_constructible_v<E, std::initializer_list<U>&,
                                             Args...>
        constexpr explicit Expected(UnexpectedBase,
                                    std::initializer_list<U> ilist,
                                    Args&&... args) noexcept
            : m_Unexpected(ilist, std::forward<Args>(args)...)
            , m_HasValue(false)
        {
        }

        constexpr ~Expected() = default;

        constexpr ~Expected()
            requires(!std::is_trivially_destructible_v<E>)
        {
            if (!m_HasValue) std::destroy_at(std::addressof(m_Unexpected));
        }

        Expected&           operator=(const Expected&) = delete;

        constexpr Expected& operator=(const Expected& lhs) noexcept
            requires std::is_copy_constructible_v<E>
                  && std::is_copy_assignable_v<E>
        {
            if (lhs.m_HasValue) Emplace();
            else AssignUnexpected(lhs.m_Unexpected);
            return *this;
        }

        constexpr Expected& operator=(Expected&& lhs) noexcept
            requires std::is_move_constructible_v<E>
                  && std::is_move_assignable_v<E>
        {
            if (lhs.m_HasValue) Emplace();
            else AssignUnexpected(std::move(lhs.m_Unexpected));
            return *this;
        }

        template <typename UnexpectedResult>
            requires std::is_constructible_v<E, const UnexpectedResult&>
                  && std::is_assignable_v<E&, const UnexpectedResult&>
        constexpr Expected& operator=(const Unexpected<UnexpectedResult>& e)
        {
            AssignUnexpected(e.Error());
            return *this;
        }

        template <typename UnexpectedResult>
            requires std::is_constructible_v<E, UnexpectedResult>
                  && std::is_assignable_v<E&, UnexpectedResult>
        constexpr Expected& operator=(Unexpected<UnexpectedResult>&& e)
        {
            AssignUnexpected(std::move(e.Error()));
            return *this;
        }

        constexpr void Emplace() noexcept
        {
            if (!m_HasValue)
            {
                std::destroy_at(std::addressof(m_Unexpected));
                m_HasValue = true;
            }
        }

        constexpr void Swap(Expected& lhs) noexcept
            requires std::is_swappable_v<E> && std::is_move_constructible_v<E>
        {
            if (m_HasValue)
            {
                if (!lhs.m_HasValue)
                {
                    std::construct_at(std::addressof(m_Unexpected),
                                      std::move(lhs.m_Unexpected));
                    std::destroy_at(std::addressof(lhs.m_Unexpected));
                    m_HasValue     = false;
                    lhs.m_HasValue = true;
                }
            }
            else
            {
                if (lhs.m_HasValue)
                {
                    std::construct_at(std::addressof(lhs.m_Unexpected),
                                      std::move(m_Unexpected));
                    std::destroy_at(std::addressof(m_Unexpected));
                    m_HasValue     = true;
                    lhs.m_HasValue = false;
                }
                else std::swap(m_Unexpected, lhs.m_Unexpected);
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
            return std::move(m_Unexpected);
        }

        constexpr E&& Error() && noexcept
        {
            assert(!m_HasValue);
            return std::move(m_Unexpected);
        }

        template <typename UnexpectedResult = E>
        constexpr E ErrorOr(UnexpectedResult&& e) const&
        {
            static_assert(std::is_copy_constructible_v<E>);
            static_assert(std::is_convertible_v<UnexpectedResult, E>);

            if (m_HasValue) return std::forward<UnexpectedResult>(e);
            return m_Unexpected;
        }

        template <typename UnexpectedResult = E>
        constexpr E ErrorOr(UnexpectedResult&& e) &&
        {
            static_assert(std::is_move_constructible_v<E>);
            static_assert(std::is_convertible_v<UnexpectedResult, E>);

            if (m_HasValue) return std::forward<UnexpectedResult>(e);
            return std::move(m_Unexpected);
        }

        template <typename F>
            requires std::is_constructible_v<E, E&>
        constexpr auto AndThen(F&& f) &
        {
            using U = Detail::Result0<F>;
            static_assert(Detail::IsExpected<U>);
            static_assert(std::is_same_v<typename U::ErrorType, E>);

            if (HasValue()) return std::invoke(std::forward<F>(f));
            else return U(unexpect, m_Unexpected);
        }

        template <typename F>
            requires std::is_constructible_v<E, const E&>
        constexpr auto AndThen(F&& f) const&
        {
            using U = Detail::Result0<F>;
            static_assert(Detail::IsExpected<U>);
            static_assert(std::is_same_v<typename U::ErrorType, E>);

            if (HasValue()) return std::invoke(std::forward<F>(f));
            else return U(unexpect, m_Unexpected);
        }

        template <typename F>
            requires std::is_constructible_v<E, E>
        constexpr auto AndThen(F&& f) &&
        {
            using U = Detail::Result0<F>;
            static_assert(Detail::IsExpected<U>);
            static_assert(std::is_same_v<typename U::ErrorType, E>);

            if (HasValue()) return std::invoke(std::forward<F>(f));
            else return U(unexpect, std::move(m_Unexpected));
        }

        template <typename F>
            requires std::is_constructible_v<E, const E>
        constexpr auto AndThen(F&& f) const&&
        {
            using U = Detail::Result0<F>;
            static_assert(Detail::IsExpected<U>);
            static_assert(std::is_same_v<typename U::ErrorType, E>);

            if (HasValue()) return std::invoke(std::forward<F>(f));
            else return U(unexpect, std::move(m_Unexpected));
        }

        template <typename F>
        constexpr auto OrElse(F&& f) &
        {
            using UnexpectedResult = Detail::Result<F, E&>;
            static_assert(Detail::IsExpected<UnexpectedResult>);
            static_assert(
                std::is_same_v<typename UnexpectedResult::ValueType, T>);

            if (HasValue()) return UnexpectedResult();
            else return std::invoke(std::forward<F>(f), m_Unexpected);
        }

        template <typename F>
        constexpr auto OrElse(F&& f) const&
        {
            using UnexpectedResult = Detail::Result<F, const E&>;
            static_assert(Detail::IsExpected<UnexpectedResult>);
            static_assert(
                std::is_same_v<typename UnexpectedResult::ValueType, T>);

            if (HasValue()) return UnexpectedResult();
            else return std::invoke(std::forward<F>(f), m_Unexpected);
        }

        template <typename F>
        constexpr auto OrElse(F&& f) &&
        {
            using UnexpectedResult = Detail::Result<F, E&&>;
            static_assert(Detail::IsExpected<UnexpectedResult>);
            static_assert(
                std::is_same_v<typename UnexpectedResult::ValueType, T>);

            if (HasValue()) return UnexpectedResult();
            else
                return std::invoke(std::forward<F>(f), std::move(m_Unexpected));
        }

        template <typename F>
        constexpr auto OrElse(F&& f) const&&
        {
            using UnexpectedResult = Detail::Result<F, const E&&>;
            static_assert(Detail::IsExpected<UnexpectedResult>);
            static_assert(
                std::is_same_v<typename UnexpectedResult::ValueType, T>);

            if (HasValue()) return UnexpectedResult();
            else
                return std::invoke(std::forward<F>(f), std::move(m_Unexpected));
        }

        template <typename F>
            requires std::is_constructible_v<E, E&>
        constexpr auto Transform(F&& f) &
        {
            using U          = Detail::Result0From<F>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(InPlaceInvocation{}, std::forward<F>(f));
            else return ResultType(unexpect, m_Unexpected);
        }

        template <typename F>
            requires std::is_constructible_v<E, const E&>
        constexpr auto Transform(F&& f) const&
        {
            using U          = Detail::Result0From<F>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(InPlaceInvocation{}, std::forward<F>(f));
            else return ResultType(unexpect, m_Unexpected);
        }

        template <typename F>
            requires std::is_constructible_v<E, E>
        constexpr auto Transform(F&& f) &&
        {
            using U          = Detail::Result0From<F>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(InPlaceInvocation{}, std::forward<F>(f));
            else return ResultType(unexpect, std::move(m_Unexpected));
        }

        template <typename F>
            requires std::is_constructible_v<E, const E>
        constexpr auto Transform(F&& f) const&&
        {
            using U          = Detail::Result0From<F>;
            using ResultType = Expected<U, E>;

            if (HasValue())
                return ResultType(InPlaceInvocation{}, std::forward<F>(f));
            else return ResultType(unexpect, std::move(m_Unexpected));
        }

        template <typename F>
        constexpr auto TransformError(F&& f) &
        {
            using UnexpectedResult = Detail::ResultFrom<F, E&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue()) return ResultType();
            else
                return ResultType(
                    UnexpectedInvocation{}, [&]()
                    { return std::invoke(std::forward<F>(f), m_Unexpected); });
        }

        template <typename F>
        constexpr auto TransformError(F&& f) const&
        {
            using UnexpectedResult = Detail::ResultFrom<F, const E&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue()) return ResultType();
            else
                return ResultType(
                    UnexpectedInvocation{}, [&]()
                    { return std::invoke(std::forward<F>(f), m_Unexpected); });
        }

        template <typename F>
        constexpr auto TransformError(F&& f) &&
        {
            using UnexpectedResult = Detail::ResultFrom<F, E&&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue()) return ResultType();
            else
                return ResultType(UnexpectedInvocation{},
                                  [&]()
                                  {
                                      return std::invoke(
                                          std::forward<F>(f),
                                          std::move(m_Unexpected));
                                  });
        }

        template <typename F>
        constexpr auto TransformError(F&& f) const&&
        {
            using UnexpectedResult = Detail::ResultFrom<F, const E&&>;
            using ResultType       = Expected<T, UnexpectedResult>;

            if (HasValue()) return ResultType();
            else
                return ResultType(UnexpectedInvocation{},
                                  [&]()
                                  {
                                      return std::invoke(
                                          std::forward<F>(f),
                                          std::move(m_Unexpected));
                                  });
        }

        template <typename U, typename E2>
            requires std::is_void_v<U>
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
                std::construct_at(std::addressof(m_Unexpected),
                                  std::forward<V>(value));
                m_HasValue = false;
                return;
            }
            m_Unexpected = std::forward<V>(value);
        }

        using InPlaceInvocation    = Detail::InPlaceInvocation;
        using UnexpectedInvocation = Detail::UnexpectedInvocation;

        template <typename F>
        explicit constexpr Expected(InPlaceInvocation, F&& f)
            : m_Void()
            , m_HasValue(true)
        {
            std::forward<F>(f)();
        }

        template <typename F>
        explicit constexpr Expected(UnexpectedInvocation, F&& f)
            : m_Unexpected(std::forward<F>(f)())
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
