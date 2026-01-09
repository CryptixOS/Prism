/*
 * Created by v1tr10l7 on 08.01.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Array.hpp>

namespace Prism
{
    /**
     * @brief Fixed-size bit span with static storage.
     *
     * BitSpan manages a contiguous set of bits whose size is known at compile
     * time. Storage is embedded directly in the object; no heap allocation is
     * performed.
     *
     * This type is suitable for global and early-boot kernel resources such as
     * I/O port allocation maps, IRQ vectors, or fixed hardware ID spaces.
     *
     * @tparam BitCount Total number of bits managed by this BitSpan
     */
    template <usize BitCount>
    class BitSpan
    {
      public:
        /** Number of bits stored per machine word */
        static constexpr usize BitsPerWord = sizeof(usize) * 8;

        /** Number of words required to store BitCount bits */
        static constexpr usize WordCount
            = (BitCount + BitsPerWord - 1) / BitsPerWord;

        /**
         * @brief Construct a BitSpan with all bits cleared.
         */
        constexpr BitSpan()
            : m_Data{}
        {
        }

        BitSpan(const BitSpan&)            = delete;
        BitSpan& operator=(const BitSpan&) = delete;

        /**
         * @brief Test whether a single bit is set.
         *
         * @param bit Index of the bit to test
         * @return true if the bit is set, false otherwise
         */
        bool     Test(usize bit) const
        {
            assert(bit < BitCount);
            return m_Data[bit / BitsPerWord]
                 & (usize(1) << (bit % BitsPerWord));
        }

        /**
         * @brief Set a single bit.
         *
         * @param bit Index of the bit to set
         */
        void Set(usize bit)
        {
            assert(bit < BitCount);
            m_Data[bit / BitsPerWord] |= (usize(1) << (bit % BitsPerWord));
        }

        /**
         * @brief Clear a single bit.
         *
         * @param bit Index of the bit to clear
         */
        void Clear(usize bit)
        {
            assert(bit < BitCount);
            m_Data[bit / BitsPerWord] &= ~(usize(1) << (bit % BitsPerWord));
        }

        /**
         * @brief Clear all bits in the span.
         */
        void ClearAll()
        {
            for (usize i = 0; i < WordCount; ++i) m_Data[i] = 0;
        }

#ifdef PRISM_DEBUG
        /**
         * @brief Poison all bits (debug builds only).
         *
         * Useful to detect use-before-init bugs.
         */
        void PoisonAll()
        {
            for (usize i = 0; i < WordCount; ++i) m_Data[i] = ~usize(0);
        }
#endif

        /* ------------------------------------------------------------ */
        /* Range operations                                              */
        /* ------------------------------------------------------------ */

        bool TestRange(usize base, usize length) const
        {
            assert(base + length <= BitCount);

            for (usize i = 0; i < length; ++i)
            {
                if (Test(base + i)) return false;
            }
            return true;
        }

        void SetRange(usize base, usize length)
        {
            assert(base + length <= BitCount);

            for (usize i = 0; i < length; ++i) Set(base + i);
        }

        void ClearRange(usize base, usize length)
        {
            assert(base + length <= BitCount);

            for (usize i = 0; i < length; ++i) Clear(base + i);
        }

        /* ------------------------------------------------------------ */
        /* Allocation helper                                             */
        /* ------------------------------------------------------------ */

        /**
         * @brief Find a contiguous range of clear bits.
         *
         * This function does NOT modify the bitmap.
         *
         * @param length Number of contiguous bits required
         * @return Base index of the free range, or -1 if none exists
         */
        isize FindFreeRange(usize length) const
        {
            assert(length > 0);

            usize bit = 0;

            while (bit + length <= BitCount)
            {
                usize word = bit / BitsPerWord;

                /* Fast skip: whole word occupied */
                if (m_Data[word] == ~usize(0))
                {
                    bit = (word + 1) * BitsPerWord;
                    continue;
                }

                /* Slow path: check bit-by-bit */
                bool free = true;
                for (usize i = 0; i < length; ++i)
                {
                    if (Test(bit + i))
                    {
                        free = false;
                        bit += i + 1;
                        break;
                    }
                }

                if (free) return static_cast<isize>(bit);
            }

            return -1;
        }

      private:
        /** Embedded bit storage */
        Array<usize, WordCount> m_Data;
    };

} // namespace Prism

#if PRISM_USE_NAMESPACE != 0
using Prism::BitSpan;
#endif
