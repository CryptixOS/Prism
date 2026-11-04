/*
 * Created by v1tr10l7 on 01.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */

#include <cstdio>

#if PRISM_TEST_MODULES
// Headers (std, fmt, macros) must come before any import.
#include <Prism/Macros.hpp>
import Prism;
#else
#include <Prism/String/StringView.hpp>
#endif

using namespace Prism;

int main() { std::printf("HelloWorld"); };
