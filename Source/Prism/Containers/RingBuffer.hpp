#pragma once

#include <Prism/Core/Types.hpp>
#include <Prism/Utility/Atomic.hpp>

namespace Prism
{
    class RingBuffer
    {
      public:
        constexpr RingBuffer() = default;
        explicit RingBuffer(usize capacity);
        ~RingBuffer();

        constexpr bool  Empty() const { return m_Head == m_Tail; }

        constexpr usize Capacity() const { return m_Capacity; }
        constexpr usize Used() const
        {
            auto head = m_Head.Load();
            auto tail = m_Tail.Load(MemoryOrder::eAtomicRelaxed);
            return (tail >= head) ? (tail - head)
                                  : (m_Capacity - (tail - head));
        }
        constexpr usize Free() const { return m_Capacity - Used(); }

        void            Reserve(usize newCapacity);

        usize           Read(u8* const buffer, usize size);
        usize           Write(const u8* const data, usize size);

      private:
        u8*           m_Buffer   = nullptr;
        usize         m_Capacity = 0;
        Atomic<usize> m_Head     = 0;
        Atomic<usize> m_Tail     = 0;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::RingBuffer;
#endif
