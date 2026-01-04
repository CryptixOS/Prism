/*
 * Created by v1tr10l7 on 07.11.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Math/Vector.hpp>

namespace Prism
{
    namespace Math
    {
        template <usize C, usize R, typename T>
        struct Matrix
        {
            using ValueType                    = T;
            static constexpr usize ColumnCount = C;
            static constexpr usize RowCount    = R;

            Vector<R, T>           Columns[C]{};

            // Constructors
            constexpr Matrix() = default;
            constexpr Matrix(InitializerList<InitializerList<T>> columns);

            // Init from vector-of-columns
            constexpr Matrix(InitializerList<Vector<R, T>> columns);

            constexpr Matrix&
            operator=(InitializerList<InitializerList<T>> columns);
            constexpr Matrix& operator=(InitializerList<Vector<R, T>> columns);

            // Column access
            constexpr Vector<R, T>&       operator[](usize col);
            constexpr const Vector<R, T>& operator[](usize col) const;

            // Element access (row, col)
            constexpr T&                  operator()(usize row, usize col);
            constexpr const T&        operator()(usize row, usize col) const;

            // Comparison
            constexpr bool            operator==(const Matrix& other) const;
            constexpr bool            operator!=(const Matrix& other) const;

            // Arithmetic (elementwise)
            constexpr Matrix          operator+(const Matrix& rhs) const;
            constexpr Matrix          operator-(const Matrix& rhs) const;

            constexpr Matrix&         operator+=(const Matrix& rhs);
            constexpr Matrix&         operator-=(const Matrix& rhs);

            // Scalar
            constexpr Matrix          operator*(T scalar) const;
            constexpr Matrix          operator/(T scalar) const;
            constexpr Matrix&         operator*=(T scalar);
            constexpr Matrix&         operator/=(T scalar);

            // Matrix × Vector (vector is column vector of length C -> result
            // length R)
            constexpr Vector<R, T>    operator*(const Vector<C, T>& rhs) const;

            // Matrix × Matrix
            constexpr Matrix          operator*(const Matrix& rhs) const;

            // Transpose (R x C -> C x R)
            constexpr Matrix<R, C, T> Transpose() const;

            // Determinant (Laplace expansion). Only available for square
            // matrices.
            constexpr T               Determinant() const
                requires(C == R);

            // Inverse. Only available for square matrices.
            // Throws assertion if determinant is zero (non-invertible).
            constexpr Matrix Inverse() const
                requires(C == R);

          private:
            // Helper: build minor by removing row 'skipRow' and column
            // 'skipCol'
            template <usize N = R>
            constexpr Matrix<N - 1, N - 1, T> MinorMatrix(usize skipRow,
                                                          usize skipCol) const
                requires(N == C && N > 1);

            // Cofactor helper (used for adjugate)
            template <usize N = R>
            constexpr Matrix<N, N, T> CofactorMatrix() const
                requires(N == C && N > 0);
        };
    }; // namespace Math
} // namespace Prism

#include <Prism/Math/Matrix.inl>
