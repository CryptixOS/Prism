/*
 * Created by v1tr10l7 on 15.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

// Macros only: the code lives in module Prism.Core.Error, which
// files using these macros import too.

#ifndef Stringify
    #define Stringify(x) PrismStringify(x)
#endif

#define PM_TryOrRetFmt(expr, return_value, ...)                                \
    ({                                                                         \
        auto result = (expr);                                                  \
        if (!result)                                                           \
        {                                                                      \
            LogError(__VA_ARGS__);                                             \
            return (return_value);                                             \
        }                                                                      \
        Prism::Move(result.Value());                                           \
    })

#define PM_TryOrRetMsg(expr, return_value)                                     \
    PM_TryOrRetFmt(expr, return_value, msg)
#define PM_TryOrRetVal(expr, return_value)                                     \
    ({                                                                         \
        auto result = (expr);                                                  \
        if (!result) return (return_value);                                    \
        Prism::Move(result.Value());                                           \
    })
#define PM_TryOrRetCode(expr, code_on_fail)                                    \
    PM_TryOrRetVal(expr, Error(code_on_fail))
#define PM_TryOrRet(expr) PM_TryOrRetCode(expr, result.Error())
#define PM_TryAcquire(expr)                                                    \
    ({                                                                         \
        auto result = (expr);                                                  \
        if (!result) return;                                                   \
        Prism::Move(result.Value());                                           \
    })
#define PM_Try(expr)                                                           \
    ({                                                                         \
        auto result = (expr);                                                  \
        if (!result) return;                                                   \
    })

#if PRISM_USE_NAMESPACE != 0
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
