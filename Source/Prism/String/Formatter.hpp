/*
 * Created by v1tr10l7 on 24.04.2025.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/TypeTraits.hpp>
#include <Prism/Debug/Log.hpp>
#include <Prism/Memory/Pointer.hpp>
#include <Prism/String/FormatHandler.hpp>
#include <Prism/String/FormatString.hpp>
#include <Prism/String/String.hpp>
#include <Prism/String/StringUtils.hpp>

/**
 * Prism formatting library: a freestanding replacement for {fmt} with the
 * same format string syntax and the feature set of the patched fmt 10.2
 * Prism used to ship (everything except floating point, int128 and the
 * chrono/ranges/std/ostream/printf headers).
 *
 *   Format("{} + {:#x} = {:>8}", a, b, c)       -> String
 *   FormatTo(out, "...", args...)               -> output iterator / appends
 *   FormatToN(out, n, "...", args...)           -> {Out, Size}, truncating
 *   FormattedSize("...", args...)               -> usize
 *   FormatToSink(sink, "...", args...)          -> sink(const char*, usize)
 *   Print/Println("...", args...)               -> Log::Print, no allocation
 *   VFormat / VFormatTo + MakeFormatArgs        -> type-erased variants
 *   Runtime(str)                                -> skip compile-time checks
 *   Arg("name", v), "name"_a = v                -> named arguments
 *   Join(range, sep), Ptr(p), Underlying(e), GroupDigits(v), ToString(v)
 *   Styled(v, style), Format(style, "...")      -> ANSI colors/emphasis
 *
 * Format strings are checked at compile time. Formatting never allocates
 * except where the result is a String.
 */
namespace Prism
{
    namespace Details
    {
        // UTF-8 helpers

        /// Decodes one code point; on malformed input consumes one byte and
        /// returns false.
        constexpr bool DecodeUtf8(const char* p, const char* end, u32& cp,
                                  usize& length)
        {
            auto lead = static_cast<u8>(*p);
            length    = 1;
            cp        = lead;
            if (lead < 0x80) return true;

            usize need = 0;
            if ((lead >> 5) == 0x06)
            {
                need = 1;
                cp   = lead & 0x1f;
            }
            else if ((lead >> 4) == 0x0e)
            {
                need = 2;
                cp   = lead & 0x0f;
            }
            else if ((lead >> 3) == 0x1e)
            {
                need = 3;
                cp   = lead & 0x07;
            }
            else return false;

            if (static_cast<usize>(end - p) <= need) return false;
            for (usize i = 1; i <= need; ++i)
            {
                auto c = static_cast<u8>(p[i]);
                if ((c & 0xc0) != 0x80) return false;
                cp = (cp << 6) | (c & 0x3f);
            }

            constexpr u32 minimum[] = {0, 0x80, 0x800, 0x10000};
            if (cp < minimum[need] || cp > 0x10ffff
                || (cp >= 0xd800 && cp <= 0xdfff))
                return false;

            length = need + 1;
            return true;
        }

        /// Terminal column width of a code point (same table as fmt).
        constexpr usize DisplayWidth(u32 cp)
        {
            return 1
                 + (cp >= 0x1100
                    && (cp <= 0x115f || cp == 0x2329 || cp == 0x232a
                        || (cp >= 0x2e80 && cp <= 0xa4cf && cp != 0x303f)
                        || (cp >= 0xac00 && cp <= 0xd7a3)
                        || (cp >= 0xf900 && cp <= 0xfaff)
                        || (cp >= 0xfe10 && cp <= 0xfe19)
                        || (cp >= 0xfe30 && cp <= 0xfe6f)
                        || (cp >= 0xff00 && cp <= 0xff60)
                        || (cp >= 0xffe0 && cp <= 0xffe6)
                        || (cp >= 0x20000 && cp <= 0x2fffd)
                        || (cp >= 0x30000 && cp <= 0x3fffd)
                        || (cp >= 0x1f300 && cp <= 0x1f64f)
                        || (cp >= 0x1f900 && cp <= 0x1f9ff)));
        }

        constexpr usize ComputeWidth(StringView str)
        {
            const char* p     = str.Raw();
            const char* end   = p + str.Size();
            usize       width = 0;
            while (p != end)
            {
                u32   cp     = 0;
                usize length = 0;
                width += DecodeUtf8(p, end, cp, length) ? DisplayWidth(cp) : 1;
                p += length;
            }
            return width;
        }

