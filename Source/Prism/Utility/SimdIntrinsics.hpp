/*
 * Created by v1tr10l7 on 08.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

namespace Prism
{
#if PRISM_TARGET_CRYPTIX == 0
    #if defined(__ARM_NEON) || defined(__ARM_NEON__)
        #include <arm_neon.h>
    using simd_float = float32x4_t;
    constexpr auto LoadFloat(auto ptr) { return vld1q_f32(ptr); }
    constexpr auto StoreFloat(auto ptr, auto value)
    {
        return vst1q_f32(ptr, value);
    }
    constexpr auto  AddFloat(auto a, auto b) { return vaddq_f32(a, b); }
    constexpr auto  SubFloat(auto a, auto b) { return vsubq_f32(a, b); }
    constexpr auto  MulFloat(auto a, auto b) { return vmulq_f32(a, b); }
    constexpr auto  DivFloat(auto a, auto b) { return vdivq_f32(a, b); }
    constexpr auto  Set1Float(auto x) { return vdupq_n_f32(x); }
    constexpr usize FloatWidth = 4;

    #elif defined(__SSE2__)
        #include <emmintrin.h>
    using simd_float = __m128;
    constexpr auto LoadFloat(auto ptr) { return _mm_loadu_ps(ptr); }
    constexpr auto StoreFloat(auto ptr, auto value)
    {
        return _mm_storeu_ps(ptr, value);
    }
    constexpr auto  AddFloat(auto a, auto b) { return _mm_add_ps(a, b); }
    constexpr auto  SubFloat(auto a, auto b) { return _mm_sub_ps(a, b); }
    constexpr auto  MulFloat(auto a, auto b) { return _mm_mul_ps(a, b); }
    constexpr auto  DivFloat(auto a, auto b) { return _mm_div_ps(a, b); }
    constexpr auto  Set1Float(auto x) { return _mm_set1_ps(x); }
    constexpr usize FloatWidth = 4;
    #else
    using simd_float = f32;
    constexpr auto  LoadFloat(auto ptr) { return *ptr; }
    constexpr auto  StoreFloat(auto ptr, auto value) { *ptr = value; }
    constexpr auto  AddFloat(auto a, auto b) { return a + b; }
    constexpr auto  SubFloat(auto a, auto b) { return a - b; }
    constexpr auto  MulFloat(auto a, auto b) { return a * b; }
    constexpr auto  DivFloat(auto a, auto b) { return a / b; }
    constexpr auto  Set1Float(auto x) { return x; }
    constexpr usize FloatWidth = 1;
    #endif
#endif
}; // namespace Prism
