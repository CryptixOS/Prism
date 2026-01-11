/*
 * Created by v1tr10l7 on 24.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/TypeTraits.hpp>
#include <Prism/String/FormatHandler.hpp>
#include <Prism/String/FormatString.hpp>
#include <Prism/String/StringBuilder.hpp>

namespace Prism
{
    template <bool ConstExpr, typename T, typename S = const T*>
    constexpr auto Find(S first, S last, T value, S& out) -> bool
    {
        for (out = first; out != last; ++out)
        {
            if (*out == value) return true;
        }
        return false;
    }
    template <typename OutContext>
    class Formatter
    {
      public:
        constexpr Formatter(OutContext& builder)
            : m_Builder(builder)
        {
        }

        template <typename T>
        constexpr void VisitArgument(const T& value, const FormatSpec& spec)
        {
            if constexpr (IsIntegralV<T>)
                VisitArgument(static_cast<u64>(value), spec);
            else if constexpr (IsSameV<T, const char*>
                               || IsSameV<T, StringView>)
                m_Builder << value;
            else static_assert("Unsupported format");
        }
        constexpr void VisitArgument(const char* s, const FormatSpec& spec)
        {
            m_Builder << s;
        }
        constexpr void VisitArgument(const String& s, const FormatSpec& spec)
        {
            m_Builder << s;
        }
        constexpr void VisitArgument(char c, const FormatSpec& spec)
        {
            m_Builder << c;
        }
        constexpr void VisitArgument(u64 val, const FormatSpec& spec)
        {
            if (spec.PrintAsAscii)
                return VisitArgument(static_cast<char>(val), spec);
            if (spec.PrintBasePrefix)
            {
                if (spec.Base == FormatSpec::eBinary) m_Builder.Append("0b");
                else if (spec.Base == FormatSpec::eOctal) m_Builder.Append("0");
                else if (spec.Base == FormatSpec::eHexadecimal)
                    m_Builder.Append(spec.UpperCase ? "0X" : "0x");
            }

            auto  string = StringUtils::ToString<u64>(val, spec.Base);
            usize length = string.Size();

            // Padding
            if (length < spec.Width)
                for (usize i = 0; i < spec.Width - length; ++i)
                    m_Builder << spec.PaddingChar;

            m_Builder << string;
        }

        OutContext& Context() { return m_Builder; }

      private:
        OutContext& m_Builder;
    };

    template <typename Char, typename Handler, typename... Args>
    constexpr auto ParseReplacementField(const Char* begin, const Char* end,
                                         Handler&& handler, Args&&... args)
        -> const Char*
    {
        ++begin;
        if (begin == end) handler.OnError("Invalid format string");
        if (*begin == '{') handler.OnText(begin, begin + 1);
        // if (*begin == '}')
        //     handler.OnReplacementField(handler.OnArgID(), begin);
        else {
            Char c = begin != end ? *begin : Char();
            if (c == ':')
            {
                begin = handler.OnFormatSpecs(begin + 1, end,
                                              Forward<Args>(args)...);
                if (begin == end || *begin != '}')
                    return handler.OnError("Unknown format specifier"), end;
            }
            else return handler.OnError("Missing '}' in format string"), end;
        }

        return begin + 1;

        FormatSpec spec;
        begin = ParseFormatSpec(begin, spec);
        handler.Formatter.VisitArgument(Forward<Args>(args)..., spec);
        return begin;
    }
    template <bool IsConstExpr, typename Char, typename Handler,
              typename... Args>
    constexpr inline void ParseFormatString(BasicStringView<Char> formatString,
                                            Handler&& handler, Args&&... args)
    {
        auto begin = formatString.Raw();
        auto end   = begin + formatString.Size();

        struct Writer
        {
            constexpr void operator()(const Char* from, const Char* to)
            {
                if (from == to) return;
                for (;;)
                {
                    const Char* p = nullptr;
                    if (!Find<IsConstExpr>(from, to, Char('}'), p))
                        return Dispatcher.OnText(from, to);
                    ++p;
                    if (p == to || *p != '}')
                        return Dispatcher.OnError(
                            "unmatched '}' in format string");
                    Dispatcher.OnText(from, p);
                    from = p + 1;
                }
            }
            Handler& Dispatcher;
        } write = {handler};
        while (begin != end)
        {
            const char* p = begin;
            if (*p != '{'
                && !Find<IsConstantEvaluated()>(begin + 1, end, '{', p))
                return write(begin, p);
            write(begin, p);

            begin = ParseReplacementField(p, end, handler,
                                          Forward<Args>(args)...);
        }
    }
    template <typename Context, typename... Args>
    void FormatTo(Context& context, StringView fmt, Args&&... args)
    {
        Formatter     formatter(context);
        FormatHandler handler(formatter, fmt, Forward<Args>(args)...);
        ParseFormatString<false>(fmt, handler, Forward<Args>(args)...);
    }

    template <typename... Args>
    String Format1(StringView fmt, Args&&... args)
    {
        StringBuilder<char> context;
        Formatter           formatter(context);

        FormatTo(formatter, fmt.begin(), Forward<Args>(args)...);
        return context;
    }

    template <typename... Args>
    void Print(FormatString<Args...>&& fmt, Args const&... parameters)
    {
    }

    template <typename... Args>
    consteval FormatString<Args...> MakeFormatString(StringView fmt, Args&&...)
    {
        return FormatString<Args...>(fmt);
    }
    template <typename... Args>
    PM_NODISCARD inline constexpr String DoFormat(StringView fmt,
                                                   Args&&... args)
    {
        constexpr auto checked = MakeFormatString(fmt, args...);
        return Format1(fmt, Forward<Args>(args)...);
    }

    template <typename... Args>
    PM_NODISCARD inline auto Format(fmt::format_string<Args...> fmt,
                                     Args&&... args)
    {
        return fmt::format(fmt, Forward<Args>(args)...);
    }
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::Format;
using Prism::Formatter;
#endif
