/*
 * Created by v1tr10l7 on 15.07.2025.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>
export module Prism.String.FormatString;

export import Prism.String.FormatHandler;
export import Prism.String.String;
export import Prism.String.StringView;

export extern "C++"
{
    namespace Prism
    {
        namespace Details
        {
            // Named arguments
            template <typename T>
            struct NamedArgTraits
            {
                static constexpr bool IsNamed  = false;
                static constexpr bool IsStatic = false;
                using ValueType                = T;
            };
            template <typename T>
            struct NamedArgTraits<NamedArg<T>>
            {
                static constexpr bool IsNamed  = true;
                static constexpr bool IsStatic = false;
                using ValueType                = T;
            };
            template <FixedFormatString Name, typename T>
            struct NamedArgTraits<StaticNamedArg<Name, T>>
            {
                static constexpr bool        IsNamed  = true;
                static constexpr bool        IsStatic = true;
                static constexpr const char* Name_    = Name.Data;
                using ValueType                       = T;
            };

            /// Character arrays (string literals) are C strings; everything else
            /// is looked at without cv-ref qualifiers.
            template <typename T>
            inline constexpr bool IsCharArrayV
                = IsArrayV<RemoveCvRefType<T>>
               && IsSameV<RemoveCvType<RemoveExtentType<RemoveCvRefType<T>>>, char>;

            template <typename T>
            using FormatKeyType
                = ConditionalType<IsCharArrayV<T>, const char*, RemoveCvRefType<T>>;

            template <typename T>
            using FormatValueType = FormatKeyType<
                typename NamedArgTraits<RemoveCvRefType<T>>::ValueType>;

            template <typename T>
            inline constexpr bool IsCharLikeV
                = IsSameV<T, wchar_t> || IsSameV<T, char8_t>
               || IsSameV<T, char16_t> || IsSameV<T, char32_t>;

            template <typename T>
            inline constexpr bool IsFormatIntegerV
                = IsIntegralV<T> && !IsSameV<T, bool> && !IsSameV<T, char>
               && !IsCharLikeV<T> && sizeof(T) <= 8;

            template <typename T>
            inline constexpr bool IsFormatCStringV
                = IsSameV<T, const char*> || IsSameV<T, char*>;

            template <typename T>
            inline constexpr bool IsFormatStringV = IsSameV<T, BasicStringView<char>>
                                          || IsSameV<T, BasicString<char>>;

            template <typename T>
            inline constexpr bool IsFormatPointerV
                = IsNullPointerV<T>
               || (IsPointerV<T> && !IsFormatCStringV<T>
                   && !IsFunctionV<RemovePointerType<T>>);

            /// Argument type for the built-in (non-custom) types.
            template <typename T>
            consteval FormatArgType NativeArgType()
            {
                if constexpr (IsSameV<T, bool>) return FormatArgType::eBool;
                else if constexpr (IsSameV<T, char>) return FormatArgType::eChar;
                else if constexpr (IsFormatIntegerV<T>)
                {
                    if constexpr (IsSignedV<T>) return FormatArgType::eInt;
                    else return FormatArgType::eUInt;
                }
                else if constexpr (IsFormatCStringV<T>)
                    return FormatArgType::eCString;
                else if constexpr (IsFormatStringV<T>)
                    return FormatArgType::eString;
                else if constexpr (IsFormatPointerV<T>)
                    return FormatArgType::ePointer;
                else return FormatArgType::eNone;
            }
            template <typename T>
            inline constexpr bool IsNativeFormatTypeV
                = NativeArgType<T>() != FormatArgType::eNone;

            template <typename T>
            concept HasPrismFormatter = requires { Formatter<T, char>{}; };
            template <typename T>
            concept HasFormatAs
                = requires(const T& value) { FormatAs(value); };
            template <typename T>
            struct FormatAsFormatter;

            template <typename T>
            struct UnformattableType
            {
                static_assert(sizeof(T) == 0,
                              "Prism: this type is not formattable; specialize "
                              "Prism::Formatter<T> or provide FormatAs(const T&)");

                // Stubs so the static_assert above is the only diagnostic.
                constexpr const char* Parse(FormatParseContext& ctx)
                {
                    return ctx.begin();
                }
                template <typename V>
                void Format(const V&, FormatContext&) const
                {
                }
            };

            template <typename T>
            consteval auto SelectFormatter()
            {
                if constexpr (HasPrismFormatter<T>) return TypeIdentity<Formatter<T, char>>{};
                else if constexpr (HasFormatAs<T>)
                    return TypeIdentity<FormatAsFormatter<T>>{};
                else return TypeIdentity<UnformattableType<T>>{};
            }

            /// The formatter used for T: Formatter<T>, then FormatAs().
            template <typename T>
            using FormatterFor = typename decltype(SelectFormatter<T>())::Type;

            template <typename T>
            inline constexpr FormatArgType ArgTypeOf
                = IsNativeFormatTypeV<T> ? NativeArgType<T>()
                                         : FormatArgType::eCustom;

            /// Runs the formatter's Parse(), if it has one.
            template <typename F>
            constexpr const char* ParseWith(F& formatter, FormatParseContext& ctx)
            {
                if constexpr (requires { formatter.Parse(ctx); })
                    return formatter.Parse(ctx);
                else return ctx.begin();
            }

            template <typename T>
            constexpr const char* CheckArgSpecs(FormatParseContext& ctx)
            {
                FormatterFor<T> formatter{};
                return ParseWith(formatter, ctx);
            }

            template <typename... Args>
            consteval usize CountNamedArgs()
            {
                return (0 + ... + (NamedArgTraits<RemoveCvRefType<Args>>::IsNamed ? 1 : 0));
            }
            template <typename... Args>
            consteval usize CountStaticNamedArgs()
            {
                return (0 + ...
                        + (NamedArgTraits<RemoveCvRefType<Args>>::IsStatic ? 1 : 0));
            }

            template <typename... Args>
            constexpr isize StaticArgIndexByName(StringView name)
            {
                isize index  = 0;
                isize result = -1;
                (void)name;
                (
                    [&]
                    {
                        using Traits = NamedArgTraits<RemoveCvRefType<Args>>;
                        if constexpr (Traits::IsStatic)
                        {
                            if (result < 0
                                && NameEquals(Traits::Name_, name.Raw(),
                                              name.Size()))
                                result = index;
                        }
                        ++index;
                    }(),
                    ...);
                (void)index;
                return result;
            }

            /// Compile-time format string validation.
            template <typename... Args>
            class FormatStringChecker
            {
                static constexpr usize ArgCount = sizeof...(Args);
                using ParseFn = const char* (*)(FormatParseContext&);

                static constexpr usize ArraySize = ArgCount > 0 ? ArgCount : 1;
                FormatArgType          m_Types[ArraySize]{
                    ArgTypeOf<FormatValueType<Args>>...};
                ParseFn m_ParseFns[ArraySize]{
                    &CheckArgSpecs<FormatValueType<Args>>...};
                FormatParseContext m_Context;

              public:
                constexpr explicit FormatStringChecker(StringView format)
                    : m_Context(format, ArgCount, m_Types)
                {
                }

                constexpr void  OnText(const char*, const char*) {}
                constexpr isize OnArgID() { return m_Context.NextArgID(); }
                constexpr isize OnArgID(isize id)
                {
                    m_Context.CheckArgID(id);
                    return id;
                }
                constexpr isize OnArgID(StringView name)
                {
                    isize index = StaticArgIndexByName<Args...>(name);
                    if (index < 0)
                        ReportFormatError("named argument is not found");
                    return index;
                }
                constexpr void OnReplacementField(isize id, const char* begin)
                {
                    OnFormatSpecs(id, begin, begin);
                }
                constexpr const char* OnFormatSpecs(isize id, const char* begin,
                                                    const char*)
                {
                    m_Context.AdvanceTo(begin);
                    if (id < 0 || static_cast<usize>(id) >= ArgCount)
                        return begin;
                    return m_ParseFns[id](m_Context);
                }
            };
        } // namespace Details

        /// Whether values of type T can be formatted (built-in type,
        /// Prism::Formatter specialization or FormatAs()).
        template <typename T>
        concept Formattable
            = Details::IsNativeFormatTypeV<Details::FormatKeyType<T>>
           || Details::HasPrismFormatter<Details::FormatKeyType<T>>
           || Details::HasFormatAs<Details::FormatKeyType<T>>;

        /// Wraps a format string that is only known at run time: no compile-time
        /// checking, errors skip the faulty replacement field.
        struct RuntimeFormatString
        {
            StringView String;
        };
        constexpr RuntimeFormatString Runtime(StringView format)
        {
            return {format};
        }

        /**
         * A format string checked at compile time against the argument types:
         * argument ids, braces, format specifications (via each formatter's
         * Parse()) and dynamic width/precision.
         */
        template <typename Char, typename... Args>
        class BasicFormatString
        {
          public:
            template <typename S>
                requires(IsConvertibleV<const S&, BasicStringView<Char>>)
            consteval BasicFormatString(const S& format)
                : m_String(format)
            {
                using namespace Details;
                // Like fmt: runtime named args (Arg()) cannot be resolved at
                // compile time, so only fully static argument lists are checked.
                if constexpr (CountNamedArgs<Args...>()
                              == CountStaticNamedArgs<Args...>())
                    ParseFormatString(m_String,
                                      FormatStringChecker<Args...>(m_String));
            }
            constexpr BasicFormatString(RuntimeFormatString format)
                : m_String(format.String)
            {
            }

            constexpr BasicStringView<Char> View() const { return m_String; }
            constexpr operator BasicStringView<Char>() const { return m_String; }

          private:
            BasicStringView<Char> m_String;
        };

        template <typename... Args>
        using FormatString = BasicFormatString<char, TypeIdentityType<Args>...>;
    }; // namespace Prism
}
