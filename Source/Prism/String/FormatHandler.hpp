/*
 * Created by v1tr10l7 on 11.09.2025.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/String/FormatterContext.hpp>

/**
 * Format string grammar (same as fmt / std::format, minus floating point):
 *
 *   replacement_field ::= "{" [arg_id] [":" format_spec] "}"
 *   arg_id            ::= integer | identifier
 *   format_spec       ::= [[fill]align][sign]["#"]["0"][width]["." precision]
 *                         ["L"][type]
 *   fill              ::= <any code point except '{' and '}'>
 *   align             ::= "<" | ">" | "^"
 *   sign              ::= "+" | "-" | " "
 *   width             ::= integer | "{" [arg_id] "}"
 *   precision         ::= integer | "{" [arg_id] "}"
 *   type              ::= "b" | "B" | "c" | "d" | "o" | "p" | "s" | "x" | "X"
 *                         | "?"
 *
 * "{{" and "}}" are literal braces.
 */
namespace Prism
{
    enum class FormatAlign : u8
    {
        eNone,
        eLeft,
        eRight,
        eCenter,
        eNumeric, // set by the '0' flag: zero padding after sign and prefix
    };
    enum class FormatSign : u8
    {
        eNone,
        eMinus,
        ePlus,
        eSpace,
    };
    enum class FormatPresentation : u8
    {
        eNone,
        eDecimal,     // 'd'
        eOctal,       // 'o'
        eHexLower,    // 'x'
        eHexUpper,    // 'X'
        eBinaryLower, // 'b'
        eBinaryUpper, // 'B'
        eChar,        // 'c'
        eString,      // 's'
        ePointer,     // 'p'
        eDebug,       // '?'
    };

    struct FormatSpecs
    {
        i32                Width     = 0;
        i32                Precision = -1;
        FormatPresentation Type      = FormatPresentation::eNone;
        FormatAlign        Align     = FormatAlign::eNone;
        FormatSign         Sign      = FormatSign::eNone;
        bool               Alt       = false; // '#'
        bool               Localized = false; // 'L', digit grouping
        u8                 FillSize  = 1;
        char               Fill[4]   = {' ', 0, 0, 0};

        constexpr void     SetFill(const char* fill, usize size)
        {
            FillSize = static_cast<u8>(size);
            for (usize i = 0; i < size; ++i) Fill[i] = fill[i];
        }
    };

    /// Reference to a width/precision that comes from another argument.
    struct DynamicSpecRef
    {
        enum class Kind : u8
        {
            eNone,
            eIndex,
            eName,
        } RefKind
            = Kind::eNone;
        isize       Index      = 0;
        const char* Name       = nullptr;
        usize       NameLength = 0;
    };

    struct DynamicFormatSpecs : FormatSpecs
    {
        DynamicSpecRef WidthRef;
        DynamicSpecRef PrecisionRef;
    };

    namespace Details
    {
        inline constexpr bool IsDigit(char c) { return c >= '0' && c <= '9'; }
        inline constexpr bool IsNameStart(char c)
        {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                || c == '_';
        }
        inline constexpr bool IsAlignChar(char c)
        {
            return c == '<' || c == '>' || c == '^';
        }
        constexpr FormatAlign ToAlign(char c)
        {
            switch (c)
            {
                case '<': return FormatAlign::eLeft;
                case '>': return FormatAlign::eRight;
                case '^': return FormatAlign::eCenter;
                default: break;
            }
            return FormatAlign::eNone;
        }

        /// Length in bytes of the UTF-8 sequence starting with `lead`.
        constexpr usize CodePointLength(char lead)
        {
            auto c = static_cast<u8>(lead);
            if (c < 0x80) return 1;
            if ((c >> 5) == 0x06) return 2;
            if ((c >> 4) == 0x0e) return 3;
            if ((c >> 3) == 0x1e) return 4;
            return 1;
        }

        inline constexpr i32 MaxSpecValue = 0x7fffffff;

        /// Parses a non-negative decimal integer, reporting overflow.
        constexpr const char* ParseNonNegativeInt(const char* begin,
                                                  const char* end, i32& out)
        {
            u64 value = 0;
            while (begin != end && IsDigit(*begin))
            {
                value = value * 10 + static_cast<u64>(*begin - '0');
                if (value > static_cast<u64>(MaxSpecValue))
                {
                    ReportFormatError("number is too big");
                    value = MaxSpecValue;
                }
                ++begin;
            }
            out = static_cast<i32>(value);
            return begin;
        }

