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
#ifdef PRISM_ERRNO_T
#undef PRISM_ERRNO_T
#endif
#if PRISM_TARGET_CRYPTIX == 0
    #define PRISM_ERRNO_T int
#else 
    #define PRISM_ERRNO_T std::errno_t
#endif
#ifndef Stringify
    #define Stringify(x) PrismStringify(x)
#endif
    // #define PRISM_EXPECTED_T   std::expected
    // #define PRISM_UNEXPECTED_T std::unexpected
#ifdef PRISM_EXPECTED_T
    #undef PRISM_EXPECTED_T
#endif
#define PRISM_EXPECTED_T Expected
#ifdef PRISM_UNEXPECTED_T
    #undef PRISM_UNEXPECTED_T
#endif
    #define PRISM_UNEXPECTED_T Unexpected
    using ErrorCode = PRISM_ERRNO_T;
    using Error     = PRISM_UNEXPECTED_T<PRISM_ERRNO_T>;

    template <typename R>
    using ErrorOr = PRISM_EXPECTED_T<R, PRISM_ERRNO_T>;
    // #endif
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Error;
using Prism::ErrorCode;
using Prism::ErrorOr;
#endif
