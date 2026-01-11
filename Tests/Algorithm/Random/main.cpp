/*
 * Created by v1tr10l7 on 09.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Algorithm/Random.hpp>
#include <Prism/Utility/Math.hpp>

using namespace Prism;
using namespace Prism::Algorithm;

// Known output sequence for std::mt19937 with seed 5489 (from
// https://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine/operator())
// First 3 values for reference
constexpr u32 KnownOutputs[] = {3499211612u, 581869302u, 3890346734u};

static void   TestLog2CompileTime()
{
    static_assert(Algorithm::Details::Log2<unsigned int, 1>::Value == 0,
                  "Log2(1) == 0");
    static_assert(Algorithm::Details::Log2<unsigned int, 2>::Value == 1,
                  "Log2(2) == 1");
    static_assert(Algorithm::Details::Log2<unsigned int, 4>::Value == 2,
                  "Log2(4) == 2");
    static_assert(Algorithm::Details::Log2<unsigned int, 1024>::Value == 10,
                  "Log2(1024) == 10");

    PrismDebug("Log2 compile-time tests passed.\n");
}

static void TestDefaultSeedSequence()
{
    mt19937 engine; // default seed 5489
    for (usize i = 0; i < 3; ++i)
    {
        auto val = engine();
        assert(val == KnownOutputs[i]
               && "MT output mismatch with default seed");
    }
    PrismDebug("Default seed output tests passed.\n");
}

static void TestSeedConstructorAndEquality()
{
    mt19937 eng1(5489);
    mt19937 eng2(5489);
    mt19937 eng3(12345);

    assert(eng1 == eng2 && "Engines with same seed should be equal");
    assert(!(eng1 != eng2) && "Inequality failed on equal engines");
    assert(eng1 != eng3 && "Engines with different seeds should differ");

    PrismDebug("Seed constructor and equality tests passed.\n");
}

static void TestDiscardAdvancesState()
{
    mt19937 eng1(5489);
    mt19937 eng2(5489);

    // advance eng1 by calling operator() 10 times
    for (int i = 0; i < 10; ++i) eng1();

    // discard 10 steps on eng2
    eng2.Discard(10);

    // next value should be the same
    assert(eng1() == eng2()
           && "Discard should advance state equivalently to operator() calls");

    PrismDebug("Discard method test passed.\n");
}

int main()
{
    TestLog2CompileTime();
    TestDefaultSeedSequence();
    TestSeedConstructorAndEquality();
    TestDiscardAdvancesState();

    UniformIntDistribution<mt19937_64::ResultType> dist;
    mt19937_64                                     rng;

    PrismDebug("All MersenneTwisterEngine tests passed.\n");
    return 0;
}
