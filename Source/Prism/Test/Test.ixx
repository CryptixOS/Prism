/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Test/Macros.hpp>
#include <Prism/Test/TestCase.hpp>
#include <Prism/Test/Test.hpp>

export module Prism.Test.Test;

export import Prism.Test.CrashTest;
export import Prism.Test.Macros;
export import Prism.Test.Randomized.Generator;
export import Prism.Test.Randomized.RandomRun;
export import Prism.Test.Randomized.RandSource;
export import Prism.Test.TestCase;
export import Prism.Test.TestResult;
export import Prism.Test.TestSuite;

export extern "C++"
{
    namespace Gen = Prism::Test::Gen;
}
