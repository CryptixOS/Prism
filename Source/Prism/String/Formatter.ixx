/*
 * Created by v1tr10l7 on 24.04.2025.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>
export module Prism.String.Formatter;

export import Prism.Core.TypeTraits;
export import Prism.Memory.Pointer;
export import Prism.String.FormatHandler;
export import Prism.String.FormatString;
export import Prism.String.String;
export import Prism.String.StringUtils;

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

export extern "C++"
{
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

    /**
     * Text styles (fmt/color.h): ANSI foreground/background colors and
     * emphasis.
     *
     *   Format(Fg(TextColor::eRed) | TextEmphasis::eBold, "error: {}", msg)
     *   Format("{} {}", Styled(42, Bg(TerminalColor::eBlue)), "plain")
     */
    namespace Prism
    {
        /// Named 24-bit colors (the CSS/X11 set, same values as fmt::color).
        enum class TextColor : u32
        {
            eAliceBlue            = 0xF0F8FF,
            eAntiqueWhite         = 0xFAEBD7,
            eAqua                 = 0x00FFFF,
            eAquamarine           = 0x7FFFD4,
            eAzure                = 0xF0FFFF,
            eBeige                = 0xF5F5DC,
            eBisque               = 0xFFE4C4,
            eBlack                = 0x000000,
            eBlanchedAlmond       = 0xFFEBCD,
            eBlue                 = 0x0000FF,
            eBlueViolet           = 0x8A2BE2,
            eBrown                = 0xA52A2A,
            eBurlyWood            = 0xDEB887,
            eCadetBlue            = 0x5F9EA0,
            eChartreuse           = 0x7FFF00,
            eChocolate            = 0xD2691E,
            eCoral                = 0xFF7F50,
            eCornflowerBlue       = 0x6495ED,
            eCornsilk             = 0xFFF8DC,
            eCrimson              = 0xDC143C,
            eCyan                 = 0x00FFFF,
            eDarkBlue             = 0x00008B,
            eDarkCyan             = 0x008B8B,
            eDarkGoldenRod        = 0xB8860B,
            eDarkGray             = 0xA9A9A9,
            eDarkGreen            = 0x006400,
            eDarkKhaki            = 0xBDB76B,
            eDarkMagenta          = 0x8B008B,
            eDarkOliveGreen       = 0x556B2F,
            eDarkOrange           = 0xFF8C00,
            eDarkOrchid           = 0x9932CC,
            eDarkRed              = 0x8B0000,
            eDarkSalmon           = 0xE9967A,
            eDarkSeaGreen         = 0x8FBC8F,
            eDarkSlateBlue        = 0x483D8B,
            eDarkSlateGray        = 0x2F4F4F,
            eDarkTurquoise        = 0x00CED1,
            eDarkViolet           = 0x9400D3,
            eDeepPink             = 0xFF1493,
            eDeepSkyBlue          = 0x00BFFF,
            eDimGray              = 0x696969,
            eDodgerBlue           = 0x1E90FF,
            eFireBrick            = 0xB22222,
            eFloralWhite          = 0xFFFAF0,
            eForestGreen          = 0x228B22,
            eFuchsia              = 0xFF00FF,
            eGainsboro            = 0xDCDCDC,
            eGhostWhite           = 0xF8F8FF,
            eGold                 = 0xFFD700,
            eGoldenRod            = 0xDAA520,
            eGray                 = 0x808080,
            eGreen                = 0x008000,
            eGreenYellow          = 0xADFF2F,
            eHoneyDew             = 0xF0FFF0,
            eHotPink              = 0xFF69B4,
            eIndianRed            = 0xCD5C5C,
            eIndigo               = 0x4B0082,
            eIvory                = 0xFFFFF0,
            eKhaki                = 0xF0E68C,
            eLavender             = 0xE6E6FA,
            eLavenderBlush        = 0xFFF0F5,
            eLawnGreen            = 0x7CFC00,
            eLemonChiffon         = 0xFFFACD,
            eLightBlue            = 0xADD8E6,
            eLightCoral           = 0xF08080,
            eLightCyan            = 0xE0FFFF,
            eLightGoldenRodYellow = 0xFAFAD2,
            eLightGray            = 0xD3D3D3,
            eLightGreen           = 0x90EE90,
            eLightPink            = 0xFFB6C1,
            eLightSalmon          = 0xFFA07A,
            eLightSeaGreen        = 0x20B2AA,
            eLightSkyBlue         = 0x87CEFA,
            eLightSlateGray       = 0x778899,
            eLightSteelBlue       = 0xB0C4DE,
            eLightYellow          = 0xFFFFE0,
            eLime                 = 0x00FF00,
            eLimeGreen            = 0x32CD32,
            eLinen                = 0xFAF0E6,
            eMagenta              = 0xFF00FF,
            eMaroon               = 0x800000,
            eMediumAquamarine     = 0x66CDAA,
            eMediumBlue           = 0x0000CD,
            eMediumOrchid         = 0xBA55D3,
            eMediumPurple         = 0x9370DB,
            eMediumSeaGreen       = 0x3CB371,
            eMediumSlateBlue      = 0x7B68EE,
            eMediumSpringGreen    = 0x00FA9A,
            eMediumTurquoise      = 0x48D1CC,
            eMediumVioletRed      = 0xC71585,
            eMidnightBlue         = 0x191970,
            eMintCream            = 0xF5FFFA,
            eMistyRose            = 0xFFE4E1,
            eMoccasin             = 0xFFE4B5,
            eNavajoWhite          = 0xFFDEAD,
            eNavy                 = 0x000080,
            eOldLace              = 0xFDF5E6,
            eOlive                = 0x808000,
            eOliveDrab            = 0x6B8E23,
            eOrange               = 0xFFA500,
            eOrangeRed            = 0xFF4500,
            eOrchid               = 0xDA70D6,
            ePaleGoldenRod        = 0xEEE8AA,
            ePaleGreen            = 0x98FB98,
            ePaleTurquoise        = 0xAFEEEE,
            ePaleVioletRed        = 0xDB7093,
            ePapayaWhip           = 0xFFEFD5,
            ePeachPuff            = 0xFFDAB9,
            ePeru                 = 0xCD853F,
            ePink                 = 0xFFC0CB,
            ePlum                 = 0xDDA0DD,
            ePowderBlue           = 0xB0E0E6,
            ePurple               = 0x800080,
            eRebeccaPurple        = 0x663399,
            eRed                  = 0xFF0000,
            eRosyBrown            = 0xBC8F8F,
            eRoyalBlue            = 0x4169E1,
            eSaddleBrown          = 0x8B4513,
            eSalmon               = 0xFA8072,
            eSandyBrown           = 0xF4A460,
            eSeaGreen             = 0x2E8B57,
            eSeaShell             = 0xFFF5EE,
            eSienna               = 0xA0522D,
            eSilver               = 0xC0C0C0,
            eSkyBlue              = 0x87CEEB,
            eSlateBlue            = 0x6A5ACD,
            eSlateGray            = 0x708090,
            eSnow                 = 0xFFFAFA,
            eSpringGreen          = 0x00FF7F,
            eSteelBlue            = 0x4682B4,
            eTan                  = 0xD2B48C,
            eTeal                 = 0x008080,
            eThistle              = 0xD8BFD8,
            eTomato               = 0xFF6347,
            eTurquoise            = 0x40E0D0,
            eViolet               = 0xEE82EE,
            eWheat                = 0xF5DEB3,
            eWhite                = 0xFFFFFF,
            eWhiteSmoke           = 0xF5F5F5,
            eYellow               = 0xFFFF00,
            eYellowGreen          = 0x9ACD32,
        };

        /// The 16 standard terminal colors (SGR foreground codes).
        enum class TerminalColor : u8
        {
            eBlack = 30,
            eRed,
            eGreen,
            eYellow,
            eBlue,
            eMagenta,
            eCyan,
            eWhite,
            eBrightBlack = 90,
            eBrightRed,
            eBrightGreen,
            eBrightYellow,
            eBrightBlue,
            eBrightMagenta,
            eBrightCyan,
            eBrightWhite,
        };

        enum class TextEmphasis : u8
        {
            eBold          = 1,
            eFaint         = 1 << 1,
            eItalic        = 1 << 2,
            eUnderline     = 1 << 3,
            eBlink         = 1 << 4,
            eReverse       = 1 << 5,
            eConceal       = 1 << 6,
            eStrikethrough = 1 << 7,
        };

        struct RgbColor
        {
            u8 R                 = 0;
            u8 G                 = 0;
            u8 B                 = 0;

            constexpr RgbColor() = default;
            constexpr RgbColor(u8 r, u8 g, u8 b)
                : R(r)
                , G(g)
                , B(b)
            {
            }
            constexpr RgbColor(u32 hex)
                : R((hex >> 16) & 0xff)
                , G((hex >> 8) & 0xff)
                , B(hex & 0xff)
            {
            }
            constexpr RgbColor(TextColor color)
                : RgbColor(static_cast<u32>(color))
            {
            }
        };

        /// Either a 24-bit color or one of the terminal colors.
        struct StyleColor
        {
            bool IsRgb = true;
            union
            {
                struct RgbColor Rgb;
                TerminalColor   Terminal;
            };

            constexpr StyleColor()
                : Rgb()
            {
            }
            constexpr StyleColor(struct RgbColor rgb)
                : Rgb(rgb)
            {
            }
            constexpr StyleColor(TextColor color)
                : Rgb(color)
            {
            }
            constexpr StyleColor(u32 hex)
                : Rgb(hex)
            {
            }
            constexpr StyleColor(TerminalColor color)
                : IsRgb(false)
                , Terminal(color)
            {
            }
        };

        class TextStyle
        {
          public:
            constexpr TextStyle() = default;
            constexpr TextStyle(TextEmphasis emphasis)
                : m_Emphasis(static_cast<u8>(emphasis))
            {
            }

            constexpr bool HasForeground() const { return m_HasForeground; }
            constexpr bool HasBackground() const { return m_HasBackground; }
            constexpr bool HasEmphasis() const { return m_Emphasis != 0; }
            constexpr bool Empty() const
            { return !HasForeground() && !HasBackground() && !HasEmphasis(); }

            constexpr StyleColor Foreground() const { return m_Foreground; }
            constexpr StyleColor Background() const { return m_Background; }
            constexpr u8         EmphasisMask() const { return m_Emphasis; }

            /// Combines two styles; colors set in `rhs` win.
            constexpr TextStyle& operator|=(const TextStyle& rhs)
            {
                if (rhs.m_HasForeground)
                {
                    m_Foreground    = rhs.m_Foreground;
                    m_HasForeground = true;
                }
                if (rhs.m_HasBackground)
                {
                    m_Background    = rhs.m_Background;
                    m_HasBackground = true;
                }
                m_Emphasis |= rhs.m_Emphasis;
                return *this;
            }
            friend constexpr TextStyle operator|(TextStyle        lhs,
                                                 const TextStyle& rhs)
            { return lhs |= rhs; }

            static constexpr TextStyle MakeForeground(StyleColor color)
            {
                TextStyle style;
                style.m_Foreground    = color;
                style.m_HasForeground = true;
                return style;
            }
            static constexpr TextStyle MakeBackground(StyleColor color)
            {
                TextStyle style;
                style.m_Background    = color;
                style.m_HasBackground = true;
                return style;
            }

          private:
            StyleColor m_Foreground;
            StyleColor m_Background;
            bool       m_HasForeground = false;
            bool       m_HasBackground = false;
            u8         m_Emphasis      = 0;
        };

        constexpr TextStyle operator|(TextEmphasis lhs, TextEmphasis rhs)
        { return TextStyle(lhs) | TextStyle(rhs); }

        /// Foreground color style (fmt::fg).
        constexpr TextStyle Fg(StyleColor color)
        { return TextStyle::MakeForeground(color); }
        /// Background color style (fmt::bg).
        constexpr TextStyle Bg(StyleColor color)
        { return TextStyle::MakeBackground(color); }

        namespace Details
        {
            inline void WriteDecimal(FormatBuffer& out, u32 value)
            {
                char  digits[12];
                char* end = digits + sizeof(digits);
                usize n   = FormatDigits(end, value, 10, false, false);
                out.Append(end - n, n);
            }
            /// fmt 10.2 always writes RGB components as three digits.
            inline void WriteColorComponent(FormatBuffer& out, u8 value)
            {
                out.PushBack(static_cast<char>('0' + value / 100));
                out.PushBack(static_cast<char>('0' + value / 10 % 10));
                out.PushBack(static_cast<char>('0' + value % 10));
            }
            inline void WriteStyleColor(FormatBuffer& out, StyleColor color,
                                        bool background)
            {
                out.Append("\x1b[", 2);
                if (!color.IsRgb)
                    WriteDecimal(out, static_cast<u32>(color.Terminal)
                                          + (background ? 10 : 0));
                else
                {
                    out.Append(background ? "48;2;" : "38;2;", 5);
                    WriteColorComponent(out, color.Rgb.R);
                    out.PushBack(';');
                    WriteColorComponent(out, color.Rgb.G);
                    out.PushBack(';');
                    WriteColorComponent(out, color.Rgb.B);
                }
                out.PushBack('m');
            }

            /// Emits the escape sequences for `style`; returns whether anything
            /// was written (and thus needs a reset afterwards).
            inline bool WriteStyleBegin(FormatBuffer& out, const TextStyle& style)
            {
                if (style.HasEmphasis())
                {
                    constexpr u8 codes[] = {1, 2, 3, 4, 5, 7, 8, 9};
                    for (usize i = 0; i < 8; ++i)
                    {
                        if (!(style.EmphasisMask() & (1u << i))) continue;
                        out.Append("\x1b[", 2);
                        WriteDecimal(out, codes[i]);
                        out.PushBack('m');
                    }
                }
                if (style.HasForeground())
                    WriteStyleColor(out, style.Foreground(), false);
                if (style.HasBackground())
                    WriteStyleColor(out, style.Background(), true);
                return !style.Empty();
            }
            inline void WriteStyleReset(FormatBuffer& out)
            { out.Append("\x1b[0m", 4); }
        } // namespace Details

        inline void VFormatTo(FormatBuffer& out, const TextStyle& style,
                              StringView format, FormatArgs args)
        {
            bool styled = Details::WriteStyleBegin(out, style);
            VFormatTo(out, format, args);
            if (styled) Details::WriteStyleReset(out);
        }

        template <typename Sink>
        void VFormatToSink(Sink&& sink, const TextStyle& style, StringView format,
                           FormatArgs args)
        {
            using Ref = Details::SinkRef<RemoveReferenceType<Sink>>;
            SinkFormatBuffer<Ref> buffer(Ref{&sink});
            VFormatTo(buffer, style, format, args);
            buffer.Flush();
        }

        PM_NODISCARD inline String VFormat(const TextStyle& style,
                                           StringView format, FormatArgs args)
        {
            String result;
            VFormatToSink([&result](const char* data, usize size)
                          { result += StringView(data, size); }, style, format,
                          args);
            return result;
        }
        template <typename... Args>
        PM_NODISCARD String Format(const TextStyle&      style,
                                   FormatString<Args...> format, Args&&... args)
        { return VFormat(style, format.View(), MakeFormatArgs(args...)); }

        template <typename OutputIt, typename... Args>
            requires FormatOutputIterator<OutputIt>
        OutputIt FormatTo(OutputIt out, const TextStyle& style,
                          FormatString<Args...> format, Args&&... args)
        {
            auto formatArgs = MakeFormatArgs(args...);
            if constexpr (IsSameV<OutputIt, FormatAppender>)
            {
                VFormatTo(out.Buffer(), style, format.View(), formatArgs);
                return out;
            }
            else
            {
                VFormatToSink(
                    [&out](const char* data, usize size)
                    {
                        for (usize i = 0; i < size; ++i) *out++ = data[i];
                    },
                    style, format.View(), formatArgs);
                return out;
            }
        }
        template <typename Container, typename... Args>
            requires FormatAppendable<Container>
        Container& FormatTo(Container& out, const TextStyle& style,
                            FormatString<Args...> format, Args&&... args)
        {
            String styled = Format(style, format, Forward<Args>(args)...);
            return VFormatTo(out, "{}", MakeFormatArgs(styled));
        }

        /// A value printed with a style (fmt::styled).
        template <typename T>
        struct StyledArg
        {
            const T&  Value;
            TextStyle Style;
        };
        template <typename T>
        constexpr StyledArg<T> Styled(const T& value, TextStyle style)
        { return {value, style}; }
        template <typename T>
        struct Formatter<StyledArg<T>, char>
        {
            Details::FormatterFor<Details::FormatKeyType<T>> Inner{};

            constexpr const char* Parse(FormatParseContext& ctx)
            { return Details::ParseWith(Inner, ctx); }
            FormatAppender Format(const StyledArg<T>& arg, FormatContext& ctx) const
            {
                bool styled = Details::WriteStyleBegin(ctx.Buffer(), arg.Style);
                auto inner  = Inner;
                inner.Format(arg.Value, ctx);
                if (styled) Details::WriteStyleReset(ctx.Buffer());
                return ctx.Out();
            }
        };
    }; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    using Prism::Format;
    using Prism::FormattedSize;
    using Prism::Formatter;
    using Prism::FormatTo;
    using Prism::FormatToN;
#endif
}
