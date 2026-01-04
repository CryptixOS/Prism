/*
 * Created by v1tr10l7 on 07.11.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Core.hpp>

namespace Prism
{
    namespace Math
    {
        template <usize N, typename T>
        struct Vector;
        template <usize N, typename T>
        constexpr Vector<N, T> operator*(T scalar, const Vector<N, T>& v);

        template <usize N, typename T>
        struct Vector
        {
            T Data[N]{};

            // Constructors
            constexpr Vector();
            constexpr explicit Vector(T value);
            constexpr Vector(InitializerList<T> values);

            // Assignment
            constexpr Vector&  operator=(InitializerList<T> values);

            // Access
            constexpr T&       operator[](usize i);
            constexpr const T& operator[](usize i) const;

            // Comparison
            constexpr bool     operator==(const Vector& other) const;
            constexpr bool     operator!=(const Vector& other) const;

            // Arithmetic
            constexpr Vector   operator+(const Vector& rhs) const;
            constexpr Vector   operator-(const Vector& rhs) const;
            constexpr Vector   operator*(const Vector& rhs) const;
            constexpr Vector   operator/(const Vector& rhs) const;
            constexpr Vector   operator*(T scalar) const;
            constexpr Vector   operator/(T scalar) const;
            constexpr Vector   operator-() const;

            // Compound assignment
            constexpr Vector&  operator+=(const Vector& rhs);
            constexpr Vector&  operator-=(const Vector& rhs);
            constexpr Vector&  operator*=(const Vector& rhs);
            constexpr Vector&  operator/=(const Vector& rhs);
            constexpr Vector&  operator*=(T scalar);
            constexpr Vector&  operator/=(T scalar);

            // Math
            constexpr T        Dot(const Vector& rhs) const;
            template <usize M = N>
            constexpr EnableIfType<M == 3, Vector>
                        Cross(const Vector& rhs) const;
            constexpr T LengthSquared() const;

#if PRISM_TARGET_CRYPTIX == 0
            T      Length() const;
            Vector Normalized() const;
            void   Normalize();
#endif

            // Iterators
            constexpr T*       Begin();
            constexpr const T* Begin() const;
            constexpr T*       End();
            constexpr const T* End() const;

            // Swizzle accessors
            constexpr T&       X();
            constexpr T&       Y();
            constexpr T&       Z();
            constexpr T&       W();
            constexpr const T& X() const;
            constexpr const T& Y() const;
            constexpr const T& Z() const;
            constexpr const T& W() const;
        };

        template <usize N, typename T>
        constexpr Vector<N, T> operator*(T scalar, const Vector<N, T>& v);
    }; // namespace Math
}; // namespace Prism

#include <Prism/Math/Vector.inl>