        /// Byte offset of the code point at index `n` (or the size).
        constexpr usize CodePointIndex(StringView str, usize n)
        {
            usize count = 0;
            for (usize i = 0; i < str.Size(); ++i)
                if ((static_cast<u8>(str[i]) & 0xc0) != 0x80 && ++count > n)
                    return i;
            return str.Size();
        }

        // Padding

        template <typename Writer>
        void WritePadded(FormatBuffer& out, const FormatSpecs& specs,
                         usize width, FormatAlign defaultAlign,
                         Writer&& write)
        {
            usize specWidth = specs.Width > 0 ? usize(specs.Width) : 0;
            usize padding   = specWidth > width ? specWidth - width : 0;
            auto  align
                = specs.Align == FormatAlign::eNone ? defaultAlign : specs.Align;

            usize left = 0;
            if (align == FormatAlign::eRight || align == FormatAlign::eNumeric)
                left = padding;
            else if (align == FormatAlign::eCenter) left = padding / 2;

            out.Fill(left, specs.Fill, specs.FillSize);
            write();
            out.Fill(padding - left, specs.Fill, specs.FillSize);
        }

        // Integers

        constexpr usize FormatDigits(char* end, u64 value, u32 base,
                                     bool upper, bool grouped)
        {
            const char* digits
                = upper ? "0123456789ABCDEF" : "0123456789abcdef";
            char* p     = end;
            usize count = 0;
            do {
                if (grouped && count > 0 && count % 3 == 0) *--p = ',';
                *--p = digits[value % base];
                value /= base;
                ++count;
            } while (value != 0);
            return static_cast<usize>(end - p);
        }

        inline void WriteInteger(FormatBuffer& out, u64 absValue,
                                 bool negative, const FormatSpecs& specs)
        {
            char  prefix[4]{};
            usize prefixSize = 0;
            if (negative) prefix[prefixSize++] = '-';
            else if (specs.Sign == FormatSign::ePlus) prefix[prefixSize++] = '+';
            else if (specs.Sign == FormatSign::eSpace)
                prefix[prefixSize++] = ' ';

            u32  base  = 10;
            bool upper = false;
            switch (specs.Type)
            {
                case FormatPresentation::eHexUpper: upper = true; [[fallthrough]];
                case FormatPresentation::eHexLower:
                    base = 16;
                    if (specs.Alt)
                    {
                        prefix[prefixSize++] = '0';
                        prefix[prefixSize++] = upper ? 'X' : 'x';
                    }
                    break;
                case FormatPresentation::eBinaryUpper: upper = true; [[fallthrough]];
                case FormatPresentation::eBinaryLower:
                    base = 2;
                    if (specs.Alt)
                    {
                        prefix[prefixSize++] = '0';
                        prefix[prefixSize++] = upper ? 'B' : 'b';
                    }
                    break;
                case FormatPresentation::eOctal:
                    base = 8;
                    if (specs.Alt && absValue != 0) prefix[prefixSize++] = '0';
                    break;
                default: break;
            }

            char  digits[96];
            char* digitsEnd = digits + sizeof(digits);
            usize digitCount
                = FormatDigits(digitsEnd, absValue, base, upper,
                               specs.Localized && base == 10);
            const char* digitsBegin = digitsEnd - digitCount;
            usize       size        = prefixSize + digitCount;

            if (specs.Align == FormatAlign::eNumeric)
            {
                usize width = specs.Width > 0 ? usize(specs.Width) : 0;
                out.Append(prefix, prefixSize);
                if (width > size) out.Fill(width - size, "0", 1);
                out.Append(digitsBegin, digitCount);
                return;
            }

            WritePadded(out, specs, size, FormatAlign::eRight,
                        [&]
                        {
                            out.Append(prefix, prefixSize);
                            out.Append(digitsBegin, digitCount);
                        });
        }

        inline void WritePointer(FormatBuffer& out, u64 value,
                                 const FormatSpecs& specs)
        {
            char  digits[24];
            char* end   = digits + sizeof(digits);
            usize count = FormatDigits(end, value, 16, false, false);
            usize size  = count + 2;

            if (specs.Align == FormatAlign::eNumeric)
            {
                usize width = specs.Width > 0 ? usize(specs.Width) : 0;
                out.Append("0x", 2);
                if (width > size) out.Fill(width - size, "0", 1);
                out.Append(end - count, count);
                return;
            }
            WritePadded(out, specs, size, FormatAlign::eRight,
                        [&]
                        {
                            out.Append("0x", 2);
                            out.Append(end - count, count);
                        });
        }

