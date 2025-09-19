/*
 * Created by v1tr10l7 on 09.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Utility/SimdIntrinsics.hpp>
#include <cassert>
#include <cmath>
#include <iostream>

using namespace Prism;

void TestSimdFloatAdd()
{
    alignas(16) float a[FloatWidth];
    alignas(16) float b[FloatWidth];
    alignas(16) float out[FloatWidth];

    for (usize i = 0; i < FloatWidth; ++i)
    {
        a[i] = static_cast<float>(i);
        b[i] = static_cast<float>(2 * i);
    }

    simd_float va = LoadFloat(a);
    simd_float vb = LoadFloat(b);
    simd_float vc = AddFloat(va, vb);
    StoreFloat(out, vc);

    for (usize i = 0; i < FloatWidth; ++i)
        assert(std::fabs(out[i] - (a[i] + b[i])) < 1e-5f);
}

void TestSimdFloatSub()
{
    alignas(16) float a[FloatWidth];
    alignas(16) float b[FloatWidth];
    alignas(16) float out[FloatWidth];

    for (usize i = 0; i < FloatWidth; ++i)
    {
        a[i] = static_cast<float>(i + 5);
        b[i] = static_cast<float>(i);
    }

    simd_float va = LoadFloat(a);
    simd_float vb = LoadFloat(b);
    simd_float vc = SubFloat(va, vb);
    StoreFloat(out, vc);

    for (usize i = 0; i < FloatWidth; ++i)
        assert(std::fabs(out[i] - (a[i] - b[i])) < 1e-5f);
}

void TestSimdFloatMul()
{
    alignas(16) float a[FloatWidth];
    alignas(16) float b[FloatWidth];
    alignas(16) float out[FloatWidth];

    for (usize i = 0; i < FloatWidth; ++i)
    {
        a[i] = static_cast<float>(i);
        b[i] = 2.0f;
    }

    simd_float va = LoadFloat(a);
    simd_float vb = LoadFloat(b);
    simd_float vc = MulFloat(va, vb);
    StoreFloat(out, vc);

    for (usize i = 0; i < FloatWidth; ++i)
        assert(std::fabs(out[i] - (a[i] * b[i])) < 1e-5f);
}

void TestSimdFloatDiv()
{
    alignas(16) float a[FloatWidth];
    alignas(16) float b[FloatWidth];
    alignas(16) float out[FloatWidth];

    for (usize i = 0; i < FloatWidth; ++i)
    {
        a[i] = static_cast<float>(i + 1);
        b[i] = 2.0f;
    }

    simd_float va = LoadFloat(a);
    simd_float vb = LoadFloat(b);
    simd_float vc = DivFloat(va, vb);
    StoreFloat(out, vc);

    for (usize i = 0; i < FloatWidth; ++i)
        assert(std::fabs(out[i] - (a[i] / b[i])) < 1e-5f);
}

void TestSimdFloatSet1()
{
    float      out[FloatWidth];
    float      val = 3.14159f;
    simd_float v   = Set1Float(val);
    StoreFloat(out, v);

    for (usize i = 0; i < FloatWidth; ++i)
        assert(std::fabs(out[i] - val) < 1e-5f);
}

int main()
{
    TestSimdFloatAdd();
    TestSimdFloatSub();
    TestSimdFloatMul();
    TestSimdFloatDiv();
    TestSimdFloatSet1();
    std::cout << "SIMD intrinsic tests passed.\n";
    return 0;
}
