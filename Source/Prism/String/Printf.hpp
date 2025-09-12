/*
 * Created by v1tr10l7 on 24.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/String/StringView.hpp>

namespace Prism
{
    namespace Printf
    {
        enum class ArgumentType
        {
            eChar                = 0,
            eInteger             = 1,
            eLong                = 2,
            eLongLong            = 3,
            eSize                = 4,

            eUnsignedChar        = 5,
            eUnsignedInteger     = 6,
            eUnsignedLong        = 7,
            eUnsignedLongLong    = 8,
            eUnsignedSize        = 9,
            eLastArithmetic      = 10,
            eString              = 11,
            eOutWrittenCharCount = 12,
        };

        enum class Sign
        {
            eNone  = 0,
            eMinus = 1,
            ePlus  = 2,
            eSpace = 3,
        };
        struct FormatSpec
        {
            ArgumentType   Type            = ArgumentType::eInteger;
            u8             Base            = 10;
            isize          Length          = -1;
            isize          Precision       = 0;
            enum Sign      Sign            = Sign::eNone;

            bool           JustifyLeft : 1 = false;
            bool           PrintBase   : 1 = false;
            bool           ZeroPad     : 1 = false;
            bool           UpperCase   : 1 = false;

            constexpr bool Arithmetic() const
            {
                return Type < ArgumentType::eLastArithmetic;
            }
        };
        struct FormatParser
        {
            enum class State
            {
                eBegin     = 0,
                eFlags     = 1,
                eWidth     = 2,
                ePrecision = 3,
                eLength    = 4,
                eSpecifier = 5,
            };

            FormatSpec operator()(StringView::Iterator& it, VaList& args);
        };

        isize Formatted(const char* format, ...);
        isize Variadic(const char* format, VaList& args);
    }; // namespace Printf
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
namespace Printf = Prism::Printf;
#endif