        // Strings and characters

        /// Calls emit(piece, width) for each piece of the escaped form of
        /// `str` (the '?' debug presentation), without the quotes.
        template <typename Emit>
        constexpr void ForEachEscaped(StringView str, char quote, Emit&& emit)
        {
            constexpr const char* hex = "0123456789abcdef";
            const char*           p   = str.Raw();
            const char*           end = p + str.Size();
            while (p != end)
            {
                u32   cp     = 0;
                usize length = 0;
                bool  valid  = DecodeUtf8(p, end, cp, length);

                char  escape[12]{};
                usize escapeSize = 0;
                auto  hexEscape  = [&](u32 value, char kind, usize digits)
                {
                    escape[escapeSize++] = '\\';
                    escape[escapeSize++] = kind;
                    for (usize i = digits; i > 0; --i)
                        escape[escapeSize++] = hex[(value >> ((i - 1) * 4)) & 0xf];
                };

                if (!valid) hexEscape(static_cast<u8>(*p), 'x', 2);
                else if (cp == '\n')
                {
                    escape[escapeSize++] = '\\';
                    escape[escapeSize++] = 'n';
                }
                else if (cp == '\r')
                {
                    escape[escapeSize++] = '\\';
                    escape[escapeSize++] = 'r';
                }
                else if (cp == '\t')
                {
                    escape[escapeSize++] = '\\';
                    escape[escapeSize++] = 't';
                }
                else if (cp == '\\' || cp == static_cast<u8>(quote))
                {
                    escape[escapeSize++] = '\\';
                    escape[escapeSize++] = static_cast<char>(cp);
                }
                else if (cp < 0x20 || cp == 0x7f) hexEscape(cp, 'x', 2);

                if (escapeSize > 0) emit(StringView(escape, escapeSize), escapeSize);
                else emit(StringView(p, length), DisplayWidth(cp));
                p += length;
            }
        }

        inline void WriteEscaped(FormatBuffer& out, StringView str, char quote,
                                 const FormatSpecs& specs)
        {
            usize width = 2;
            if (specs.Width > 0)
                ForEachEscaped(str, quote,
                               [&](StringView, usize w) { width += w; });

            WritePadded(out, specs, width, FormatAlign::eLeft,
                        [&]
                        {
                            out.PushBack(quote);
                            ForEachEscaped(str, quote, [&](StringView piece, usize)
                                           { out.Append(piece); });
                            out.PushBack(quote);
                        });
        }

        inline void WriteString(FormatBuffer& out, StringView str,
                                const FormatSpecs& specs)
        {
            if (specs.Type == FormatPresentation::eDebug)
                return WriteEscaped(out, str, '"', specs);

            usize size = str.Size();
            if (specs.Precision >= 0 && usize(specs.Precision) < size)
                size = CodePointIndex(str, usize(specs.Precision));

            StringView view(str.Raw(), size);
            usize      width = specs.Width > 0 ? ComputeWidth(view) : 0;
            WritePadded(out, specs, width, FormatAlign::eLeft,
                        [&] { out.Append(view); });
        }

        inline void WriteChar(FormatBuffer& out, char c,
                              const FormatSpecs& specs)
        {
            if (specs.Type == FormatPresentation::eDebug)
                return WriteEscaped(out, StringView(&c, 1), '\'', specs);
            WritePadded(out, specs, 1, FormatAlign::eLeft,
                        [&] { out.PushBack(c); });
        }

        inline constexpr bool IsIntegerPresentation(FormatPresentation type)
        {
            switch (type)
            {
                case FormatPresentation::eDecimal:
                case FormatPresentation::eOctal:
                case FormatPresentation::eHexLower:
                case FormatPresentation::eHexUpper:
                case FormatPresentation::eBinaryLower:
                case FormatPresentation::eBinaryUpper: return true;
                default: break;
            }
            return false;
        }

