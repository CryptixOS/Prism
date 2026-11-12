/*
 * Created by v1tr10l7 on 27.01.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>
export module Prism.Math.SimdVector;

export import Prism.Utility.SimdIntrinsics;

export extern "C++"
{
    namespace Prism::Math
    {
        template <typename T, usize N>
        class SimdVector;

        template <usize N>
            requires(N % FloatWidth == 0)
        class SimdVector<f32, N>
        {
          public:
            using ElementType                       = f32;
            static constexpr usize Width            = 4;

            PM_ALWAYS_INLINE constexpr SimdVector() = default;

            PM_ALWAYS_INLINE constexpr explicit SimdVector(f32 value)
            {
                for (usize i = 0; i < N; i += FloatWidth)
                    m_Data[i / FloatWidth] = Set1Float(value);
            }

            PM_ALWAYS_INLINE constexpr explicit SimdVector(const f32* ptr)
            {
                Load(ptr);
            }

            PM_ALWAYS_INLINE constexpr void Load(const f32* ptr)
            {
                for (usize i = 0; i < N; i += FloatWidth)
                    m_Data[i / FloatWidth] = LoadFloat(ptr + i);
            }

            PM_ALWAYS_INLINE constexpr void Store(f32* ptr) const
            {
                for (usize i = 0; i < N; i += FloatWidth)
                    StoreFloat(ptr + i, m_Data[i / FloatWidth]);
            }

            // -------- arithmetic --------
            PM_ALWAYS_INLINE friend constexpr SimdVector
            operator+(const SimdVector& a, const SimdVector& b)
            {
                SimdVector r;
                for (usize i = 0; i < ChunkCount; ++i)
                    r.m_Data[i] = AddFloat(a.m_Data[i], b.m_Data[i]);
                return r;
            }

            PM_ALWAYS_INLINE friend constexpr SimdVector
            operator-(const SimdVector& a, const SimdVector& b)
            {
                SimdVector r;
                for (usize i = 0; i < ChunkCount; ++i)
                    r.m_Data[i] = SubFloat(a.m_Data[i], b.m_Data[i]);
                return r;
            }

            PM_ALWAYS_INLINE friend constexpr SimdVector
            operator*(const SimdVector& a, const SimdVector& b)
            {
                SimdVector r;
                for (usize i = 0; i < ChunkCount; ++i)
                    r.m_Data[i] = MulFloat(a.m_Data[i], b.m_Data[i]);
                return r;
            }

            PM_ALWAYS_INLINE friend constexpr SimdVector
            operator/(const SimdVector& a, const SimdVector& b)
            {
                SimdVector r;
                for (usize i = 0; i < ChunkCount; ++i)
                    r.m_Data[i] = DivFloat(a.m_Data[i], b.m_Data[i]);
                return r;
            }

            // -------- scalar access (slow path) --------
            PM_ALWAYS_INLINE constexpr f32 operator[](usize index) const
            {
                assert(index < N);
                alignas(32) f32 tmp[N];
                Store(tmp);
                return tmp[index];
            }

          private:
            static constexpr usize ChunkCount = N / FloatWidth;
            simd_float             m_Data[ChunkCount];
        };
    } // namespace Prism::Math
}