        /**
         * Parses an argument id at `begin` and calls one of
         * handler.OnAuto(), handler.OnIndex(isize) or
         * handler.OnName(StringView).
         */
        template <typename IDHandler>
        constexpr const char* ParseArgID(const char* begin, const char* end,
                                         IDHandler&& handler)
        {
            char c = *begin;
            if (c == '}' || c == ':')
            {
                handler.OnAuto();
                return begin;
            }
            if (IsDigit(c))
            {
                i32 index = 0;
                if (c != '0') begin = ParseNonNegativeInt(begin, end, index);
                else ++begin;

                if (begin == end || (*begin != '}' && *begin != ':'))
                {
                    ReportFormatError("invalid format string");
                    return begin;
                }
                handler.OnIndex(index);
                return begin;
            }
            if (!IsNameStart(c))
            {
                ReportFormatError("invalid format string");
                return begin;
            }

            auto it = begin;
            do ++it;
            while (it != end && (IsNameStart(*it) || IsDigit(*it)));
            handler.OnName(StringView(begin, static_cast<usize>(it - begin)));
            return it;
        }

        constexpr const char* ParseDynamicSpec(const char* begin,
                                               const char* end, i32& value,
                                               DynamicSpecRef&     ref,
                                               FormatParseContext& ctx)
        {
            if (IsDigit(*begin))
            {
                begin = ParseNonNegativeInt(begin, end, value);
                return begin;
            }
            // *begin == '{'
            ++begin;
            if (begin == end)
            {
                ReportFormatError("invalid format string");
                return begin;
            }

            struct
            {
                FormatParseContext& Context;
                DynamicSpecRef&     Ref;

                constexpr void      OnAuto()
                {
                    Ref.RefKind = DynamicSpecRef::Kind::eIndex;
                    Ref.Index   = Context.NextArgID();
                    Context.CheckDynamicSpec(Ref.Index);
                }
                constexpr void OnIndex(isize id)
                {
                    Ref.RefKind = DynamicSpecRef::Kind::eIndex;
                    Ref.Index   = id;
                    Context.CheckArgID(id);
                    Context.CheckDynamicSpec(id);
                }
                constexpr void OnName(StringView name)
                {
                    Ref.RefKind    = DynamicSpecRef::Kind::eName;
                    Ref.Name       = name.Raw();
                    Ref.NameLength = name.Size();
                    Context.CheckArgID(name);
                }
            } handler{ctx, ref};

            if (*begin == '}') handler.OnAuto();
            else begin = ParseArgID(begin, end, handler);

            if (begin == end || *begin != '}')
            {
                ReportFormatError("invalid format string");
                return begin;
            }
            return begin + 1;
        }

        inline constexpr bool ParsePresentation(char c, FormatPresentation& out)
        {
            switch (c)
            {
                case 'd': out = FormatPresentation::eDecimal; break;
                case 'o': out = FormatPresentation::eOctal; break;
                case 'x': out = FormatPresentation::eHexLower; break;
                case 'X': out = FormatPresentation::eHexUpper; break;
                case 'b': out = FormatPresentation::eBinaryLower; break;
                case 'B': out = FormatPresentation::eBinaryUpper; break;
                case 'c': out = FormatPresentation::eChar; break;
                case 's': out = FormatPresentation::eString; break;
                case 'p': out = FormatPresentation::ePointer; break;
                case '?': out = FormatPresentation::eDebug; break;
                default: return false;
            }
            return true;
        }
    } // namespace Details

