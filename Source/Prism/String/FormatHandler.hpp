/*
 * Created by v1tr10l7 on 11.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/String/FormatterContext.hpp>

namespace Prism
{
    enum class Alignment
    {
        eNone,
        eLeft,
        eRight,
        eCenter,
    };
    struct FormatSpec
    {
        Alignment Align           = Alignment::eNone;
        char      PaddingChar     = ' ';
        usize     Width           = 0;
        bool      PrintBasePrefix = false;
        bool      UpperCase       = false;
        bool      PrintAsAscii    = false;

        enum Base
        {
            eBinary      = 2,
            eOctal       = 8,
            eDecimal     = 10,
            eHexadecimal = 16,
        } Base
            = eDecimal;
    };

    struct FormatSpecParser
    {
        enum class State
        {
            eBegin,
            eAlign,
            eSign,
            eHash,
            eZero,
            eWidth,
            ePrecision,
            eEnd,
        };

        inline constexpr Alignment ParseAlign(char c)
        {
            switch (c)
            {
                case '<': return Alignment::eLeft;
                case '>': return Alignment::eRight;
                case '^': return Alignment::eCenter;
                default: break;
            }

            return Alignment::eNone;
        }
        inline constexpr usize ParseWidth(const char*& fmt)
        {
            usize width = 0;
            while (StringUtils::IsDigit(*fmt))
            {
                width = width * 10 + (*fmt - '0');
                ++fmt;
            }

            return width;
        }
        constexpr FormatSpec operator()(const char*& fmt)
        {
            struct
            {
                enum State     State = State::eBegin;

                constexpr void operator()(enum State state)
                {
                    if (State >= state) assert("invalid format specifier");
                    State = state;
                }
            } enterState;
            FormatSpec spec;

            for (;;)
            {
                switch (*fmt)
                {
                    case ':':
                        enterState(State::eBegin);
                        ++fmt;
                        break;
                    case '}': return spec;
                    case '<':
                    case '>':
                    case '^':
                        enterState(State::eAlign);
                        spec.Align = ParseAlign(*fmt);
                        ++fmt;
                        break;
                    case '0':
                        enterState(State::eZero);
                        spec.PaddingChar = '0';
                        ++fmt;
                        break;
                    case '1' ... '9':
                        enterState(State::eWidth);
                        spec.Width = ParseWidth(fmt);
                        break;
                    case '#':
                        enterState(State::eHash);
                        spec.PrintBasePrefix = true;
                        ++fmt;
                        break;

                    case 'c':
                        enterState(State::eEnd);
                        spec.PrintAsAscii = true;
                        ++fmt;
                        break;

                    case 'b':
                        enterState(State::eEnd);
                        spec.Base = FormatSpec::eBinary;
                        ++fmt;
                        break;
                    case 'o':
                        enterState(State::eEnd);
                        spec.Base = FormatSpec::eOctal;
                        ++fmt;
                        break;
                    case 'X': spec.UpperCase = true; [[fallthrough]];
                    case 'x':
                        enterState(State::eEnd);
                        spec.Base = FormatSpec::eHexadecimal;
                        ++fmt;
                        break;

                    default: assert("Invalid format specifier");
                }
            }

            return spec;
        }
    };
    constexpr const char* ParseFormatSpec(const char* fmt, FormatSpec& spec)
    {
        FormatSpecParser parser;
        spec = parser(fmt);

        return fmt;
    }

    template <typename StringFormatter, typename Char, typename... Args>
    struct FormatHandler
    {
        using ValueType = char;
        BasicFormatterContext<ValueType> ParseContext;
        StringFormatter&                 Formatter;

        FormatHandler(StringFormatter& formatter, BasicStringView<Char> str,
                      Args&&... args)
            : ParseContext(str)
            , Formatter(formatter)
        {
        }

        void OnText(const ValueType* begin, const ValueType* end)
        {
            auto text = BasicStringView<ValueType>(begin, end - begin);
            Formatter.Context() << text;
        }
        constexpr void OnError(const char* message) { assert(message); }
        constexpr auto OnFormatSpecs(const Char* begin, const Char* end,
                                     Args&&... args) -> const Char*
        {
            FormatSpec spec;
            begin = ParseFormatSpec(begin, spec);
            Formatter.VisitArgument(Forward<Args>(args)..., spec);
            return begin;
        }
    };

    template <typename Char, typename... Args>
    class FormatStringChecker
    {
      private:
        using ParseContextType             = CompileFormatterContext<Char>;
        static constexpr int ArgumentCount = sizeof...(Args);

        using ParseFn                      = const Char* (*)(ParseContextType&);

        ParseContextType m_Context;

      public:
        explicit constexpr FormatStringChecker(BasicStringView<Char> fmt)
            : m_Context(fmt, ArgumentCount)

        {
        }

        constexpr void OnText(const Char*, const Char*) {}

        constexpr auto OnArgID() -> int { return m_Context.NextArgID(); }
        constexpr auto OnArgID(int id) -> int
        {
            return m_Context.CheckArgID(id), id;
        }
        constexpr auto OnArgID(BasicStringView<Char> id) -> int
        {
#if FMT_USE_NONTYPE_TEMPLATE_ARGS
            auto index = GetArgIndexByName<Args...>(id);
            if (index < 0) OnError("named argument is not found");
            return index;
#else
            (void)id;
            OnError(
                "compile-time checks for named arguments require C++20 "
                "support");
            return 0;
#endif
        }

        constexpr void OnReplacementField(int id, const Char* begin)
        {
            OnFormatSpecs(id, begin, begin); // Call parse() on empty specs.
        }

        constexpr auto OnFormatSpecs(int id, const Char* begin, const Char*)
            -> const Char*
        {
            m_Context.AdvanceTo(begin);
            return begin;
        }

        constexpr void OnError(const char* message) { assert(message); }
    };
}; // namespace Prism
