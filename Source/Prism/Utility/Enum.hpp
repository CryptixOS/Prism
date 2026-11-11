/*
 * Created by v1tr10l7 on 11.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Concepts.hpp>
#include <Prism/Core/Core.hpp>
#include <Prism/Core/Limits.hpp>
#include <Prism/Core/TypeTraits.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/String/StringView.hpp>

/**
 * Compile-time enumerator names, without magic_enum.
 *
 * Each value in an enum's range is instantiated as a template argument and
 * the compiler's __PRETTY_FUNCTION__ tells named enumerators ("Color::eRed")
 * from plain values ("(Color)5"). Only the named ones are kept, so the
 * runtime table is as large as the enum, not as its range.
 *
 * Range: enums with a 1-byte underlying type are scanned whole; others use
 * [PRISM_ENUM_RANGE_MIN, PRISM_ENUM_RANGE_MAX]. Either is clamped to the
 * underlying type. Specialize EnumRange<E> to override it for one enum:
 *
 *     template <>
 *     struct Prism::EnumRange<MyEnum>
 *     {
 *         static constexpr i64 Min = 0;
 *         static constexpr i64 Max = 0x1000;
 *     };
 *
 * Values outside the range have no name. Enumerators sharing a value report
 * whichever name the compiler prints (normally the first).
 *
 * Enums without a fixed underlying type (C enums) can only hold the values
 * that fit the bits their enumerators need; clang rejects a constant cast to
 * anything else. Such values are detected and skipped, so C enums work with
 * the default range too.
 */

#ifndef PRISM_ENUM_RANGE_MIN
    #define PRISM_ENUM_RANGE_MIN -128
#endif
#ifndef PRISM_ENUM_RANGE_MAX
    #define PRISM_ENUM_RANGE_MAX 127
#endif

namespace Prism
{
    /// Specialize to change the range of values scanned for one enum.
    template <EnumType E>
    struct EnumRange
    {
      private:
        using Underlying = UnderlyingTypeType<E>;
        static constexpr bool IsByte = sizeof(Underlying) == 1;

      public:
        static constexpr i64 Min
            = IsByte ? static_cast<i64>(NumericLimits<Underlying>::Min())
                     : PRISM_ENUM_RANGE_MIN;
        static constexpr i64 Max
            = IsByte ? static_cast<i64>(NumericLimits<Underlying>::Max())
                     : PRISM_ENUM_RANGE_MAX;
    };

    namespace Details
    {
        struct EnumNameSpan
        {
            const char* Data = nullptr;
            usize       Size = 0;
        };

        constexpr bool IsEnumIdentifierChar(char c)
        {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
                || (c >= '0' && c <= '9') || c == '_';
        }

        /// The trailing identifier of `prettyFunction`'s template argument,
        /// or an empty span if the value isn't a named enumerator.
        constexpr EnumNameSpan ParseEnumName(const char* prettyFunction,
                                             usize       size)
        {
            // GCC:   "... [with auto V = Color::eRed]"
            // Clang: "... [V = Color::eRed]"
            // Either, for a value without a name: "... = (Color)5]"
            usize end = size;
            while (end > 0 && prettyFunction[end - 1] != ']') --end;
            if (end == 0) return {};
            --end;

            usize start = end;
            while (start > 0 && IsEnumIdentifierChar(prettyFunction[start - 1]))
                --start;

            if (start == end) return {};
            char first = prettyFunction[start];
            if (first >= '0' && first <= '9') return {};

            return {prettyFunction + start, end - start};
        }

        template <auto V>
        constexpr EnumNameSpan RawEnumName()
        {
            return ParseEnumName(__PRETTY_FUNCTION__,
                                 sizeof(__PRETTY_FUNCTION__) - 1);
        }

        template <auto V>
        struct EnumConstantProbe
        {
        };
        /// Whether E can hold N. False only for enums without a fixed
        /// underlying type, outside the range their enumerators span (the
        /// cast is then not a constant expression, so the probe fails).
        template <typename E, i64 N>
        concept EnumRepresentable = requires {
            typename EnumConstantProbe<static_cast<E>(
                static_cast<UnderlyingTypeType<E>>(N))>;
        };

        /// Only unscoped enums can lack a fixed underlying type; scoped ones
        /// skip the probe, which matters for wide custom ranges.
        template <typename E>
        concept UnscopedEnum = requires(E e) { static_cast<void>(+e); };

        template <EnumType E>
        struct EnumTable
        {
            using Underlying = UnderlyingTypeType<E>;

