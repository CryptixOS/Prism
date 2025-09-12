/*
 * Created by v1tr10l7 on 24.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/String/Printf.hpp>
#include <Prism/String/StringUtils.hpp>

namespace Prism::Printf
{
    using namespace StringUtils;

    FormatSpec FormatParser::operator()(StringView::Iterator& it, va_list& args)
    {
        FormatSpec specs;

        struct
        {
            enum State     State = State::eBegin;
            constexpr void operator()(enum State state)
            {
                if (State >= state) assert("invalid format specifier");
                State = state;
            }
        } enterState;

        for (; *it;)
        {
            switch (*it)
            {
                case '\0': return specs;
                case '%':
                    enterState(State::eBegin);
                    break;
                    ++it;
                    if (*it == '%')
                    {
                        specs.Type = ArgumentType::eChar;
                        return specs;
                    }
                    continue;
                case '-':
                    enterState(State::eFlags);
                    specs.JustifyLeft = true;
                    break;
                case '+':
                    enterState(State::eFlags);
                    specs.Sign = Sign::ePlus;
                    break;
                case ' ':
                    enterState(State::eFlags);
                    specs.Sign = Sign::eSpace;
                    break;
                case '#':
                    enterState(State::eFlags);
                    specs.PrintBase = true;
                    break;
                case '0':
                    enterState(State::eFlags);
                    specs.ZeroPad = true;
                    break;
                case '1' ... '9':
                {
                    enterState(State::eWidth);
                    const char* start = it;
                    for (; StringUtils::IsDigit(*it); it++);

                    StringView len(start, it - start);
                    specs.Length = ToNumber<isize>(len, 10);

                    continue;
                }
                case '*':
                    enterState(State::eWidth);
                    specs.Length = static_cast<isize>(PrismVaArg(args, i32));
                    break;
                case '.':
                {
                    enterState(State::ePrecision);
                    ++it;
                    const char* start = it;
                    for (; StringUtils::IsDigit(*it); it++);

                    usize      strLength = it - start;
                    StringView len(start, strLength);
                    if (!len.Empty())
                    {
                        specs.Precision = ToNumber<isize>(len, 10);
                        continue;
                    }
                    else if (*it == '*')
                        specs.Precision
                            = static_cast<isize>(PrismVaArg(args, i32));
                    else
                        assert(
                            "Printf: Invalid format specifier, parsing "
                            "precision failed");

                    PM_FALLTHROUGH;
                }
                case 'l':
                    enterState(State::eLength);
                    specs.Type = ArgumentType::eLong;
                    ++it;
                    if (*it == 'l') specs.Type = ArgumentType::eLongLong;
                    else continue;
                    break;
                case 'z':
                    enterState(State::eLength);
                    specs.Type = ArgumentType::eSize;
                    break;

                default: goto loop_end;
            }
            it++;
        }
    loop_end:

        switch (*it)
        {
            case 'b':
                specs.Base = 2;
                specs.Type
                    = static_cast<ArgumentType>(ToUnderlying(specs.Type) + 5);
                break;
            case 'o':
                specs.Base = 8;
                specs.Type
                    = static_cast<ArgumentType>(ToUnderlying(specs.Type) + 5);
                break;
            case 'p':
                specs.Type = ArgumentType::eSize;
                specs.Type
                    = static_cast<ArgumentType>(ToUnderlying(specs.Type) + 5);
                specs.Base = 16;
                break;
            case 'X': specs.UpperCase = true; PM_FALLTHROUGH;
            case 'x':
                specs.Base = 16;
                if (specs.Type < ArgumentType::eInteger)
                    specs.Type = ArgumentType::eInteger;
                specs.Type
                    = static_cast<ArgumentType>(ToUnderlying(specs.Type) + 5);
                break;
            case 'u':
                specs.Type
                    = static_cast<ArgumentType>(ToUnderlying(specs.Type) + 5);
                break;
            case 'd': PM_FALLTHROUGH;
            case 'i': break; PM_FALLTHROUGH;
            case 'c':
                specs.Type = ArgumentType::eChar;
                break;
                PM_FALLTHROUGH;
            case 's':
                specs.Type = ArgumentType::eString;
                break;
                PM_FALLTHROUGH;
            case 'n': specs.Type = ArgumentType::eOutWrittenCharCount; break;
        }
        ++it;

        return specs;
    }
}; // namespace Prism::Printf
