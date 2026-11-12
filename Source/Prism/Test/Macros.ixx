/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>
#include <Prism/Test/Macros.hpp>

export module Prism.Test.Macros;

export import Prism.String.Formatter;
export import Prism.Test.CrashTest;
export import Prism.Test.Randomized.RandSource;
export import Prism.Test.TestResult;

export extern "C++"
{
    namespace Prism::Test
    {
        /// Writes to the test output (stderr).
        void WriteOutput(StringView text);
        /// Whether the test output is a terminal (enables colors).
        bool OutputIsTerminal();

        template <typename... Args>
        void Warn(FormatString<Args...> format, Args&&... args)
        {
            FormatToSink([](const char* data, usize size)
                         { WriteOutput(StringView(data, size)); }, format,
                         Forward<Args>(args)...);
            WriteOutput("\n");
        }

        namespace Details
        {
            /// "FAIL" in bold red when printing to a terminal.
            StringView FailTag();

            String     FormatDouble(double value);

            template <typename T>
            constexpr bool IsCharPointerV = ::Prism::Details::IsFormatCStringV<
                ::Prism::Details::FormatKeyType<T>>;

            /// Printable form of any value: uses Prism::Formatter when
            /// available, falls back to the underlying value for enums and to
            /// "<unprintable>" otherwise (like Serenity's FormatIfSupported).
            template <typename T>
            String ToPrintable(const T& value)
            {
                using U = RemoveCvRefType<T>;
                if constexpr (IsFloatingPointV<U>)
                    return FormatDouble(static_cast<double>(value));
                else if constexpr (Formattable<U>) return Format("{}", value);
                else if constexpr (IsEnumV<U>)
                    return Format("{}", ToUnderlying(value));
                else if constexpr (IsArrayV<U>
                                   || requires { value.begin() != value.end(); })
                {
                    // Arrays and ranges print as [a, b, c].
                    String result = "[";
                    bool   first  = true;
                    for (const auto& element : value)
                    {
                        if (!first) result += StringView(", ");
                        first = false;
                        result += ToPrintable(element);
                    }
                    result += StringView("]");
                    return result;
                }
                else return "<unprintable>";
            }

            /// `lhs == rhs`, except that C strings compare by content and
            /// integers of different signedness compare by value.
            template <typename L, typename R>
            constexpr bool Equals(const L& lhs, const R& rhs)
            {
                if constexpr (IsCharPointerV<L> && IsCharPointerV<R>)
                {
                    const char* a = lhs;
                    const char* b = rhs;
                    if (!a || !b) return a == b;
                    for (; *a && *a == *b; ++a, ++b);
                    return *a == *b;
                }
                else if constexpr (IsIntegralV<L> && IsIntegralV<R>
                                   && !IsSameV<L, bool> && !IsSameV<R, bool>
                                   && IsSignedV<L> != IsSignedV<R>)
                {
                    if constexpr (IsSignedV<L>)
                        return lhs >= 0
                            && static_cast<u64>(lhs) == static_cast<u64>(rhs);
                    else
                        return rhs >= 0
                            && static_cast<u64>(lhs) == static_cast<u64>(rhs);
                }
                else return lhs == rhs;
            }

            template <typename L, typename R>
            void ReportComparisonFailure(const char* macro, const char* lhsText,
                                         const char* rhsText, const L& lhs,
                                         const R& rhs, const char* file, u32 line)
            {
                if (IsReportingEnabled())
                    Warn("{}: {}:{}: {}({}, {}) failed with lhs={} and rhs={}",
                         FailTag(), file, line, macro, lhsText, rhsText,
                         ToPrintable(lhs), ToPrintable(rhs));
                CurrentTestCaseDidFail();
            }

            template <typename L, typename R>
            void ReportTruthFailure(const char* lhsText, const char* rhsText,
                                    const L& lhs, const R& rhs, const char* file,
                                    u32 line)
            {
                if (IsReportingEnabled())
                    Warn(
                        "{}: {}:{}: EXPECT_EQ_TRUTH({}, {}) failed with lhs={} "
                        "({}) and rhs={} ({})",
                        FailTag(), file, line, lhsText, rhsText, ToPrintable(lhs),
                        static_cast<bool>(lhs), ToPrintable(rhs),
                        static_cast<bool>(rhs));
                CurrentTestCaseDidFail();
            }

            inline void ReportFailure(StringView message, const char* file,
                                      u32 line)
            {
                if (IsReportingEnabled())
                    Warn("{}: {}:{}: {}", FailTag(), file, line, message);
                CurrentTestCaseDidFail();
            }

            void ReportApproximateFailure(const char* lhsText, const char* rhsText,
                                          double lhs, double rhs, const char* file,
                                          u32 line);

            template <typename T>
            void ReportGenerated(const char* name, const T& value)
            {
                if (IsReportingEnabled()) Warn("{} = {}", name, ToPrintable(value));
            }

            /// Error text of an Expected-like value for TRY_OR_FAIL()/MUST().
            template <typename R>
            String DescribeError(const R& result)
            {
                if constexpr (requires { result.Error(); })
                    return ToPrintable(result.Error());
                else return "empty value";
            }

            template <typename T>
            T&& Identity(T&& value)
            { return Forward<T>(value); }
        } // namespace Details
    } // namespace Prism::Test
}
