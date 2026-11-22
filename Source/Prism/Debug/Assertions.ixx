/*
 * Created by v1tr10l7 on 25.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Config.hpp>

#include <Prism/Debug/Log.hpp>

#include <Prism/Debug/Assertions.hpp>

export module Prism.Debug.Assertions;

export import Prism.Debug.Log;
export import Prism.Debug.SourceLocation;
export import Prism.String.StringView;
export import Prism.String.Formatter;

export extern "C++"
{
#if !defined(PRISM_TARGET_CRYPTIX) || PRISM_TARGET_CRYPTIX == 0
    namespace Prism
    {
#endif
        extern void earlyPanic(Prism::StringView msg);
        PM_NORETURN
        extern void panic(Prism::StringView msg);
#if PRISM_TARGET_CRYPTIX == 0
    }; // namespace Prism
#endif
}
