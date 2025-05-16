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

#if !defined(PRISM_TARGET_CRYPTIX) || PRISM_TARGET_CRYPTIX == 0
namespace Prism
{
#endif
    extern void earlyPanic(Prism::StringView msg);
    [[noreturn]]
    extern void panic(Prism::StringView msg);
#if PRISM_TARGET_CRYPTIX == 0
}; // namespace Prism
#endif

#if !defined(PRISM_TARGET_CRYPTIX) || PRISM_TARGET_CRYPTIX == 0
    #define PrismEarlyPanic(fmt, ...)                                          \
        Prism::earlyPanic("Error Message: " fmt __VA_OPT__(, ) __VA_ARGS__)
    #define PrismPanic(...) Prism::panic(fmt::format(__VA_ARGS__).data())

#else
    #define PrismEarlyPanic(fmt, ...)                                          \
        earlyPanic("Error Message: " fmt __VA_OPT__(, ) __VA_ARGS__)
    #define PrismPanic(...) panic(fmt::format(__VA_ARGS__).data())

#endif

#define PrismAssert(expr) PrismAssertMsg(expr, #expr)
#define PrismAssertMsg(expr, msg)                                              \
    {                                                                          \
        [[maybe_unused]] Prism::SourceLocation source                          \
            = Prism::SourceLocation::Current();                                \
        if (!(expr)) PrismAssertionFailed("{}", msg);                          \
    }

#define PrismAssertFmt(expr, fmt, ...)                                         \
    {                                                                          \
        if (!(expr)) PrismAssertionFailed(fmt, __VA_ARGS__);                   \
    }
#define PrismWarnAssert(cond, msg)                                             \
    {                                                                          \
        if ((cond)) PrismWarn(msg);                                            \
    }
#define PrismNotReached() __builtin_unreachable();

#define PrismToDo()                                                            \
    PrismAssertFmt(false, "{} is not implemented!", __PRETTY_FUNCTION__)
#define PrismToDoWarn()                                                        \
    {                                                                          \
        Prism::SourceLocation source = Prism::SourceLocation::Current();       \
        PrismWarn("{}[{}:{}]: {} is not implemented!", source.FileName(),      \
                  source.Line(), source.Column(), source.FunctionName());      \
    }

#define PrismAssertionFailed(...)                                              \
    {                                                                          \
        Prism::SourceLocation source = Prism::SourceLocation::Current();       \
        PrismPanic("{}: ==>\nAssertion Failed: {}", source,                    \
                   fmt::format(__VA_ARGS__));                                  \
    }

#if PRISM_PREFIXLESS_MACROS != 0 || PRISM_TARGET_CRYPTIX != 0
    #define Assert(expr)              PrismAssert(expr)
    #define AssertMsg(expr, msg)      PrismAssertMsg(expr, msg)
    #define AssertFmt(expr, fmt, ...) PrismAssertFmt(expr, fmt, __VA_ARGS__)
    #define AssertNotReached          PrismNotReached

    #define ToDo                      PrismToDo
    #define ToDoWarn                  PrismToDoWarn
    #define DebugWarnIf               PrismWarnAssert

    #define EarlyPanic                PrismEarlyPanic
    #define Panic                     PrismPanic
#endif
