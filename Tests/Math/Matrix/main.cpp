/*
 * Created by v1tr10l7 on 07.11.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Debug/Assertions.hpp>
#include <Prism/Math/Matrix.hpp>

using namespace Prism;
using Math::Matrix;

void TestMatrixBasicConstruction()
{
    PrismDebug("[TEST] Basic Matrix construction and indexing\n");

    Matrix<2, 3, f64> m = {
        {1.0, 2.0, 3.0}, // column 0
        {4.0, 5.0, 6.0}  // column 1
    };

    PrismAssert(m(0, 0) == 1.0);
    PrismAssert(m(1, 0) == 2.0);
    PrismAssert(m(2, 0) == 3.0);
    PrismAssert(m(0, 1) == 4.0);
    PrismAssert(m(1, 1) == 5.0);
    PrismAssert(m(2, 1) == 6.0);

    auto col0 = m[0];
    PrismAssert(col0[0] == 1.0 && col0[1] == 2.0 && col0[2] == 3.0);
}

void TestMatrixArithmetic()
{
    PrismDebug("[TEST] Matrix arithmetic (+,-,*,/ scalar)\n");

    Matrix<2, 2, f64> a   = {{1.0, 2.0}, {3.0, 4.0}};

    Matrix<2, 2, f64> b   = {{5.0, 6.0}, {7.0, 8.0}};

    auto              sum = a + b;
    PrismAssert(sum(0, 0) == 6.0 && sum(1, 0) == 8.0);
    PrismAssert(sum(0, 1) == 10.0 && sum(1, 1) == 12.0);

    auto diff = b - a;
    PrismAssert(diff(0, 0) == 4.0 && diff(1, 0) == 4.0);
    PrismAssert(diff(0, 1) == 4.0 && diff(1, 1) == 4.0);

    auto scaled = a * 2.0;
    PrismAssert(scaled(0, 0) == 2.0 && scaled(1, 0) == 4.0);
    PrismAssert(scaled(0, 1) == 6.0 && scaled(1, 1) == 8.0);

    a *= 3.0;
    PrismAssert(a(0, 0) == 3.0 && a(1, 1) == 12.0);
}

void TestMatrixVectorMultiplication()
{
    PrismDebug("[TEST] Matrix × Vector multiplication\n");

    Matrix<2, 3, f64>    m   = {{1.0, 0.0, 0.0}, {0.0, 2.0, 0.0}};

    Math::Vector<2, f64> v   = {4.0, 5.0};
    auto                 res = m * v;

    // result = col0*v[0] + col1*v[1]
    // col0 = (1,0,0), col1 = (0,2,0)
    PrismAssert(res[0] == 4.0 && res[1] == 10.0 && res[2] == 0.0);
}

void TestMatrixTranspose()
{
    PrismDebug("[TEST] Transpose\n");

    Matrix<3, 2, f64> m = {{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}};

    auto              t = m.Transpose();
    // t is 2x3
    PrismAssert(t(0, 0) == 1.0 && t(0, 1) == 2.0);
    PrismAssert(t(1, 0) == 3.0 && t(1, 1) == 4.0);
    PrismAssert(t(2, 0) == 5.0 && t(2, 1) == 6.0);
}

void TestMatrixDeterminant2x2()
{
    PrismDebug("[TEST] Determinant 2x2\n");

    Matrix<2, 2, f64> m   = {{1.0, 2.0}, {3.0, 4.0}};

    f64               det = m.Determinant();
    PrismAssert(Math::Absolute(det - (-2.0)) < 1e-12);
}

void TestMatrixDeterminant3x3()
{
    PrismDebug("[TEST] Determinant 3x3\n");

    Matrix<3, 3, f64> m = {{1.0, 2.0, 3.0}, {0.0, 1.0, 4.0}, {5.0, 6.0, 0.0}};

    f64 det = m.Determinant(); // expected = 1*(1*0 - 4*6) - 2*(0*0 - 4*5) +
                               // 3*(0*6 - 1*5) = -24 + 40 - 15 = 1
    PrismAssert(Math::Absolute(det - 1.0) < 1e-12);
}

void TestMatrixInverse2x2()
{
    PrismDebug("[TEST] Inverse 2x2\n");

    Matrix<2, 2, f64> m   = {{4.0, 7.0}, {2.0, 6.0}};

    auto              inv = m.Inverse();
    // Expected inverse = (1/(4*6-7*2)) * [6 -7; -2 4]
    // determinant = 10
    PrismAssert(Math::Absolute(inv(0, 0) - 0.6) < 1e-12);
    PrismAssert(Math::Absolute(inv(0, 1) - -0.2) < 1e-12);
    PrismAssert(Math::Absolute(inv(1, 0) - -0.7) < 1e-12);
    PrismAssert(Math::Absolute(inv(1, 1) - 0.4) < 1e-12);
}

void TestMatrixInverse3x3()
{
    PrismDebug("[TEST] Inverse 3x3\n");

    Matrix<3, 3, f64> m   = {{1.0, 2.0, 3.0}, {0.0, 1.0, 4.0}, {5.0, 6.0, 0.0}};

    auto              inv = m.Inverse();

    Matrix<3, 3, f64> identity = m * inv;
    // Check approximate identity
    for (usize r = 0; r < 3; ++r)
        for (usize c = 0; c < 3; ++c)
            PrismAssert(Math::Absolute(identity(r, c) - (r == c ? 1.0 : 0.0))
                        < 1e-9);
}

void TestMatrixEquality()
{
    PrismDebug("[TEST] Equality and inequality\n");

    Matrix<2, 2, int> a = {{1, 2}, {3, 4}};
    Matrix<2, 2, int> b = {{1, 2}, {3, 4}};
    Matrix<2, 2, int> c = {{2, 2}, {3, 4}};

    PrismAssert(a == b);
    PrismAssert(a != c);
}

int main()
{
    TestMatrixBasicConstruction();
    TestMatrixArithmetic();
    TestMatrixVectorMultiplication();
    TestMatrixTranspose();
    TestMatrixDeterminant2x2();
    TestMatrixDeterminant3x3();
    TestMatrixInverse2x2();
    TestMatrixInverse3x3();
    TestMatrixEquality();

    PrismDebug("✅ All Matrix tests passed!\n");
    return 0;
}