    /**
     * Parses a standard format specification, starting right after the ':'.
     * Returns a pointer to the terminating '}' (or wherever parsing stopped
     * on error).
     *
     * Like the patched fmt, it is lenient about specifiers that do not apply
     * to the argument type (e.g. '#' on a pointer or a sign on an unsigned
     * integer): they are accepted and ignored where meaningless. Only a
     * malformed specification is an error.
     */
    constexpr const char* ParseFormatSpecs(const char* begin, const char* end,
                                           DynamicFormatSpecs& specs,
                                           FormatParseContext& ctx,
                                           FormatArgType type
                                           = FormatArgType::eNone)
    {
        using namespace Details;
        (void)type;
        if (begin == end || *begin == '}') return begin;

        // [[fill]align]
        {
            usize fillLength = CodePointLength(*begin);
            if (fillLength > static_cast<usize>(end - begin))
                fillLength = static_cast<usize>(end - begin);
            const char* fillEnd = begin + fillLength;

            if (fillEnd != end && IsAlignChar(*fillEnd))
            {
                if (*begin == '{')
                {
                    ReportFormatError("invalid fill character '{'");
                    return begin;
                }
                specs.SetFill(begin, fillLength);
                specs.Align = ToAlign(*fillEnd);
                begin       = fillEnd + 1;
            }
            else if (IsAlignChar(*begin))
            {
                specs.Align = ToAlign(*begin);
                ++begin;
            }
        }
        if (begin == end) return begin;

        // [sign]
        switch (*begin)
        {
            case '+':
                specs.Sign = FormatSign::ePlus;
                ++begin;
                break;
            case '-':
                specs.Sign = FormatSign::eMinus;
                ++begin;
                break;
            case ' ':
                specs.Sign = FormatSign::eSpace;
                ++begin;
                break;
            default: break;
        }
        if (begin == end) return begin;

        // ["#"]
        if (*begin == '#')
        {
            specs.Alt = true;
            if (++begin == end) return begin;
        }
        // ["0"]
        if (*begin == '0')
        {
            if (specs.Align == FormatAlign::eNone)
            {
                specs.Align   = FormatAlign::eNumeric;
                specs.Fill[0] = '0';
                specs.FillSize = 1;
            }
            if (++begin == end) return begin;
        }
        // [width]
        if (IsDigit(*begin) || *begin == '{')
        {
            begin = ParseDynamicSpec(begin, end, specs.Width, specs.WidthRef,
                                     ctx);
            if (begin == end) return begin;
        }
        // ["." precision]
        if (*begin == '.')
        {
            ++begin;
            if (begin == end || (!IsDigit(*begin) && *begin != '{'))
            {
                ReportFormatError("missing precision specifier");
                return begin;
            }
            begin = ParseDynamicSpec(begin, end, specs.Precision,
                                     specs.PrecisionRef, ctx);
            if (begin == end) return begin;
        }
        // ["L"]
        if (*begin == 'L')
        {
            specs.Localized = true;
            if (++begin == end) return begin;
        }
        // [type]
        if (*begin == '}') return begin;
        if (!ParsePresentation(*begin, specs.Type))
        {
            ReportFormatError("invalid format specifier");
            return begin;
        }
        return begin + 1;
    }

    /**
     * Drives a format string through a handler:
     *
     *   OnText(const char* begin, const char* end)
     *   OnArgID() -> isize, OnArgID(isize) -> isize, OnArgID(StringView)
     *     -> isize
     *   OnReplacementField(isize id, const char* closingBrace)
     *   OnFormatSpecs(isize id, const char* begin, const char* end)
     *     -> const char* (the closing brace)
     */
    template <typename Handler>
    constexpr void ParseFormatString(StringView format, Handler&& handler)
    {
        using namespace Details;
        const char* begin = format.Raw();
        const char* end   = begin + format.Size();

        auto        text  = [&](const char* from, const char* to)
        {
            // Unescape "}}" while emitting literal text.
            while (from != to)
            {
                const char* p = from;
                while (p != to && *p != '}') ++p;
                if (p == to) return handler.OnText(from, to);

                ++p;
                if (p == to || *p != '}')
                {
                    ReportFormatError("unmatched '}' in format string");
                    handler.OnText(from, p);
                    from = p;
                    continue;
                }
                handler.OnText(from, p);
                from = p + 1;
            }
        };

        while (begin != end)
        {
            const char* p = begin;
            while (p != end && *p != '{') ++p;
            text(begin, p);
            if (p == end) return;

            // Replacement field.
            begin = p + 1;
            if (begin == end)
            {
                ReportFormatError("invalid format string");
                return;
            }
            if (*begin == '{')
            {
                handler.OnText(begin, begin + 1);
                ++begin;
                continue;
            }
            if (*begin == '}')
            {
                handler.OnReplacementField(handler.OnArgID(), begin);
                ++begin;
                continue;
            }

            struct
            {
                Handler&       Inner;
                isize          ID = 0;

                constexpr void OnAuto() { ID = Inner.OnArgID(); }
                constexpr void OnIndex(isize id) { ID = Inner.OnArgID(id); }
                constexpr void OnName(StringView name)
                {
                    ID = Inner.OnArgID(name);
                }
            } adapter{handler};

            begin  = ParseArgID(begin, end, adapter);
            char c = begin != end ? *begin : '\0';
            if (c == '}') handler.OnReplacementField(adapter.ID, begin);
            else if (c == ':')
            {
                begin = handler.OnFormatSpecs(adapter.ID, begin + 1, end);
                if (begin == end || *begin != '}')
                {
                    ReportFormatError("unknown format specifier");
                    return;
                }
            }
            else
            {
                ReportFormatError("missing '}' in format string");
                return;
            }
            ++begin;
        }
    }
}; // namespace Prism
