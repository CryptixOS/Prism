/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Vector.hpp>
#include <Prism/Core/Types.hpp>

namespace Prism::Test
{
    /**
     * The sequence of random choices a randomized test consumed. Every
     * generated value is derived from these numbers, so a failing input can
     * be replayed exactly and shrunk by simplifying the numbers: shorter
     * runs and smaller values produce simpler inputs.
     */
    class RandomRun
    {
      public:
        RandomRun() = default;
        explicit RandomRun(Vector<u64> data)
            : m_Data(Move(data))
        {
        }

        bool       IsEmpty() const { return m_Data.Size() == 0; }
        usize      Size() const { return m_Data.Size(); }
        u64        operator[](usize index) const { return m_Data[index]; }
        u64&       operator[](usize index) { return m_Data[index]; }
        void       Append(u64 value) { m_Data.PushBack(value); }

        const u64* begin() const { return m_Data.Raw(); }
        const u64* end() const { return m_Data.Raw() + m_Data.Size(); }

        /// Shorter runs are simpler; equally long runs compare
        /// lexicographically.
        bool       IsShortlexSmallerThan(const RandomRun& other) const
        {
            if (Size() != other.Size()) return Size() < other.Size();
            for (usize i = 0; i < Size(); ++i)
                if (m_Data[i] != other.m_Data[i])
                    return m_Data[i] < other.m_Data[i];
            return false;
        }

        /// Copy of the run without `count` values starting at `start`.
        RandomRun WithoutChunk(usize start, usize count) const
        {
            Vector<u64> data;
            data.Reserve(Size());
            for (usize i = 0; i < Size(); ++i)
                if (i < start || i >= start + count) data.PushBack(m_Data[i]);
            return RandomRun(Move(data));
        }

      private:
        Vector<u64> m_Data;
    };
} // namespace Prism::Test