        /// Writes a built-in (non-custom) argument.
        inline void WriteArg(FormatBuffer& out, const FormatArg& arg,
                             const FormatSpecs& specs)
        {
            switch (arg.Type)
            {
                case FormatArgType::eInt:
                    if (specs.Type == FormatPresentation::eChar)
                        return WriteChar(out, static_cast<char>(arg.Int), specs);
                    return WriteInteger(
                        out,
                        arg.Int < 0 ? 0 - static_cast<u64>(arg.Int)
                                    : static_cast<u64>(arg.Int),
                        arg.Int < 0, specs);
                case FormatArgType::eUInt:
                    if (specs.Type == FormatPresentation::eChar)
                        return WriteChar(out, static_cast<char>(arg.UInt),
                                         specs);
                    return WriteInteger(out, arg.UInt, false, specs);
                case FormatArgType::eBool:
                    if (IsIntegerPresentation(specs.Type))
                        return WriteInteger(out, arg.Bool ? 1 : 0, false,
                                            specs);
                    return WriteString(out, arg.Bool ? "true" : "false",
                                       specs);
                case FormatArgType::eChar:
                    if (IsIntegerPresentation(specs.Type))
                        return WriteInteger(out, static_cast<u8>(arg.Char),
                                            false, specs);
                    return WriteChar(out, arg.Char, specs);
                case FormatArgType::eCString:
                    if (specs.Type == FormatPresentation::ePointer)
                        return WritePointer(
                            out, reinterpret_cast<__UINTPTR_TYPE__>(arg.CString),
                            specs);
                    return WriteString(
                        out, arg.CString ? StringView(arg.CString) : "(null)",
                        specs);
                case FormatArgType::eString:
                    return WriteString(
                        out, StringView(arg.String.Data, arg.String.Size),
                        specs);
                case FormatArgType::ePointer:
                    return WritePointer(
                        out, reinterpret_cast<__UINTPTR_TYPE__>(arg.Pointer),
                        specs);
                default: break;
            }
        }

        // Dynamic width / precision

        inline i32 GetDynamicSpec(const FormatArg& arg, i32 fallback)
        {
            switch (arg.Type)
            {
                case FormatArgType::eInt:
                    if (arg.Int < 0)
                    {
                        ReportFormatError("negative width/precision");
                        return fallback;
                    }
                    if (arg.Int > MaxSpecValue)
                    {
                        ReportFormatError("number is too big");
                        return MaxSpecValue;
                    }
                    return static_cast<i32>(arg.Int);
                case FormatArgType::eUInt:
                    if (arg.UInt > u64(MaxSpecValue))
                    {
                        ReportFormatError("number is too big");
                        return MaxSpecValue;
                    }
                    return static_cast<i32>(arg.UInt);
                default: break;
            }
            ReportFormatError("width/precision is not integer");
            return fallback;
        }

        inline FormatArg GetDynamicArg(const DynamicSpecRef& ref,
                                       FormatContext&        ctx)
        {
            if (ref.RefKind == DynamicSpecRef::Kind::eIndex)
                return ctx.Arg(ref.Index);
            return ctx.Arg(StringView(ref.Name, ref.NameLength));
        }

        inline FormatSpecs ResolveSpecs(const DynamicFormatSpecs& specs,
                                        FormatContext&            ctx)
        {
            FormatSpecs result = specs;
            if (specs.WidthRef.RefKind != DynamicSpecRef::Kind::eNone)
                result.Width
                    = GetDynamicSpec(GetDynamicArg(specs.WidthRef, ctx), 0);
            if (specs.PrecisionRef.RefKind != DynamicSpecRef::Kind::eNone)
                result.Precision = GetDynamicSpec(
                    GetDynamicArg(specs.PrecisionRef, ctx), -1);
            return result;
        }

        // Argument construction

        template <typename T>
        constexpr FormatArg MakeNativeArg(const T& value)
        {
            FormatArg arg;
            arg.Type = NativeArgType<T>();
            if constexpr (IsSameV<T, bool>) arg.Bool = value;
            else if constexpr (IsSameV<T, char>) arg.Char = value;
            else if constexpr (IsFormatIntegerV<T>)
            {
                if constexpr (IsSignedV<T>) arg.Int = static_cast<i64>(value);
                else arg.UInt = static_cast<u64>(value);
            }
            else if constexpr (IsFormatCStringV<T>) arg.CString = value;
            else if constexpr (IsFormatStringV<T>)
                arg.String = {value.Raw(), value.Size()};
            else if constexpr (IsNullPointerV<T>) arg.Pointer = nullptr;
            else
                arg.Pointer = reinterpret_cast<const void*>(
                    reinterpret_cast<__UINTPTR_TYPE__>(value));
            return arg;
        }

