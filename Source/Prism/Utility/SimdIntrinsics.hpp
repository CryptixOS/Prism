/*
 * Created by v1tr10l7 on 08.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Platform.hpp>
#include <Prism/Core/TypeTraits.hpp>
#include <Prism/Core/Types.hpp>

#if PRISM_TARGET_CRYPTIX != 0
    #error 'You should not use simd inside the kernel'
#endif

namespace Prism
{
    using i8x2   = i8 __attribute__((vector_size(2)));
    using i8x4   = i8 __attribute__((vector_size(4)));
    using i8x8   = i8 __attribute__((vector_size(8)));
    using i8x16  = i8 __attribute__((vector_size(16)));
    using i8x32  = i8 __attribute__((vector_size(32)));

    using i16x2  = i16 __attribute__((vector_size(4)));
    using i16x4  = i16 __attribute__((vector_size(8)));
    using i16x8  = i16 __attribute__((vector_size(16)));
    using i16x16 = i16 __attribute__((vector_size(32)));

    using c8x2   = char __attribute__((vector_size(2)));
    using c8x4   = char __attribute__((vector_size(4)));
    using c8x8   = char __attribute__((vector_size(8)));
    using c8x16  = char __attribute__((vector_size(16)));
    using c8x32  = char __attribute__((vector_size(32)));

    using i32x2  = i32 __attribute__((vector_size(8)));
    using i32x4  = i32 __attribute__((vector_size(16)));
    using i32x8  = i32 __attribute__((vector_size(32)));

    using i64x2  = i64 __attribute__((vector_size(16)));
    using i64x4  = i64 __attribute__((vector_size(32)));

    using u8x2   = u8 __attribute__((vector_size(2)));
    using u8x4   = u8 __attribute__((vector_size(4)));
    using u8x8   = u8 __attribute__((vector_size(8)));
    using u8x16  = u8 __attribute__((vector_size(16)));
    using u8x32  = u8 __attribute__((vector_size(32)));

    using u16x2  = u16 __attribute__((vector_size(4)));
    using u16x4  = u16 __attribute__((vector_size(8)));
    using u16x8  = u16 __attribute__((vector_size(16)));
    using u16x16 = u16 __attribute__((vector_size(32)));

    using u32x2  = u32 __attribute__((vector_size(8)));
    using u32x4  = u32 __attribute__((vector_size(16)));
    using u32x8  = u32 __attribute__((vector_size(32)));

    using u64x2  = u64 __attribute__((vector_size(16)));
    using u64x4  = u64 __attribute__((vector_size(32)));

    using f32x2  = f32 __attribute__((vector_size(8)));
    using f32x4  = f32 __attribute__((vector_size(16)));
    using f32x8  = f32 __attribute__((vector_size(32)));

    using f64x2  = f64 __attribute__((vector_size(16)));
    using f64x4  = f64 __attribute__((vector_size(32)));

    template <typename T>
    concept SIMDVector = requires { __builtin_convertvector(DeclVal<T>(), T); };

    static_assert(IsSameV<i8, signed char>);

    template <typename T>
    using ElementOf = RemoveReference<decltype(DeclVal<T>()[0])>;
    template <typename T>
    using ElementOfType = typename ElementOf<T>::Type;

    static_assert(IsSameV<ElementOfType<i8x4>, i8>);
    static_assert(IsSameV<ElementOfType<f32x4>, f32>);

    template <SIMDVector V>
    constexpr static usize VectorLength = sizeof(V) / sizeof(ElementOfType<V>);

    static_assert(VectorLength<i8x4> == 4);
    static_assert(VectorLength<f32x4> == 4);

    template <SIMDVector T, SIMDVector U>
        requires(VectorLength<T> == VectorLength<U>)
    PM_ALWAYS_INLINE constexpr T SimdCast(U v)
    {
        return __builtin_convertvector(v, T);
    }

    template <typename T>
    struct IndexVectorFor;
    template <SIMDVector T>
        requires(IsIntegralV<ElementOfType<T>>)
    struct IndexVectorFor<T>
    {
        using Type = T;
    };
    template <SIMDVector T>
        requires(IsFloatingPointV<ElementOfType<T>>)
    struct IndexVectorFor<T>
    {
        using Type
            = ConditionalType<IsSameV<ElementOfType<T>, f32>,
                              u32 __attribute__((vector_size(sizeof(T)))),
                              u64 __attribute__((vector_size(sizeof(T))))>;
    };

    template <SIMDVector T>
    using IndexVectorForType = typename IndexVectorFor<T>::Type;

    static_assert(IsSameV<IndexVectorForType<i8x16>, i8x16>);
    static_assert(IsSameV<IndexVectorForType<u32x4>, u32x4>);
    static_assert(IsSameV<IndexVectorForType<u64x4>, u64x4>);

    // static_assert(IsSameV<IndexVectorForType<f32x4>, u32x4>);
    // static_assert(IsSameV<IndexVectorForType<f64x4>, u64x4>);
#if PRISM_SIMD_SSE2_PRESENT
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
#elif PRISM_SIMD_NEON_PRESENT
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
}; // namespace Prism
