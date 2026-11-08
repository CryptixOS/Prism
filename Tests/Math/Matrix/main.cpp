/*
 * Created by v1tr10l7 on 07.11.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Test/Test.hpp>
#include <Prism/Debug/Assertions.hpp>
#include <Prism/Math/Matrix.hpp>

using namespace Prism;
using Math::Matrix;

TEST_CASE(TestMatrixBasicConstruction){
    PrismDebug("[TEST] Basic Matrix construction and indexing\n");

    Matrix<2, 3, f64> m = {
        {1.0, 2.0, 3.0}, // column 0
        {4.0, 5.0, 6.0}  // column 1
    };

    EXPECT_EQ((m(0, 0)), 1.0);
    EXPECT_EQ((m(1, 0)), 2.0);
    EXPECT_EQ((m(2, 0)), 3.0);
    EXPECT_EQ((m(0, 1)), 4.0);
    EXPECT_EQ((m(1, 1)), 5.0);
    EXPECT_EQ((m(2, 1)), 6.0);

    auto col0 = m[0];
    EXPECT(col0[0] == 1.0 && col0[1] == 2.0 && col0[2] == 3.0);
}

TEST_CASE(TestMatrixArithmetic){
    PrismDebug("[TEST] Matrix arithmetic (+,-,*,/ scalar)\n");

    Matrix<2, 2, f64> a   = {{1.0, 2.0}, {3.0, 4.0}};

    Matrix<2, 2, f64> b   = {{5.0, 6.0}, {7.0, 8.0}};

    auto              sum = a + b;
    EXPECT(sum(0, 0) == 6.0 && sum(1, 0) == 8.0);
    EXPECT(sum(0, 1) == 10.0 && sum(1, 1) == 12.0);

    auto diff = b - a;
    EXPECT(diff(0, 0) == 4.0 && diff(1, 0) == 4.0);
    EXPECT(diff(0, 1) == 4.0 && diff(1, 1) == 4.0);

    auto scaled = a * 2.0;
    EXPECT(scaled(0, 0) == 2.0 && scaled(1, 0) == 4.0);
    EXPECT(scaled(0, 1) == 6.0 && scaled(1, 1) == 8.0);

    a *= 3.0;
    EXPECT(a(0, 0) == 3.0 && a(1, 1) == 12.0);
}

TEST_CASE(TestMatrixVectorMultiplication){
    PrismDebug("[TEST] Matrix × Vector multiplication\n");

    Matrix<2, 3, f64>    m   = {{1.0, 0.0, 0.0}, {0.0, 2.0, 0.0}};

    Math::Vector<2, f64> v   = {4.0, 5.0};
    auto                 res = m * v;

    // result = col0*v[0] + col1*v[1]
    // col0 = (1,0,0), col1 = (0,2,0)
    EXPECT(res[0] == 4.0 && res[1] == 10.0 && res[2] == 0.0);
}

TEST_CASE(TestMatrixTranspose){
    PrismDebug("[TEST] Transpose\n");

    Matrix<3, 2, f64> m = {{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}};

    auto              t = m.Transpose();
    // t is 2x3
    EXPECT(t(0, 0) == 1.0 && t(0, 1) == 2.0);
    EXPECT(t(1, 0) == 3.0 && t(1, 1) == 4.0);
    EXPECT(t(2, 0) == 5.0 && t(2, 1) == 6.0);
}

TEST_CASE(TestMatrixDeterminant2x2){
    PrismDebug("[TEST] Determinant 2x2\n");

    Matrix<2, 2, f64> m   = {{1.0, 2.0}, {3.0, 4.0}};

    f64               det = m.Determinant();
    EXPECT(Math::Absolute(det - (-2.0)) < 1e-12);
}

TEST_CASE(TestMatrixDeterminant3x3){
    PrismDebug("[TEST] Determinant 3x3\n");

    Matrix<3, 3, f64> m = {{1.0, 2.0, 3.0}, {0.0, 1.0, 4.0}, {5.0, 6.0, 0.0}};

    f64 det = m.Determinant(); // expected = 1*(1*0 - 4*6) - 2*(0*0 - 4*5) +
                               // 3*(0*6 - 1*5) = -24 + 40 - 15 = 1
    EXPECT(Math::Absolute(det - 1.0) < 1e-12);
}

TEST_CASE(TestMatrixInverse2x2){
    PrismDebug("[TEST] Inverse 2x2\n");

    Matrix<2, 2, f64> m   = {{4.0, 7.0}, {2.0, 6.0}};

    auto              inv = m.Inverse();
    // Expected inverse = (1/(4*6-7*2)) * [6 -7; -2 4]
    // determinant = 10
    EXPECT(Math::Absolute(inv(0, 0) - 0.6) < 1e-12);
    EXPECT(Math::Absolute(inv(0, 1) - -0.2) < 1e-12);
    EXPECT(Math::Absolute(inv(1, 0) - -0.7) < 1e-12);
    EXPECT(Math::Absolute(inv(1, 1) - 0.4) < 1e-12);
}

TEST_CASE(TestMatrixInverse3x3){
    PrismDebug("[TEST] Inverse 3x3\n");

    Matrix<3, 3, f64> m   = {{1.0, 2.0, 3.0}, {0.0, 1.0, 4.0}, {5.0, 6.0, 0.0}};

    auto              inv = m.Inverse();

    Matrix<3, 3, f64> identity = m * inv;
    // Check approximate identity
    for (usize r = 0; r < 3; ++r)
        for (usize c = 0; c < 3; ++c)
            EXPECT(Math::Absolute(identity(r, c) - (r == c ? 1.0 : 0.0))
                        < 1e-9);
}

TEST_CASE(TestMatrixEquality){
    PrismDebug("[TEST] Equality and inequality\n");

    Matrix<2, 2, int> a = {{1, 2}, {3, 4}};
    Matrix<2, 2, int> b = {{1, 2}, {3, 4}};
    Matrix<2, 2, int> c = {{2, 2}, {3, 4}};

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}


TEST_CASE(PrismDebug_All_Matrix_tests_passed_n)
{
    PrismDebug("✅ All Matrix tests passed!\n");
}