            static constexpr i64 ClampedMin()
            {
                constexpr i64 typeMin
                    = static_cast<i64>(NumericLimits<Underlying>::Min());
                return EnumRange<E>::Min < typeMin ? typeMin
                                                   : EnumRange<E>::Min;
            }
            static constexpr i64 ClampedMax()
            {
                // u64 values above I64 max aren't scannable anyway.
                constexpr i64 typeMax
                    = sizeof(Underlying) >= sizeof(i64)
                        ? NumericLimits<i64>::Max()
                        : static_cast<i64>(NumericLimits<Underlying>::Max());
                return EnumRange<E>::Max > typeMax ? typeMax
                                                   : EnumRange<E>::Max;
            }

            static constexpr i64 Min = ClampedMin();
            static constexpr i64 Max = ClampedMax();
            static_assert(Min <= Max, "EnumRange<E>: Min must be <= Max");
            static_assert(Max - Min < 0x10000,
                          "EnumRange<E>: range is limited to 65536 values");

            static constexpr usize RangeSize = static_cast<usize>(Max - Min + 1);

            static constexpr E     ValueAt(usize i)
            {
                return static_cast<E>(
                    static_cast<Underlying>(Min + static_cast<i64>(i)));
            }

            /// The one per-value instantiation: the name of Min + I, or an
            /// empty span. Everything after works on the resulting array.
            template <usize I>
            static constexpr EnumNameSpan SpanAt()
            {
                constexpr i64 n = Min + static_cast<i64>(I);
                if constexpr (UnscopedEnum<E> && !EnumRepresentable<E, n>)
                    return {};
                else
                    return RawEnumName<static_cast<E>(
                        static_cast<Underlying>(n))>();
            }

            struct SpanArray
            {
                EnumNameSpan Items[RangeSize];
            };
            static constexpr SpanArray Spans
                = []<usize... I>(IndexSequence<I...>)
            {
                return SpanArray{{SpanAt<I>()...}};
            }(MakeIndexSequence<RangeSize>{});

            static constexpr usize Count = []
            {
                usize count = 0;
                for (usize i = 0; i < RangeSize; ++i)
                    count += Spans.Items[i].Size != 0;
                return count;
            }();
            static constexpr usize PoolSize = []
            {
                usize size = 0;
                for (usize i = 0; i < RangeSize; ++i) size += Spans.Items[i].Size;
                return size;
            }();

            /// All names back to back, so the binary keeps the names only,
            /// not every __PRETTY_FUNCTION__ string that was parsed.
            struct NamePool
            {
                char Chars[PoolSize + 1];
            };
            static constexpr NamePool Pool = []
            {
                NamePool pool{};
                usize    at = 0;
                for (usize i = 0; i < RangeSize; ++i)
                    for (usize c = 0; c < Spans.Items[i].Size; ++c)
                        pool.Chars[at++] = Spans.Items[i].Data[c];
                return pool;
            }();

            struct Entries
            {
                // At least one element, so empty enums still compile.
                E          Values[Count ? Count : 1]{};
                StringView Names[Count ? Count : 1]{};
            };

            static constexpr Entries Data = []
            {
                Entries entries{};
                usize   next = 0, at = 0;
                for (usize i = 0; i < RangeSize; ++i)
                {
                    usize size = Spans.Items[i].Size;
                    if (size == 0) continue;
                    entries.Values[next] = ValueAt(i);
                    entries.Names[next]  = StringView(Pool.Chars + at, size);
                    at += size;
                    ++next;
                }
                return entries;
            }();

            /// Index into Data, or Count if `value` has no name.
            static constexpr usize Find(E value)
            {
                // Values are in ascending order: they were scanned that way.
                auto  key = static_cast<Underlying>(value);
                usize lo = 0, hi = Count;
                while (lo < hi)
                {
                    usize mid = lo + (hi - lo) / 2;
                    auto  at  = static_cast<Underlying>(Data.Values[mid]);
                    if (at == key) return mid;
                    if (at < key) lo = mid + 1;
                    else hi = mid;
                }
                return Count;
            }
        };
    } // namespace Details

    /// The enumerator's name ("eRed" for Color::eRed), or an empty view if
    /// `value` isn't a named enumerator within EnumRange<E>.
    template <EnumType E>
    PM_NODISCARD constexpr StringView EnumName(E value) PM_NOEXCEPT
    {
        using Table = Details::EnumTable<E>;
        usize index = Table::Find(value);
        return index == Table::Count ? StringView() : Table::Data.Names[index];
    }

    /// Whether `value` is a named enumerator within EnumRange<E>.
    template <EnumType E>
    PM_NODISCARD constexpr bool EnumContains(E value) PM_NOEXCEPT
    {
        using Table = Details::EnumTable<E>;
        return Table::Find(value) != Table::Count;
    }

    /// Number of named enumerators within EnumRange<E>.
    template <EnumType E>
    inline constexpr usize EnumCount = Details::EnumTable<E>::Count;
}; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::EnumContains;
using Prism::EnumCount;
using Prism::EnumName;
using Prism::EnumRange;
#endif
