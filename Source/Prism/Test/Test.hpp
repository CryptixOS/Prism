/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/**
 * Prism's unit testing framework, modelled after SerenityOS' LibTest.
 *
 *   #include <Prism/Test/Test.hpp>
 *
 *   TEST_CASE(vector_push_back)
 *   {
 *       Vector<int> v;
 *       v.PushBack(1);
 *       EXPECT_EQ(v.Size(), 1u);
 *   }
 *
 *   RANDOMIZED_TEST_CASE(reverse_twice_is_identity)
 *   {
 *       GEN(v, Gen::Vector([] { return Gen::UnsignedInt(); }));
 *       ...
 *   }
 *
 * Link with prism_test and prism_test_main (which provides main()), or call
 * Prism::Test::TestSuite::The().Main(argc, argv) from your own main().
 *
 * Test cases:   TEST_CASE, BENCHMARK_CASE, RANDOMIZED_TEST_CASE, TEST_SETUP
 * Expectations: EXPECT, EXPECT_EQ, EXPECT_NE, EXPECT_EQ_TRUTH,
 *               EXPECT_EQ_FORCE, EXPECT_APPROXIMATE(_WITH_ERROR), FAIL,
 *               TRY_OR_FAIL, MUST, VERIFY, VERIFY_NOT_REACHED, TODO
 * Crash tests:  EXPECT_CRASH, EXPECT_CRASH_WITH_SIGNAL, EXPECT_NO_CRASH
 * Randomized:   GEN, ASSUME, REJECT and the generators in Prism::Test::Gen
 */
#include <Prism/Test/CrashTest.hpp>
#include <Prism/Test/Macros.hpp>
#include <Prism/Test/Randomized/Generator.hpp>
#include <Prism/Test/Randomized/RandomRun.hpp>
#include <Prism/Test/Randomized/RandSource.hpp>
#include <Prism/Test/TestCase.hpp>
#include <Prism/Test/TestResult.hpp>
#include <Prism/Test/TestSuite.hpp>

namespace Gen = Prism::Test::Gen;
