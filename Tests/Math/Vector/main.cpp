/*
 * Created by v1tr10l7 on 07.11.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Debug/Assertions.hpp>
#include <Prism/Math/Vector.hpp>

#include <iostream>

using namespace Prism;

int main()
{
    Math::Vector<3, f32> a{1.0f, 2.0f, 3.0f};
    Math::Vector<3, f32> b{3.0f, 4.0f, 5.0f};

    // Basic arithmetic
    PrismAssert((a + b) == Math::Vector<3, f32>{4, 6, 8});
    PrismAssert((b - a) == Math::Vector<3, f32>{2, 2, 2});
    PrismAssert((a * 2.0f) == Math::Vector<3, f32>{2, 4, 6});
    PrismAssert((b / 2.0f) == Math::Vector<3, f32>{1.5f, 2.0f, 2.5f});

    // Dot product
    PrismAssert(a.Dot(b) == 26.0f);

    // Cross product
    auto c = a.Cross(b);
    PrismAssert(c == Math::Vector<3, f32>{-2, 4, -2});

    // Length and normalization
    PrismAssert(a.LengthSquared() == 14.0f);
    auto val = a.Length() - Math::SquareRoot<f64>(14.0f);
    auto abs = Math::Absolute<f64>(val);
    PrismAssert(abs < 1e-5);
    auto n = a.Normalized();
    PrismAssert(Math::Absolute(n.Length() - 1.0f) < 1e-5);

    // Compound operations
    a += b;
    PrismAssert(a == Math::Vector<3, f32>{4, 6, 8});

    a -= b;
    PrismAssert(a == Math::Vector<3, f32>{1, 2, 3});

    // Equality
    PrismAssert(a != b);

    // Static SquareRoot
    PrismAssert(Math::SquareRoot<f32>(9.0f) == 3.0f);

    // 2D test
    constexpr f64        Epsilon = 1e-12;
    Math::Vector<2, f64> v2{3.0, 4.0};
    PrismAssert(Math::Absolute(v2.Length() - 5.0) < Epsilon);
    PrismAssert(static_cast<f64>(v2.Length()) == static_cast<f64>(5.0));
    v2.Normalize();
    PrismAssert(Math::Absolute(v2.Length() - 1.0) < 1e-10);

    return 0;
}