        template <typename T>
        void FormatCustomArg(const void* value, FormatParseContext& parseCtx,
                             FormatContext& ctx)
        {
            FormatterFor<T> formatter{};
            parseCtx.AdvanceTo(ParseWith(formatter, parseCtx));
            formatter.Format(*static_cast<const T*>(value), ctx);
        }

        template <typename T>
        FormatArg MakeArg(const T& value)
        {
            using Key = FormatKeyType<T>;
            if constexpr (IsNativeFormatTypeV<Key>)
                return MakeNativeArg<Key>(value);
            else
            {
                FormatArg arg;
                arg.Type   = FormatArgType::eCustom;
                arg.Custom = {&value, &FormatCustomArg<Key>};
                return arg;
            }
        }

        /// Runtime counterpart of FormatStringChecker: formats as it parses.
        class FormatHandler
        {
          public:
            FormatHandler(FormatBuffer& out, StringView format,
                          FormatArgs args)
                : m_ParseContext(format, static_cast<isize>(args.Size()))
                , m_Context(out, args)
            {
            }

            void OnText(const char* begin, const char* end)
            {
                m_Context.Buffer().Append(begin, end);
            }
            isize OnArgID() { return m_ParseContext.NextArgID(); }
            isize OnArgID(isize id)
            {
                m_ParseContext.CheckArgID(id);
                return id;
            }
            isize OnArgID(StringView name)
            {
                m_ParseContext.CheckArgID(name);
                isize id = m_Context.ArgID(name);
                if (id < 0) ReportFormatError("argument not found");
                return id;
            }

            void OnReplacementField(isize id, const char* begin)
            {
                FormatArg arg = m_Context.Arg(id);
                if (arg.Type == FormatArgType::eCustom)
                {
                    m_ParseContext.AdvanceTo(begin);
                    arg.Custom.Format(arg.Custom.Value, m_ParseContext,
                                      m_Context);
                    return;
                }
                WriteArg(m_Context.Buffer(), arg, FormatSpecs{});
            }
            const char* OnFormatSpecs(isize id, const char* begin,
                                      const char* end)
            {
                FormatArg arg = m_Context.Arg(id);
                if (arg.Type == FormatArgType::eCustom)
                {
                    m_ParseContext.AdvanceTo(begin);
                    arg.Custom.Format(arg.Custom.Value, m_ParseContext,
                                      m_Context);
                    return m_ParseContext.begin();
                }

                DynamicFormatSpecs specs;
                begin
                    = ParseFormatSpecs(begin, end, specs, m_ParseContext, arg.Type);
                if (begin != end && *begin == '}')
                    WriteArg(m_Context.Buffer(), arg,
                             ResolveSpecs(specs, m_Context));
                return begin;
            }

          private:
            FormatParseContext m_ParseContext;
            FormatContext      m_Context;
        };

        /// Lets a sink be held by reference inside a SinkFormatBuffer.
        template <typename Sink>
        struct SinkRef
        {
            Sink* Target;
            void  operator()(const char* data, usize size) const
            {
                (*Target)(data, size);
            }
        };
    } // namespace Details

    // Argument lists

    /**
     * Owns the type-erased arguments for one formatting call. It references
     * the values, so it must not outlive them (pass MakeFormatArgs(...)
     * directly to VFormat*).
     */
    template <typename... Args>
    class FormatArgStore
    {
        static constexpr usize Count      = sizeof...(Args);
        static constexpr usize NamedCount = Details::CountNamedArgs<Args...>();

      public:
        explicit FormatArgStore(const Args&... args)
        {
            usize index = 0, named = 0;
            (Store(args, index, named), ...);
            (void)index;
            (void)named;
        }

        operator FormatArgs() const
        {
            return FormatArgs(m_Args, Count, m_Named, NamedCount);
        }

      private:
        FormatArg          m_Args[Count > 0 ? Count : 1];
        FormatNamedArgInfo m_Named[NamedCount > 0 ? NamedCount : 1];

        template <typename T>
        void Store(const T& value, usize& index, usize& named)
        {
            using Traits = Details::NamedArgTraits<T>;
            if constexpr (Traits::IsNamed)
            {
                const char* name = nullptr;
                if constexpr (Traits::IsStatic) name = Traits::Name_;
                else name = value.Name;

                m_Named[named++]  = {name, static_cast<isize>(index)};
                m_Args[index++] = Details::MakeArg(value.Value);
            }
            else m_Args[index++] = Details::MakeArg(value);
        }
    };

