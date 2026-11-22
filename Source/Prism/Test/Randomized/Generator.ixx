/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>
export module Prism.Test.Randomized.Generator;

export import Prism.Core.Limits;
export import Prism.Test.Randomized.RandSource;
export import Prism.Containers.Vector;

/**
 * Generators for RANDOMIZED_TEST_CASE, used through GEN():
 *
 *   RANDOMIZED_TEST_CASE(sort_is_idempotent)
 *   {
 *       GEN(vec, Gen::Vector([] { return Gen::UnsignedInt(); }));
 *       ...
 *   }
 *
 * Every generator is built on RandSource::DrawValue(), and is written so
 * that smaller random choices give simpler values: shrinking the RandomRun
 * then shrinks the counterexample (smaller numbers, shorter vectors,
 * earlier OneOf() alternatives, `false` before `true`).
 */

export extern "C++"
{
    namespace Prism::Test::Gen
    {
        /// Unsigned integer in [0, max].
        inline u64 NumberU64(u64 max) { return CurrentRandSource().DrawValue(max); }
        /// Unsigned integer in [min, max].
        inline u64 NumberU64(u64 min, u64 max)
        {
            if (min >= max) return min;
            return min + NumberU64(max - min);
        }
        /// Any u64, biased towards small values: picks a bit width first, so
        /// edge cases like 0, 1 and small numbers come up often.
        inline u64 NumberU64()
        {
            u64 bits = NumberU64(64);
            if (bits == 0) return 0;
            u64 max = bits == 64 ? ~0ull : (1ull << bits) - 1;
            return NumberU64(max);
        }

        /// Unsigned 32-bit integer in [0, max].
        inline u32 UnsignedInt(u32 max) { return static_cast<u32>(NumberU64(max)); }
        /// Unsigned 32-bit integer in [min, max].
        inline u32 UnsignedInt(u32 min, u32 max)
        {
            return static_cast<u32>(NumberU64(min, max));
        }
        /// Any u32, biased towards small values (see NumberU64()).
        inline u32 UnsignedInt()
        {
            u64 bits = NumberU64(32);
            if (bits == 0) return 0;
            u64 max = (1ull << bits) - 1;
            return static_cast<u32>(NumberU64(max));
        }

        /// Signed integer in [min, max]; shrinks towards the value closest to 0.
        inline i64 NumberI64(i64 min, i64 max)
        {
            if (min >= max) return min;
            if (min >= 0) return static_cast<i64>(NumberU64(u64(min), u64(max)));
            if (max <= 0) return -static_cast<i64>(NumberU64(u64(-max), u64(-min)));

            // Spans zero: draw a magnitude and a sign, both shrinking to zero.
            bool negative = NumberU64(1) == 1;
            u64  limit    = negative ? 0 - static_cast<u64>(min) : u64(max);
            u64  value    = NumberU64(limit);
            return negative ? static_cast<i64>(0 - value) : static_cast<i64>(value);
        }
        inline i64 NumberI64()
        {
            bool negative = NumberU64(1) == 1;
            u64  value    = NumberU64() >> 1;
            return negative ? -static_cast<i64>(value) - 1 : static_cast<i64>(value);
        }

        inline bool Boolean() { return NumberU64(1) == 1; }
        /// `true` with the given probability (shrinks to `false`).
        inline bool WeightedBoolean(double probability)
        {
            if (probability <= 0) return false;
            if (probability >= 1) return NumberU64(1, 1) == 1;
            auto& source = CurrentRandSource();
            return source.DrawValue(1,
                                    [&]
                                    {
                                        return source.LiveRandom().UniformDouble()
                                                       < probability
                                                 ? 1ull
                                                 : 0ull;
                                    })
                == 1;
        }

        /// Double in [0, 1], shrinking towards 0.
        inline double Percentage()
        {
            constexpr u64 Resolution = 1ull << 53;
            return static_cast<double>(NumberU64(Resolution))
                 / static_cast<double>(Resolution);
        }
        /// Double in [min, max], shrinking towards min.
        inline double NumberF64(double min, double max)
        {
            return min + Percentage() * (max - min);
        }

        /// Printable ASCII character, shrinking towards 'a'.
        inline char Character()
        {
            constexpr const char* Alphabet
                = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 "
                  "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
            constexpr usize Size = sizeof(
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 "
                "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~")
                                 - 1;
            return Alphabet[NumberU64(Size - 1)];
        }

        /// Vector of [min, max] elements produced by `generator()`. The length
        /// is decided element by element, so shrinking can drop elements.
        template <typename Fn>
        auto Vector(usize min, usize max, Fn&& generator)
        {
            using T = decltype(generator());
            Prism::Vector<T> result;

            // Continue with a probability that gives roughly the average length.
            double average = min + (max - min) / 2.0;
            if (average > min + 16) average = min + 16;
            double keepGoing = 1.0 - 1.0 / (1.0 + (average - min));

            for (usize i = 0; i < max; ++i)
            {
                if (i >= min && !WeightedBoolean(keepGoing)) break;
                if (CurrentTestResult() == TestResult::eOverrun) break;
                result.PushBack(generator());
            }
            return result;
        }
        /// Vector of exactly `length` elements.
        template <typename Fn>
        auto Vector(usize length, Fn&& generator)
        {
            return Vector(length, length, Forward<Fn>(generator));
        }
        /// Vector of 0 to 32 elements.
        template <typename Fn>
        auto Vector(Fn&& generator)
        {
            return Vector(0, 32, Forward<Fn>(generator));
        }

        /// Random string of printable characters, [min, max] long.
        template <typename StringType>
        StringType String(usize min = 0, usize max = 32)
        {
            auto       chars = Vector(min, max, [] { return Character(); });
            StringType result;
            for (auto c : chars) result += c;
            return result;
        }

        /// One of the given values (shrinks towards the first one).
        template <typename T, typename... Ts>
        T OneOf(T first, Ts... rest)
        {
            T     values[] = {first, static_cast<T>(rest)...};
            usize index    = NumberU64(sizeof...(Ts));
            return values[index];
        }

        template <typename Fn>
        struct FrequencyOption
        {
            u64 Weight;
            Fn  Generator;
        };
        template <typename Fn>
        FrequencyOption(u64, Fn) -> FrequencyOption<Fn>;

        /// Picks one of the generators with probability proportional to its
        /// weight and returns its result (shrinks towards the first option):
        ///
        ///   Gen::Frequency(Gen::FrequencyOption{9, [] { return 0; }},
        ///                  Gen::FrequencyOption{1, [] { return Gen::UnsignedInt(); }})
        template <typename Fn, typename... Fns>
        auto Frequency(FrequencyOption<Fn> first, FrequencyOption<Fns>... rest)
        {
            u64 total = first.Weight + (0 + ... + rest.Weight);
            u64 pick  = total > 0 ? NumberU64(total - 1) : 0;

            using T   = decltype(first.Generator());
            if (pick < first.Weight) return first.Generator();
            pick -= first.Weight;

            T    result{};
            bool done = false;
            (
                [&]
                {
                    if (done) return;
                    if (pick < rest.Weight)
                    {
                        result = rest.Generator();
                        done   = true;
                        return;
                    }
                    pick -= rest.Weight;
                }(),
                ...);
            return result;
        }
    } // namespace Prism::Test::Gen
}
