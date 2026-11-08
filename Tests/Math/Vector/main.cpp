/*
 * Created by v1tr10l7 on 07.11.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Debug/Assertions.hpp>
#include <Prism/Math/Vector.hpp>

#include <iostream>

using namespace Prism;

TEST_CASE(vector_math)
{
    Math::Vector<3, f32> a{1.0f, 2.0f, 3.0f};
    Math::Vector<3, f32> b{3.0f, 4.0f, 5.0f};

    // Basic arithmetic
    EXPECT((a + b) == Math::Vector<3, f32>{4, 6, 8});
    EXPECT((b - a) == Math::Vector<3, f32>{2, 2, 2});
    EXPECT((a * 2.0f) == Math::Vector<3, f32>{2, 4, 6});
    EXPECT((b / 2.0f) == Math::Vector<3, f32>{1.5f, 2.0f, 2.5f});

    // Dot product
    EXPECT_EQ(a.Dot(b), 26.0f);

    // Cross product
    auto c = a.Cross(b);
    EXPECT(c == Math::Vector<3, f32>{-2, 4, -2});

    // Length and normalization
    EXPECT_EQ(a.LengthSquared(), 14.0f);
    auto val = a.Length() - Math::SquareRoot<f64>(14.0f);
    auto abs = Math::Absolute<f64>(val);
    EXPECT(abs < 1e-5);
    auto n = a.Normalized();
    EXPECT(Math::Absolute(n.Length() - 1.0f) < 1e-5);

    // Compound operations
    a += b;
    EXPECT(a == Math::Vector<3, f32>{4, 6, 8});

    a -= b;
    EXPECT(a == Math::Vector<3, f32>{1, 2, 3});

    // Equality
    EXPECT_NE(a, b);

    // Static SquareRoot
    EXPECT(Math::SquareRoot<f32>(9.0f) == 3.0f);

    // 2D test
    constexpr f64        Epsilon = 1e-12;
    Math::Vector<2, f64> v2{3.0, 4.0};
    EXPECT(Math::Absolute(v2.Length() - 5.0) < Epsilon);
    EXPECT(static_cast<f64>(v2.Length()) == static_cast<f64>(5.0));
    v2.Normalize();
    EXPECT(Math::Absolute(v2.Length() - 1.0) < 1e-10);
}
