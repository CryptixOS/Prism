/*
 * Created by v1tr10l7 on 15.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Utility/Expected.hpp>
#include <cerrno>
#include <expected>

namespace Prism
{
#ifndef Stringify
    #define Stringify(x) PrismStringify(x)
#endif
#if PRISM_TARGET_CRYPTIX != 0
    using ErrorCode = std::errno_t;
#else
    using ErrorCode = i32;
#endif
    using Error = Unexpected<ErrorCode>;

    template <typename R>
    using ErrorOr = Expected<R, ErrorCode>;
}; // namespace Prism

#define PM_TryOrRetVal(expr, return_value)                                     \
    ({                                                                         \
        auto result = (expr);                                                  \
        if (!result) return (return_value);                                    \
        Prism::Move(result.Value());                                           \
    })
#define PM_TryOrRetCode(expr, code_on_fail)                                    \
    PM_TryOrRetVal(expr, Error(code_on_fail))
#define PM_TryOrRet(expr) PM_TryOrRetCode(expr, result.Error())

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Error;
using Prism::ErrorCode;
using Prism::ErrorOr;
    #define TryOrRetVal(expr, val)           PM_TryOrRetVal(expr, val)
    #define TryOrRetCode(expr, code_on_fail) PM_TryOrRetCode(expr, code_on_fail)
    #define TryOrRet(expr)                   PM_TryOrRet(expr)
#endif
