/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Types.hpp>

// Macros only: the code lives in module Prism.Debug.Log, which
// files using these macros import too.

#ifdef PRISM_BUILD_DEBUG
    #define PrismDebug(...)        Log::Debug(__VA_ARGS__)
    #define PrismDebugNoAlloc(...) Log::Printf(LogLevel::eDebug, __VA_ARGS__)
#else
    #define PrismDebug(...)
    #define PrismDebugNoAlloc(...)
#endif

#define PrismMessage(...)        Log::Message(__VA_ARGS__)
#define PrismMessageNoAlloc(...) Log::Printf(LogLevel::eNone, __VA_ARGS__)

#if PRISM_LOG_ENABLE != 0
    #define PrismTraceNoAlloc(...) Log::Logf(LogLevel::eTrace, __VA_ARGS__)
    #define PrismInfoNoAlloc(...)  Log::Logf(LogLevel::eInfo, __VA_ARGS__)
    #define PrismWarnNoAlloc(...)  Log::Logf(LogLevel::eWarn, __VA_ARGS__)
    #define PrismErrorNoAlloc(...) Log::Logf(LogLevel::eError, __VA_ARGS__)
    #define PrismFatalNoAlloc(...) Log::Logf(LogLevel::eFatal, __VA_ARGS__)

    #define PrismTrace(...)        Log::Trace(__VA_ARGS__)
    #define PrismInfo(...)         Log::Info(__VA_ARGS__)
    #define PrismWarn(...)         Log::Warn(__VA_ARGS__)
    #define PrismError(...)        Log::Error(__VA_ARGS__)
    #define PrismFatal(...)        Log::Fatal(__VA_ARGS__)
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
