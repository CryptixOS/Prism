/*
 * Created by v1tr10l7 on 25.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Debug/Log.hpp>
#include <Prism/Debug/SourceLocation.hpp>

#include <format>

#if PRISM_TARGET_CRYPTIX == false
namespace Prism
{
#endif
    [[noreturn]]
    extern void earlyPanic(std::string_view msg);
    [[noreturn]]
    extern void panic(std::string_view msg);

#if PRISM_TARGET_CRYPTIX == false
}; // namespace Prism
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
        /*PrismPanic("{}[{}:{}]: Assertion Failed =>\n{}: {}", */              \
        /*               source.FileName(), source.Line(), source.Column(), */ \
        /*source.FunctionName(), std::format(__VA_ARGS__));           */       \
        PrismPanic("{}: ==>\nAssertion Failed: {}", source,                    \
                   std::format(__VA_ARGS__));                                  \
    }

#define PrismEarlyPanic(fmt, ...)                                              \
    earlyPanic("Error Message: " fmt __VA_OPT__(, ) __VA_ARGS__)
#define PrismPanic(...) panic(std::format(__VA_ARGS__).data())

#if PRISM_PREFIXLESS_MACROS == 1 || PRISM_TARGET_CRYPTIX
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
