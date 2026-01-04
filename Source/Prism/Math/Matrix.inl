/*
 * Created by v1tr10l7 on 07.11.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Math/Matrix.hpp>

namespace Prism
{
    namespace Math
    {
        // ------------------------------
        // Constructors / assignment
        // ------------------------------
        template <usize C, usize R, typename T>
        constexpr Matrix<C, R, T>::Matrix(
            InitializerList<InitializerList<T>> columns)
        {
            assert(columns.size() == C && "Initializer must have C columns");
            usize c = 0;
            for (auto const& col : columns)
            {
                assert(col.size() == R
                       && "Each column must have exactly R elements");
                usize r = 0;
                for (auto const& v : col) Columns[c][r++] = v;
                ++c;
            }
        }

        template <usize C, usize R, typename T>
        constexpr Matrix<C, R, T>::Matrix(InitializerList<Vector<R, T>> columns)
        {
            assert(columns.size() == C && "Initializer must have C columns");
            usize c = 0;
            for (auto const& vec : columns) { Columns[c++] = vec; }
        }

        template <usize C, usize R, typename T>
        constexpr Matrix<C, R, T>&
        Matrix<C, R, T>::operator=(InitializerList<InitializerList<T>> columns)
        {
            assert(columns.size() == C && "Initializer must have C columns");
            usize c = 0;
            for (auto const& col : columns)
            {
                assert(col.size() == R
                       && "Each column must have exactly R elements");
                usize r = 0;
                for (auto const& v : col) Columns[c][r++] = v;
                ++c;
            }
            return *this;
        }

        template <usize C, usize R, typename T>
        constexpr Matrix<C, R, T>&
        Matrix<C, R, T>::operator=(InitializerList<Vector<R, T>> columns)
        {
            assert(columns.size() == C && "Initializer must have C columns");
            usize c = 0;
            for (auto const& vec : columns) Columns[c++] = vec;
            return *this;
        }

        // ------------------------------
        // Accessors
        // ------------------------------
        template <usize C, usize R, typename T>
        constexpr Vector<R, T>& Matrix<C, R, T>::operator[](usize col)
        {
            assert(col < C);
            return Columns[col];
        }

        template <usize C, usize R, typename T>
        constexpr const Vector<R, T>&
        Matrix<C, R, T>::operator[](usize col) const
        {
            assert(col < C);
            return Columns[col];
        }

        template <usize C, usize R, typename T>
        constexpr T& Matrix<C, R, T>::operator()(usize row, usize col)
        {
            assert(row < R && col < C);
            return Columns[col][row];
        }

        template <usize C, usize R, typename T>
        constexpr const T& Matrix<C, R, T>::operator()(usize row,
                                                       usize col) const
        {
            assert(row < R && col < C);
            return Columns[col][row];
        }

        // ------------------------------
        // Comparison
        // ------------------------------
        template <usize C, usize R, typename T>
        constexpr bool Matrix<C, R, T>::operator==(const Matrix& other) const
        {
            for (usize c = 0; c < C; ++c)
                if (Columns[c] != other.Columns[c]) return false;
            return true;
        }

        template <usize C, usize R, typename T>
        constexpr bool Matrix<C, R, T>::operator!=(const Matrix& other) const
        {
            return !(*this == other);
        }

        // ------------------------------
        // Arithmetic elementwise
        // ------------------------------
        template <usize C, usize R, typename T>
        constexpr Matrix<C, R, T>
        Matrix<C, R, T>::operator+(const Matrix& rhs) const
        {
            Matrix result;
            for (usize c = 0; c < C; ++c)
                result.Columns[c] = Columns[c] + rhs.Columns[c];
            return result;
        }

        template <usize C, usize R, typename T>
        constexpr Matrix<C, R, T>
        Matrix<C, R, T>::operator-(const Matrix& rhs) const
        {
            Matrix result;
            for (usize c = 0; c < C; ++c)
                result.Columns[c] = Columns[c] - rhs.Columns[c];
            return result;
        }

        template <usize C, usize R, typename T>
        constexpr Matrix<C, R, T>&
        Matrix<C, R, T>::operator+=(const Matrix& rhs)
        {
            for (usize c = 0; c < C; ++c) Columns[c] += rhs.Columns[c];
            return *this;
        }

        template <usize C, usize R, typename T>
        constexpr Matrix<C, R, T>&
        Matrix<C, R, T>::operator-=(const Matrix& rhs)
        {
            for (usize c = 0; c < C; ++c) Columns[c] -= rhs.Columns[c];
            return *this;
        }

        // ------------------------------
        // Scalar ops
        // ------------------------------
        template <usize C, usize R, typename T>
        constexpr Matrix<C, R, T> Matrix<C, R, T>::operator*(T scalar) const
        {
            Matrix result;
            for (usize c = 0; c < C; ++c)
                result.Columns[c] = Columns[c] * scalar;
            return result;
        }

        template <usize C, usize R, typename T>
        constexpr Matrix<C, R, T> Matrix<C, R, T>::operator/(T scalar) const
        {
            Matrix result;
            for (usize c = 0; c < C; ++c)
                result.Columns[c] = Columns[c] / scalar;
            return result;
        }

        template <usize C, usize R, typename T>
        constexpr Matrix<C, R, T>& Matrix<C, R, T>::operator*=(T scalar)
        {
            for (usize c = 0; c < C; ++c) Columns[c] *= scalar;
            return *this;
        }

        template <usize C, usize R, typename T>
        constexpr Matrix<C, R, T>& Matrix<C, R, T>::operator/=(T scalar)
        {
            for (usize c = 0; c < C; ++c) Columns[c] /= scalar;
            return *this;
        }

        // ------------------------------
        // Matrix × Vector (vector is length C) => resulting column vector of
        // length R result = sum_k Columns[k] * rhs[k]
        //
        // Note: Vector<T, N>::operator*(scalar) returns vector scaled.
        // ------------------------------
        template <usize C, usize R, typename T>
        constexpr Vector<R, T>
        Matrix<C, R, T>::operator*(const Vector<C, T>& rhs) const
        {
            Vector<R, T> result; // initially zeros
            for (usize k = 0; k < C; ++k) result += Columns[k] * rhs[k];
            return result;
        }

        // ------------------------------
        // Matrix × Matrix
        // Compute each column j of result as A * (column j of B)
        // ------------------------------
        template <usize C, usize R, typename T>
        constexpr Matrix<C, R, T>
        Matrix<C, R, T>::operator*(const Matrix& rhs) const
        {
            Matrix result;
            for (usize j = 0; j < C; ++j)
            {
                // rhs.Columns[j] is a Vector<R, T>? careful: rhs.Columns
                // size = C, each column length R. For multiplication, A
                // (R x C) * B (R x C) is only valid when C ==
                // R. But general multiplication (R x C) * (Cols2 x
                // Rows2) isn't supported by single template. Here we interpret
                // multiplication of two matrices with the same template as
                // usual square-multiplication when C==R. A general
                // multiply A( R x C ) * B( C x K ) would result in
                // (R x K); we only support C==rhs.R for now.
            }
            // Implement generic multiplication only if rhs has the same
            // C/R template shape: We can multiply when this->kCols ==
            // rhs.kRows (i.e., C == R).
            static_assert(
                C == R,
                "Matrix multiplication requires square matrices in this "
                "operator overload; use Matrix<NewCols, NewRows, T> "
                "specialized operator for general shapes.");
            // For square matrix multiplication:
            Matrix res;
            for (usize j = 0; j < C; ++j)
            {
                // Build result column j:
                // result.Columns[j] = (*this) * rhs.Columns[j]  (Matrix *
                // Vector<C>)
                res.Columns[j] = (*this) * rhs.Columns[j];
            }
            return res;
        }

        // ------------------------------
        // Transpose
        // Returns Matrix<R, C, T>
        // ------------------------------
        template <usize C, usize R, typename T>
        constexpr Matrix<R, C, T> Matrix<C, R, T>::Transpose() const
        {
            Matrix<R, C, T> t;
            for (usize r = 0; r < R; ++r)
                for (usize c = 0; c < C; ++c) t.Columns[r][c] = Columns[c][r];
            return t;
        }

        // ------------------------------
        // MinorMatrix: build (N-1)x(N-1) matrix by removing skipRow, skipCol
        // Only available when square and N>1
        // ------------------------------
        template <usize C, usize R, typename T>
        template <usize N>
        constexpr Matrix<N - 1, N - 1, T>
        Matrix<C, R, T>::MinorMatrix(usize skipRow, usize skipCol) const
            requires(N == C && N > 1)
        {
            Matrix<N - 1, N - 1, T> m;
            usize                   tc = 0;
            for (usize c = 0; c < N; ++c)
            {
                if (c == skipCol) continue;
                usize tr = 0;
                for (usize r = 0; r < N; ++r)
                {
                    if (r == skipRow) continue;
                    m.Columns[tc][tr++] = Columns[c][r];
                }
                ++tc;
            }
            return m;
        }

        // ------------------------------
        // Determinant via Laplace expansion (recursive).
        // We expand along row 0 for convenience.
        // ------------------------------
        template <usize C, usize R, typename T>
        constexpr T Matrix<C, R, T>::Determinant() const
            requires(C == R)
        {
            constexpr usize N = R;

            if constexpr (N == 0) return T(1);
            else if constexpr (N == 1) return Columns[0][0];
            else if constexpr (N == 2)
            {
                T a = Columns[0][0];
                T c = Columns[0][1];
                T b = Columns[1][0];
                T d = Columns[1][1];
                return a * d - b * c;
            }
            else // <--- Add this else block!
            {
                T det = T(0);
                for (usize col = 0; col < N; ++col)
                {
                    T    coeff  = Columns[col][0];
                    T    sign   = ((col & 1) ? T(-1) : T(1));

                    // Now the compiler knows this only exists for N > 2
                    auto minor  = MinorMatrix<N>(0, col);
                    T    subdet = minor.Determinant();
                    det += sign * coeff * subdet;
                }
                return det;
            }
        }
        // ------------------------------
        // Cofactor Matrix (matrix of cofactors), used in adjugate/inverse
        // ------------------------------
        template <usize C, usize R, typename T>
        template <usize N>
        constexpr Matrix<N, N, T> Matrix<C, R, T>::CofactorMatrix() const
            requires(N == C && N > 0)
        {
            Matrix<N, N, T> cof;
            for (usize r = 0; r < N; ++r)
            {
                for (usize c = 0; c < N; ++c)
                {
                    auto minor        = MinorMatrix<N>(r, c);
                    T    subdet       = minor.Determinant();
                    T    sign         = (((r + c) & 1) ? T(-1) : T(1));
                    // Cofactor is sign * subdet; store at (r, c) in
                    // column-major: cof.Columns[c][r]
                    cof.Columns[c][r] = sign * subdet;
                }
            }
            return cof;
        }

        // ------------------------------
        // Inverse: inverse = adjugate / det
        // adjugate = transpose(cofactor matrix)
        // ------------------------------
        template <usize C, usize R, typename T>
        constexpr Matrix<C, R, T> Matrix<C, R, T>::Inverse() const
            requires(C == R)
        {
            constexpr usize N = R;
            static_assert(N > 0, "Cannot invert empty matrix");

            T det = Determinant();
            assert(
                det != T(0)
                && "Matrix is singular (determinant is zero), cannot invert");

            auto cof = CofactorMatrix<N>();
            auto adj
                = cof.Transpose(); // adjugate is transpose of cofactor matrix
            return adj / det;
        }

    }; // namespace Math
}; // namespace Prism