    template <typename... Args>
    FormatArgStore<RemoveCvRefType<Args>...> MakeFormatArgs(const Args&... args)
    {
        return FormatArgStore<RemoveCvRefType<Args>...>(args...);
    }

    // Core entry points

    /// Formats into any FormatBuffer.
    inline void VFormatTo(FormatBuffer& out, StringView format, FormatArgs args)
    {
        ParseFormatString(format, Details::FormatHandler(out, format, args));
    }

    /// Formats into `sink(const char* data, usize size)`, in chunks, without
    /// allocating.
    template <typename Sink>
    void VFormatToSink(Sink&& sink, StringView format, FormatArgs args)
    {
        using Ref = Details::SinkRef<RemoveReferenceType<Sink>>;
        SinkFormatBuffer<Ref> buffer(Ref{&sink});
        VFormatTo(buffer, format, args);
        buffer.Flush();
    }
    template <typename Sink, typename... Args>
    void FormatToSink(Sink&& sink, FormatString<Args...> format,
                      Args&&... args)
    {
        VFormatToSink(sink, format.View(), MakeFormatArgs(args...));
    }

    PM_NODISCARD inline String VFormat(StringView format, FormatArgs args)
    {
        String result;
        VFormatToSink([&result](const char* data, usize size)
                      { result += StringView(data, size); },
                      format, args);
        return result;
    }
    template <typename... Args>
    PM_NODISCARD inline String Format(FormatString<Args...> format,
                                      Args&&... args)
    {
        return VFormat(format.View(), MakeFormatArgs(args...));
    }

    template <typename Container>
    concept FormatAppendable
        = !IsBaseOfV<FormatBuffer, Container>
       && (requires(Container& c, const char* s, usize n) { c.Append(s, n); }
           || requires(Container& c, StringView s) { c += s; }
           || requires(Container& c, char ch) { c.PushBack(ch); });

    template <typename It>
    concept FormatOutputIterator
        = !FormatAppendable<It> && requires(It it, char c) { *it++ = c; };

    template <typename OutputIt>
        requires FormatOutputIterator<OutputIt>
    OutputIt VFormatTo(OutputIt out, StringView format, FormatArgs args)
    {
        if constexpr (IsSameV<OutputIt, FormatAppender>)
        {
            VFormatTo(out.Buffer(), format, args);
            return out;
        }
        else if constexpr (IsSameV<OutputIt, char*>)
        {
            VFormatToSink(
                [&out](const char* data, usize size)
                {
                    __builtin_memcpy(out, data, size);
                    out += size;
                },
                format, args);
            return out;
        }
        else
        {
            VFormatToSink(
                [&out](const char* data, usize size)
                {
                    for (usize i = 0; i < size; ++i) *out++ = data[i];
                },
                format, args);
            return out;
        }
    }
    /// Appends to a container (String, StringBuilder, anything with
    /// Append(const char*, usize), operator+=(StringView) or PushBack(char)).
    template <typename Container>
        requires FormatAppendable<Container>
    Container& VFormatTo(Container& out, StringView format, FormatArgs args)
    {
        VFormatToSink(
            [&out](const char* data, usize size)
            {
                if constexpr (requires { out.Append(data, size); })
                    out.Append(data, size);
                else if constexpr (requires { out += StringView(data, size); })
                    out += StringView(data, size);
                else
                    for (usize i = 0; i < size; ++i) out.PushBack(data[i]);
            },
            format, args);
        return out;
    }

    template <typename OutputIt, typename... Args>
        requires FormatOutputIterator<OutputIt>
    OutputIt FormatTo(OutputIt out, FormatString<Args...> format,
                      Args&&... args)
    {
        return VFormatTo(out, format.View(), MakeFormatArgs(args...));
    }
    template <typename Container, typename... Args>
        requires FormatAppendable<Container>
    Container& FormatTo(Container& out, FormatString<Args...> format,
                        Args&&... args)
    {
        return VFormatTo(out, format.View(), MakeFormatArgs(args...));
    }

    template <typename OutputIt>
    struct FormatToNResult
    {
        /// Iterator past the last character written.
        OutputIt Out;
        /// Size of the full (untruncated) output.
        usize    Size;
    };

