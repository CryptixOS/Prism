/*
 * Created by v1tr10l7 on 25.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Config.hpp>

#include <Prism/Debug/Log.hpp>
#include <Prism/Debug/SourceLocation.hpp>

#include <Prism/String/StringView.hpp>

#if PRISM_DISABLE_FMT == 0
    #include <fmt/format.h>
#else
    #include <Prism/String/Formatter.hpp>
#endif

#if !defined(PRISM_TARGET_CRYPTIX) || PRISM_TARGET_CRYPTIX == 0
PRISM_EXPORT namespace Prism
{
#endif
    extern void earlyPanic(Prism::StringView msg);
    PM_NORETURN
    extern void panic(Prism::StringView msg);
#if PRISM_TARGET_CRYPTIX == 0
}; // namespace Prism
#endif

PRISM_EXPORT namespace Prism
{
    /// Formats a message and panics. Replaces the PrismPanic() macro (kept
    /// as an alias in Prism/Macros.hpp).
#if PRISM_DISABLE_FMT == 0
    template <typename... Args>
    PM_NORETURN void FormattedPanic(fmt::format_string<Args...> format,
                                    Args&&... args)
    {
        auto message = fmt::format(format, Forward<Args>(args)...);
    #if PRISM_TARGET_CRYPTIX == 0
        ::Prism::panic(StringView(message.data(), message.size()));
    #else
        ::panic(StringView(message.data(), message.size()));
    #endif
    }
#else
    template <typename... Args>
    PM_NORETURN void FormattedPanic(StringView format, Args&&... args)
    {
        auto message = Format1(format, Forward<Args>(args)...);
    #if PRISM_TARGET_CRYPTIX == 0
        ::Prism::panic(message);
    #else
        ::panic(message);
    #endif
    }
#endif

    /// Panics reporting that the calling function isn't implemented.
    /// Replaces PrismToDo().
    PM_NORETURN inline void
    NotImplemented(SourceLocation location = SourceLocation::Current())
    {
        FormattedPanic("{}: ==>\nAssertion Failed: {} is not implemented!",
                       location, location.FunctionName());
    }
    /// Warns that the calling function isn't implemented. Replaces
    /// PrismToDoWarn().
    inline void
    NotImplementedWarn(SourceLocation location = SourceLocation::Current())
    {
        IgnoreUnused(location);
        PrismWarn("{}[{}:{}]: {} is not implemented!", location.FileName(),
                  location.Line(), location.Column(), location.FunctionName());
    }
    /// Logs `message` as a warning when `condition` holds. Replaces
    /// PrismWarnAssert().
    inline void WarnIf(bool condition, StringView message)
    {
        IgnoreUnused(message);
        if (condition) { PrismWarn("{}", message); }
    }
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
PRISM_EXPORT using Prism::FormattedPanic;
PRISM_EXPORT using Prism::NotImplemented;
PRISM_EXPORT using Prism::NotImplementedWarn;
PRISM_EXPORT using Prism::WarnIf;
#endif
