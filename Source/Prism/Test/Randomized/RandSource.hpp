/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Test/Randomized/RandomRun.hpp>
#include <Prism/Test/TestResult.hpp>

namespace Prism::Test
{
    /// Small, fast PRNG (xoshiro256**) used for live randomized runs.
    class Random
    {
      public:
        explicit Random(u64 seed = 0x5eed) { Seed(seed); }

        void Seed(u64 seed)
        {
            // splitmix64 to spread the seed over the state.
            for (auto& word : m_State)
            {
                seed += 0x9e3779b97f4a7c15ull;
                u64 z = seed;
                z     = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
                z     = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
                word  = z ^ (z >> 31);
            }
        }

        u64 Next()
        {
            u64 result = Rotl(m_State[1] * 5, 7) * 9;
            u64 t      = m_State[1] << 17;
            m_State[2] ^= m_State[0];
            m_State[3] ^= m_State[1];
            m_State[1] ^= m_State[2];
            m_State[0] ^= m_State[3];
            m_State[2] ^= t;
            m_State[3] = Rotl(m_State[3], 45);
            return result;
        }

        /// Uniform value in [0, max].
        u64 Uniform(u64 max)
        {
            if (max == ~0ull) return Next();
            u64 bound     = max + 1;
            u64 threshold = (0 - bound) % bound;
            for (;;)
            {
                u64 value = Next();
                if (value >= threshold) return value % bound;
            }
        }

        /// Uniform double in [0, 1).
        double UniformDouble()
        { return static_cast<double>(Next() >> 11) * 0x1.0p-53; }

      private:
        u64        m_State[4]{};

        static u64 Rotl(u64 x, int k) { return (x << k) | (x >> (64 - k)); }
    };

    /**
     * Where generators get their randomness from: either live (fresh random
     * values, recorded into a RandomRun) or a replay of a recorded run (used
     * when shrinking and when reporting a counterexample).
     */
    class RandSource
    {
      public:
        static RandSource Live(Random& random)
        {
            RandSource source;
            source.m_Random = &random;
            return source;
        }
        static RandSource Recorded(const RandomRun& run)
        {
            RandSource source;
            source.m_Run = run;
            return source;
        }

        bool IsLive() const { return m_Random != nullptr; }

        /**
         * Draws a value in [0, max]. `live()` produces the value for live
         * runs; replays reuse the recorded value (clamped to `max`). Running
         * out of recorded values marks the test as overrun.
         */
        template <typename LiveFn>
        u64 DrawValue(u64 max, LiveFn&& live)
        {
            if (IsLive())
            {
                u64 value = live();
                if (value > max) value = max;
                m_Run.Append(value);
                return value;
            }

            if (m_Index >= m_Run.Size())
            {
                SetCurrentTestResult(TestResult::eOverrun);
                return 0;
            }
            u64 value = m_Run[m_Index++];
            return value > max ? max : value;
        }
        u64 DrawValue(u64 max)
        {
            return DrawValue(max, [&] { return m_Random->Uniform(max); });
        }

        Random&          LiveRandom() { return *m_Random; }
        const RandomRun& Run() const { return m_Run; }

      private:
        Random*   m_Random = nullptr;
        RandomRun m_Run;
        usize     m_Index = 0;
    };

    /// The source used by GEN() inside the running randomized test.
    inline constexpr RandSource& CurrentRandSource()
    { return Suite::CurrentRandSource(); }
} // namespace Prism::Test