    template <typename OutputIt>
        requires FormatOutputIterator<OutputIt>
    FormatToNResult<OutputIt> VFormatToN(OutputIt out, usize n,
                                         StringView format, FormatArgs args)
    {
        usize total = 0;
        VFormatToSink(
            [&](const char* data, usize size)
            {
                total += size;
                usize count = size < n ? size : n;
                if constexpr (IsSameV<OutputIt, char*>)
                {
                    __builtin_memcpy(out, data, count);
                    out += count;
                }
                else
                    for (usize i = 0; i < count; ++i) *out++ = data[i];
                n -= count;
            },
            format, args);
        return {out, total};
    }
    /// Writes at most `n` characters; Size reports the full length.
    template <typename OutputIt, typename... Args>
        requires FormatOutputIterator<OutputIt>
    FormatToNResult<OutputIt> FormatToN(OutputIt out, usize n,
                                        FormatString<Args...> format,
                                        Args&&... args)
    {
        return VFormatToN(out, n, format.View(), MakeFormatArgs(args...));
    }

    template <typename... Args>
    PM_NODISCARD usize FormattedSize(FormatString<Args...> format,
                                     Args&&... args)
    {
        usize total = 0;
        VFormatToSink([&total](const char*, usize size) { total += size; },
                      format.View(), MakeFormatArgs(args...));
        return total;
    }

    /// Formats straight to Log::Print, in chunks, without allocating.
    inline void VPrint(StringView format, FormatArgs args)
    {
        VFormatToSink([](const char* data, usize size)
                      { Log::Print(StringView(data, size)); },
                      format, args);
    }
    template <typename... Args>
    void Print(FormatString<Args...> format, Args&&... args)
    {
        VPrint(format.View(), MakeFormatArgs(args...));
    }
    template <typename... Args>
    void Println(FormatString<Args...> format, Args&&... args)
    {
        VPrint(format.View(), MakeFormatArgs(args...));
        Log::Print("\n");
    }

    // Built-in formatters

    /**
     * Formatter for the built-in types: integers, bool, char, C strings,
     * String/StringView and pointers. Derive from it to reuse the standard
     * format specification for your type, e.g.
     * `struct Formatter<Foo> : Formatter<StringView>`.
     */
    template <typename T>
        requires(Details::IsNativeFormatTypeV<T>)
    struct Formatter<T, char>
    {
        constexpr const char* Parse(FormatParseContext& ctx)
        {
            return ParseFormatSpecs(ctx.begin(), ctx.end(), m_Specs, ctx,
                                    Details::NativeArgType<T>());
        }
        FormatAppender Format(const T& value, FormatContext& ctx) const
        {
            Details::WriteArg(ctx.Buffer(), Details::MakeNativeArg<T>(value),
                              Details::ResolveSpecs(m_Specs, ctx));
            return ctx.Out();
        }

        DynamicFormatSpecs m_Specs{};
    };

    /// Prism::Pointer prints as hex with a 0x prefix unless told otherwise.
    template <>
    struct Formatter<Pointer, char> : Formatter<u64, char>
    {
        constexpr const char* Parse(FormatParseContext& ctx)
        {
            auto it = Formatter<u64, char>::Parse(ctx);
            if (m_Specs.Type == FormatPresentation::eNone)
            {
                m_Specs.Type = FormatPresentation::eHexLower;
                m_Specs.Alt  = true;
            }
            return it;
        }
        FormatAppender Format(const Pointer& value, FormatContext& ctx) const
        {
            return Formatter<u64, char>::Format(value.Raw<u64>(), ctx);
        }
    };

    namespace Details
    {
        /// Formats T through FormatAs(T) (fmt's format_as).
        template <typename T>
        struct FormatAsFormatter
        {
            using ResultType
                = FormatKeyType<decltype(FormatAs(DeclVal<const T&>()))>;
            FormatterFor<ResultType> Inner{};

            constexpr const char*    Parse(FormatParseContext& ctx)
            {
                return ParseWith(Inner, ctx);
            }
            void Format(const T& value, FormatContext& ctx) const
            {
                auto inner = Inner;
                inner.Format(FormatAs(value), ctx);
            }
        };

#if PRISM_DISABLE_FMT == 0
        /// Transition aid: formats types that only have an fmt::formatter by
        /// forwarding the replacement field's spec to fmt. Dynamic
        /// width/precision referring to other arguments is not supported
        /// here.
        template <typename T>
        struct FmtBridgeFormatter
        {
            static constexpr usize Capacity       = 64;
            char                   Spec[Capacity] = {};
            usize                  Size           = 0;

