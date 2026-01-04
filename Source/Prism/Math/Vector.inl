/*
 * Created by v1tr10l7 on 07.11.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Math/Vector.hpp>
#include <Prism/Utility/Math.hpp>

namespace Prism
{
    namespace Math
    {
        // ---------------------
        // Constructors
        // ---------------------
        template <usize N, typename T>
        constexpr Vector<N, T>::Vector() = default;

        template <usize N, typename T>
        constexpr Vector<N, T>::Vector(T value)
        {
            for (usize i = 0; i < N; ++i) Data[i] = value;
        }

        template <usize N, typename T>
        constexpr Vector<N, T>::Vector(InitializerList<T> values)
        {
            assert(values.size() == N && "Initializer size mismatch");
            usize i = 0;
            for (auto& v : values) Data[i++] = v;
        }

        template <usize N, typename T>
        constexpr Vector<N, T>&
        Vector<N, T>::operator=(InitializerList<T> values)
        {
            assert(values.size() == N && "Initializer size mismatch");
            usize i = 0;
            for (auto& v : values) Data[i++] = v;
            return *this;
        }

        // ---------------------
        // Access
        // ---------------------
        template <usize N, typename T>
        constexpr T& Vector<N, T>::operator[](usize i)
        {
            assert(i < N);
            return Data[i];
        }

        template <usize N, typename T>
        constexpr const T& Vector<N, T>::operator[](usize i) const
        {
            assert(i < N);
            return Data[i];
        }

        // ---------------------
        // Comparison
        // ---------------------
        template <usize N, typename T>
        constexpr bool Vector<N, T>::operator==(const Vector& other) const
        {
            for (usize i = 0; i < N; ++i)
                if (Data[i] != other.Data[i]) return false;
            return true;
        }

        template <usize N, typename T>
        constexpr bool Vector<N, T>::operator!=(const Vector& other) const
        {
            return !(*this == other);
        }

        // ---------------------
        // Arithmetic
        // ---------------------
        template <usize N, typename T>
        constexpr Vector<N, T> Vector<N, T>::operator+(const Vector& rhs) const
        {
            Vector result;
            for (usize i = 0; i < N; ++i)
                result.Data[i] = Data[i] + rhs.Data[i];
            return result;
        }

        template <usize N, typename T>
        constexpr Vector<N, T> Vector<N, T>::operator-(const Vector& rhs) const
        {
            Vector result;
            for (usize i = 0; i < N; ++i)
                result.Data[i] = Data[i] - rhs.Data[i];
            return result;
        }

        template <usize N, typename T>
        constexpr Vector<N, T> Vector<N, T>::operator*(const Vector& rhs) const
        {
            Vector result;
            for (usize i = 0; i < N; ++i)
                result.Data[i] = Data[i] * rhs.Data[i];
            return result;
        }

        template <usize N, typename T>
        constexpr Vector<N, T> Vector<N, T>::operator/(const Vector& rhs) const
        {
            Vector result;
            for (usize i = 0; i < N; ++i)
                result.Data[i] = Data[i] / rhs.Data[i];
            return result;
        }

        template <usize N, typename T>
        constexpr Vector<N, T> Vector<N, T>::operator*(T scalar) const
        {
            Vector result;
            for (usize i = 0; i < N; ++i) result.Data[i] = Data[i] * scalar;
            return result;
        }

        template <usize N, typename T>
        constexpr Vector<N, T> Vector<N, T>::operator/(T scalar) const
        {
            Vector result;
            for (usize i = 0; i < N; ++i) result.Data[i] = Data[i] / scalar;
            return result;
        }

        template <usize N, typename T>
        constexpr Vector<N, T> Vector<N, T>::operator-() const
        {
            Vector result;
            for (usize i = 0; i < N; ++i) result.Data[i] = -Data[i];
            return result;
        }

        // ---------------------
        // Compound ops
        // ---------------------
        template <usize N, typename T>
        constexpr Vector<N, T>& Vector<N, T>::operator+=(const Vector& rhs)
        {
            for (usize i = 0; i < N; ++i) Data[i] += rhs.Data[i];
            return *this;
        }

        template <usize N, typename T>
        constexpr Vector<N, T>& Vector<N, T>::operator-=(const Vector& rhs)
        {
            for (usize i = 0; i < N; ++i) Data[i] -= rhs.Data[i];
            return *this;
        }

        template <usize N, typename T>
        constexpr Vector<N, T>& Vector<N, T>::operator*=(const Vector& rhs)
        {
            for (usize i = 0; i < N; ++i) Data[i] *= rhs.Data[i];
            return *this;
        }

        template <usize N, typename T>
        constexpr Vector<N, T>& Vector<N, T>::operator/=(const Vector& rhs)
        {
            for (usize i = 0; i < N; ++i) Data[i] /= rhs.Data[i];
            return *this;
        }

        template <usize N, typename T>
        constexpr Vector<N, T>& Vector<N, T>::operator*=(T scalar)
        {
            for (usize i = 0; i < N; ++i) Data[i] *= scalar;
            return *this;
        }

        template <usize N, typename T>
        constexpr Vector<N, T>& Vector<N, T>::operator/=(T scalar)
        {
            for (usize i = 0; i < N; ++i) Data[i] /= scalar;
            return *this;
        }

        // ---------------------
        // Math
        // ---------------------
        template <usize N, typename T>
        constexpr T Vector<N, T>::Dot(const Vector& rhs) const
        {
            T sum = T(0);
            for (usize i = 0; i < N; ++i) sum += Data[i] * rhs.Data[i];
            return sum;
        }

        template <usize N, typename T>
        template <usize M>
        constexpr EnableIfType<M == 3, struct Vector<N, T>>
        Vector<N, T>::Cross(const Vector& rhs) const
        {
            return Vector{Data[1] * rhs.Data[2] - Data[2] * rhs.Data[1],
                          Data[2] * rhs.Data[0] - Data[0] * rhs.Data[2],
                          Data[0] * rhs.Data[1] - Data[1] * rhs.Data[0]};
        }

        template <usize N, typename T>
        constexpr T Vector<N, T>::LengthSquared() const
        {
            return Dot(*this);
        }

#if PRISM_TARGET_CRYPTIX == 0
        template <usize N, typename T>
        T Vector<N, T>::Length() const
        {
            return Math::SquareRoot(LengthSquared());
        }

        template <usize N, typename T>
        Vector<N, T> Vector<N, T>::Normalized() const
        {
            T len = Length();
            assert(len != T(0));
            return *this / len;
        }

        template <usize N, typename T>
        void Vector<N, T>::Normalize()
        {
            T len = Length();
            assert(len != T(0));
            *this /= len;
        }
#endif

        // ---------------------
        // Iterators
        // ---------------------
        template <usize N, typename T>
        constexpr T* Vector<N, T>::Begin()
        {
            return Data;
        }

        template <usize N, typename T>
        constexpr const T* Vector<N, T>::Begin() const
        {
            return Data;
        }

        template <usize N, typename T>
        constexpr T* Vector<N, T>::End()
        {
            return Data + N;
        }

        template <usize N, typename T>
        constexpr const T* Vector<N, T>::End() const
        {
            return Data + N;
        }

        // ---------------------
        // Swizzle accessors
        // ---------------------
        template <usize N, typename T>
        constexpr T& Vector<N, T>::X()
        {
            static_assert(N >= 1);
            return Data[0];
        }

        template <usize N, typename T>
        constexpr T& Vector<N, T>::Y()
        {
            static_assert(N >= 2);
            return Data[1];
        }

        template <usize N, typename T>
        constexpr T& Vector<N, T>::Z()
        {
            static_assert(N >= 3);
            return Data[2];
        }

        template <usize N, typename T>
        constexpr T& Vector<N, T>::W()
        {
            static_assert(N >= 4);
            return Data[3];
        }

        template <usize N, typename T>
        constexpr const T& Vector<N, T>::X() const
        {
            static_assert(N >= 1);
            return Data[0];
        }

        template <usize N, typename T>
        constexpr const T& Vector<N, T>::Y() const
        {
            static_assert(N >= 2);
            return Data[1];
        }

        template <usize N, typename T>
        constexpr const T& Vector<N, T>::Z() const
        {
            static_assert(N >= 3);
            return Data[2];
        }

        template <usize N, typename T>
        constexpr const T& Vector<N, T>::W() const
        {
            static_assert(N >= 4);
            return Data[3];
        }

        // ---------------------
        // Scalar * Vector
        // ---------------------
        template <usize N, typename T>
        constexpr Vector<N, T> operator*(T scalar, const Vector<N, T>& v)
        {
            return v * scalar;
        }
    }; // namespace Math
}; // namespace Prism
