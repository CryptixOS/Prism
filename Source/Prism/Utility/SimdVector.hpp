/*
 * Created by v1tr10l7 on 08.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Concepts.hpp>

namespace Prism
{
    namespace Simd
    {
        template <ArithmeticType T, usize N>
        class Vector
        {
          public:
            using ValueType                = T;

            constexpr Vector() PM_NOEXCEPT = default;
            template <typename U>
            Vector(U&& value) PM_NOEXCEPT;

            Vector(const Vector& other) PM_NOEXCEPT = default;
            Vector(Vector&& other)                  = default;

            static constexpr usize Size() PM_NOEXCEPT { return N; }

            // Raw access
            constexpr const T*     Raw() const noexcept { return m_Data; }
            constexpr T*           Raw() noexcept { return m_Data; }

            constexpr T&           operator[](usize i)
            {
                assert(i < N);

                return m_Data[i];
            }
            constexpr const T& operator[](usize i) const
            {
                assert(i < N);

                return m_Data[i];
            }

            // Arithmetic operations
            constexpr Vector operator+(const Vector& rhs) const
            {
                Vector result;
                for (usize i = 0; i < N; ++i) result[i] = m_Data[i] + rhs[i];
                return result;
            }

            constexpr Vector operator-(const Vector& rhs) const
            {
                Vector result;
                for (usize i = 0; i < N; ++i) result[i] = m_Data[i] - rhs[i];
                return result;
            }

            constexpr Vector operator*(const Vector& rhs) const
            {
                Vector result;
                for (usize i = 0; i < N; ++i) result[i] = m_Data[i] * rhs[i];
                return result;
            }

            constexpr Vector operator/(const Vector& rhs) const
            {
                Vector result;
                for (usize i = 0; i < N; ++i) result[i] = m_Data[i] / rhs[i];
                return result;
            }

            // Comparison (returns mask vector in future SIMD-aware version)
            constexpr Vector<bool, N> operator==(const Vector& rhs) const
            {
                Vector<bool, N> result;
                for (usize i = 0; i < N; ++i) result[i] = m_Data[i] == rhs[i];
                return result;
            }

            // Load/store
            constexpr void Load(const T* src)
            {
                for (usize i = 0; i < N; ++i) m_Data[i] = src[i];
            }

            constexpr void Store(T* dst) const
            {
                for (usize i = 0; i < N; ++i) dst[i] = m_Data[i];
            }

          private:
            T m_Data[N]{};
        };
    }; // namespace Simd
}; // namespace Prism
