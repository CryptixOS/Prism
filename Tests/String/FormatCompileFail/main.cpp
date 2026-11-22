/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
// Must NOT compile. Tests/CMakeLists.txt builds it and expects the build to
// fail with the reason ("argument not found") in the diagnostic.
import Prism.String.Formatter;

using namespace Prism;

void TooFewArguments() { (void)Format("{} {}", 1); }
