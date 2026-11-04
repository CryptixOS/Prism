/*
 * Created by v1tr10l7 on 04.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

/**
 * Every public Prism macro, in one pure-preprocessor header.
 *
 * C++ modules can't export macros, so module users write
 *
 *     import Prism;
 *     #include <Prism/Macros.hpp>
 *
 * Header users get this file through the regular Prism headers.
 *
 * This header must only define macros (and the extern "C" __assert_fail
 * fallback, which belongs to the global module). It must never declare
 * Prism entities: in a module build those are attached to the Prism module,
 * and redeclaring them here would conflict.
 *
 * Only things that can't be functions without losing their purpose live
 * here: stringification, call-site __FILE__/__LINE__/frame access, early
 * returns from the caller, compile-time removal of arguments, attributes,
 * and configuration switches. Everything else is a function, and the old
 * macro names below just forward to it.
 */

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Config.hpp>
#include <Prism/Core/Platform.hpp>

// ------------------------------------------------------------------ basics
#if PrismHasInclude(<assert.h>)
    #include <assert.h>
#else
extern "C" PM_NORETURN void __assert_fail(const char* expr, const char* file,
                                          unsigned int line,
                                          const char*  function);
    #define assert(expr)                                                       \
        (__builtin_expect(!(expr), 0)                                          \
             ? __assert_fail(#expr, __FILE__, __LINE__, __PRETTY_FUNCTION__)   \
             : (void)0)
#endif

#define PrismStringifyInner(x) #x
#define PrismStringify(x)      PrismStringifyInner(x)
#ifndef Stringify
    #define Stringify(x) PrismStringify(x)
#endif

// va_start/va_arg have to run in the variadic function's own frame.
#define PrismVaStart(args, format) __builtin_va_start(args, format)
#define PrismVaEnd(args)           __builtin_va_end(args)
#define PrismVaArg(args, type...)  __builtin_va_arg(args, type)

// ------------------------------------------------------------------ format
#if PRISM_DISABLE_FMT == 0
    #include <fmt/format.h>
    #define PrismFmtFormat(...) fmt::format(__VA_ARGS__)
#else
    #define PrismFmtFormat(fmt, ...) ::Prism::Format1(fmt, __VA_ARGS__)
#endif

// --------------------------------------------------------------------- log
// Macros so that disabled log levels compile away, arguments included.
#ifdef PRISM_BUILD_DEBUG
    #define PrismDebug(...) ::Prism::Log::Debug(__VA_ARGS__)
    #define PrismDebugNoAlloc(...)                                             \
        ::Prism::Log::Printf(::Prism::LogLevel::eDebug, __VA_ARGS__)
#else
    #define PrismDebug(...)
    #define PrismDebugNoAlloc(...)
#endif

#define PrismMessage(...) ::Prism::Log::Message(__VA_ARGS__)
#define PrismMessageNoAlloc(...)                                               \
    ::Prism::Log::Printf(::Prism::LogLevel::eNone, __VA_ARGS__)

#if PRISM_LOG_ENABLE != 0
    #define PrismTraceNoAlloc(...)                                             \
        ::Prism::Log::Logf(::Prism::LogLevel::eTrace, __VA_ARGS__)
    #define PrismInfoNoAlloc(...)                                              \
        ::Prism::Log::Logf(::Prism::LogLevel::eInfo, __VA_ARGS__)
    #define PrismWarnNoAlloc(...)                                              \
        ::Prism::Log::Logf(::Prism::LogLevel::eWarn, __VA_ARGS__)
    #define PrismErrorNoAlloc(...)                                             \
        ::Prism::Log::Logf(::Prism::LogLevel::eError, __VA_ARGS__)
    #define PrismFatalNoAlloc(...)                                             \
        ::Prism::Log::Logf(::Prism::LogLevel::eFatal, __VA_ARGS__)

    #define PrismTrace(...) ::Prism::Log::Trace(__VA_ARGS__)
    #define PrismInfo(...)  ::Prism::Log::Info(__VA_ARGS__)
    #define PrismWarn(...)  ::Prism::Log::Warn(__VA_ARGS__)
    #define PrismError(...) ::Prism::Log::Error(__VA_ARGS__)
    #define PrismFatal(...) ::Prism::Log::Fatal(__VA_ARGS__)
#else
    #define PrismTraceNoAlloc(...)
    #define PrismInfoNoAlloc(...)
    #define PrismWarnNoAlloc(...)
    #define PrismErrorNoAlloc(...)
    #define PrismFatalNoAlloc(...)

    #define PrismTrace(...)
    #define PrismInfo(...)
    #define PrismWarn(...)
    #define PrismError(...)
    #define PrismFatal(...)
#endif

// ------------------------------------------------------- panic / assertion
// Needs string-literal concatenation with the caller's format string.
#if !defined(PRISM_TARGET_CRYPTIX) || PRISM_TARGET_CRYPTIX == 0
    #define PrismEarlyPanic(fmt, ...)                                          \
        ::Prism::earlyPanic("Error Message: " fmt __VA_OPT__(, ) __VA_ARGS__)
#else
    #define PrismEarlyPanic(fmt, ...)                                          \
        ::earlyPanic("Error Message: " fmt __VA_OPT__(, ) __VA_ARGS__)
#endif

// Compatibility aliases: these are functions now (Debug/Assertions.hpp and
// Core/Types.hpp).
#define PrismPanic(...)          ::Prism::FormattedPanic(__VA_ARGS__)
#define PrismNotReached()        ::Prism::Unreachable()
#define PrismToDo()              ::Prism::NotImplemented()
#define PrismToDoWarn()          ::Prism::NotImplementedWarn()
#define PrismWarnAssert(cond, msg) ::Prism::WarnIf((cond), (msg))
#define PmTrap()                 ::Prism::Trap()
#define PmLaunder(ptr)           ::Prism::Launder(ptr)

// Assertions stay macros: they stringify the expression, report the
// caller's location, and only evaluate the message when the check fails.
#define PrismAssertionFailed(...)                                              \
    {                                                                          \
        PM_UNUSED ::Prism::SourceLocation source                               \
            = ::Prism::SourceLocation::Current();                              \
        PrismPanic("{}: ==>\nAssertion Failed: {}", source,                    \
                   PrismFmtFormat(__VA_ARGS__));                               \
    }
#define PrismAssert(expr...)                                                   \
    {                                                                          \
        if (!(expr)) PrismAssertionFailed("{}", #expr);                        \
    }
#define PrismAssertMsg(expr, msg)                                              \
    {                                                                          \
        if (!(expr)) PrismAssertionFailed("{}", msg);                          \
    }
#define PrismAssertFmt(expr, fmt, ...)                                         \
    {                                                                          \
        if (!(expr)) PrismAssertionFailed(fmt, __VA_ARGS__);                   \
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

// ------------------------------------------------------------------ errors
// These return from the *calling* function, which only a macro can do.
#define PM_TryOrRetFmt(expr, return_value, ...)                                \
    ({                                                                         \
        auto result = (expr);                                                  \
        if (!result)                                                           \
        {                                                                      \
            PrismError("{}", PrismFmtFormat(__VA_ARGS__));                     \
            return (return_value);                                             \
        }                                                                      \
        ::Prism::Move(result.Value());                                         \
    })
#define PM_TryOrRetMsg(expr, return_value, msg)                                \
    PM_TryOrRetFmt(expr, return_value, "{}", msg)
#define PM_TryOrRetVal(expr, return_value)                                     \
    ({                                                                         \
        auto result = (expr);                                                  \
        if (!result) return (return_value);                                    \
        ::Prism::Move(result.Value());                                         \
    })
#define PM_TryOrRetCode(expr, code_on_fail)                                    \
    PM_TryOrRetVal(expr, ::Prism::Error(code_on_fail))
#define PM_TryOrRet(expr) PM_TryOrRetCode(expr, result.Error())
#define PM_TryAcquire(expr)                                                    \
    ({                                                                         \
        auto result = (expr);                                                  \
        if (!result) return;                                                   \
        ::Prism::Move(result.Value());                                         \
    })
#define PM_Try(expr)                                                           \
    ({                                                                         \
        auto result = (expr);                                                  \
        if (!result) return;                                                   \
    })

#if PRISM_USE_NAMESPACE != 0
    #define VaStart(args, format) PrismVaStart(args, format)
    #define VaEnd(args)           PrismVaEnd(args)
    #define VaArg(args, type...)  PrismVaArg(args, type)

    #define TryOrRetFmt(expr, return_value, ...)                               \
        PM_TryOrRetFmt(expr, return_value, __VA_ARGS__)
    #define TryOrRetMsg(expr, return_value, msg)                               \
        PM_TryOrRetMsg(expr, return_value, msg)
    #define TryOrRetVal(expr, val)           PM_TryOrRetVal(expr, val)
    #define TryOrRetCode(expr, code_on_fail) PM_TryOrRetCode(expr, code_on_fail)
    #define TryOrRet(expr)                   PM_TryOrRet(expr)
    #define TryAcquire(expr)                 PM_TryAcquire(expr)
    #define Try(expr)                        PM_Try(expr)
#endif