            constexpr const char*  Parse(FormatParseContext& ctx)
            {
                auto  it    = ctx.begin();
                usize depth = 0;
                Spec[Size++] = '{';
                Spec[Size++] = ':';
                for (; it != ctx.end(); ++it)
                {
                    if (*it == '{') ++depth;
                    else if (*it == '}')
                    {
                        if (depth == 0) break;
                        --depth;
                    }
                    if (Size < Capacity - 1) Spec[Size++] = *it;
                }
                Spec[Size++] = '}';
                return it;
            }
            void Format(const T& value, FormatContext& ctx) const
            {
                auto str
                    = fmt::format(fmt::runtime(fmt::string_view(Spec, Size)), value);
                ctx.Buffer().Append(str.data(), str.size());
            }
        };
#endif
    } // namespace Details

    // Helpers

    /// fmt::ptr: formats any pointer as an address.
    template <typename T>
    constexpr const void* Ptr(T* pointer)
    {
        return reinterpret_cast<const void*>(
            reinterpret_cast<__UINTPTR_TYPE__>(pointer));
    }

    /// fmt::underlying: formats an enum as its underlying integer.
    template <typename E>
        requires(IsEnumV<E>)
    constexpr auto Underlying(E value)
    {
        return ToUnderlying(value);
    }

    /// fmt::group_digits: decimal with ',' thousands separators.
    template <typename T>
        requires(Details::IsFormatIntegerV<T>)
    struct GroupDigitsView
    {
        T Value;
    };
    template <typename T>
        requires(Details::IsFormatIntegerV<T>)
    constexpr GroupDigitsView<T> GroupDigits(T value)
    {
        return {value};
    }
    template <typename T>
    struct Formatter<GroupDigitsView<T>, char> : Formatter<T, char>
    {
        FormatAppender Format(GroupDigitsView<T> view, FormatContext& ctx) const
        {
            auto specs      = Details::ResolveSpecs(this->m_Specs, ctx);
            specs.Localized = true;
            Details::WriteArg(ctx.Buffer(), Details::MakeNativeArg<T>(view.Value),
                              specs);
            return ctx.Out();
        }
    };

    /// fmt::join: formats the elements of a range separated by `separator`;
    /// the format spec applies to every element (`{:02x}`).
    template <typename It, typename Sentinel = It>
    struct JoinView
    {
        It         Begin;
        Sentinel   End;
        StringView Separator;
    };
    template <typename It, typename Sentinel>
    constexpr JoinView<It, Sentinel> Join(It begin, Sentinel end,
                                          StringView separator)
    {
        return {begin, end, separator};
    }
    template <typename Range>
    constexpr auto Join(Range&& range, StringView separator)
    {
        if constexpr (IsArrayV<RemoveReferenceType<Range>>)
            return Join(range + 0, range + sizeof(range) / sizeof(range[0]),
                        separator);
        else return Join(range.begin(), range.end(), separator);
    }
    template <typename It, typename Sentinel>
    struct Formatter<JoinView<It, Sentinel>, char>
    {
        using ElementType
            = Details::FormatKeyType<decltype(*DeclVal<It&>())>;
        Details::FormatterFor<ElementType> Element{};

        constexpr const char*              Parse(FormatParseContext& ctx)
        {
            return Details::ParseWith(Element, ctx);
        }
        FormatAppender Format(const JoinView<It, Sentinel>& view,
                              FormatContext&                ctx) const
        {
            auto element = Element;
            bool first   = true;
            for (auto it = view.Begin; it != view.End; ++it)
            {
                if (!first) ctx.Buffer().Append(view.Separator);
                first = false;
                element.Format(*it, ctx);
            }
            return ctx.Out();
        }
    };

    // Numbers and enums keep StringUtils' conversions (a base, magic_enum
    // names); both are exported unqualified, so an unconstrained overload
    // here made every ToString(enum) or ToString(int) ambiguous wherever
    // Prism is a using-directive or an associated namespace.
    using StringUtils::ToString;

    /// fmt::to_string, for everything StringUtils::ToString doesn't take.
    template <typename T>
        requires(!ArithmeticType<T> && !EnumType<T>)
    PM_NODISCARD String ToString(const T& value)
    {
        return Format("{}", value);
    }
}; // namespace Prism

#include <Prism/String/FormatStyle.hpp>

#if PRISM_USE_NAMESPACE != 0
using Prism::Format;
using Prism::FormattedSize;
using Prism::Formatter;
using Prism::FormatTo;
using Prism::FormatToN;
#endif
