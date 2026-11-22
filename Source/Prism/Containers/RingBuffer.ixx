module;

#include <Prism/Core/Types.hpp>
#include <Prism/Core/Compiler.hpp>

export module Prism.Containers.RingBuffer;

export import Prism.Utility.Atomic;
export import Prism.Core.Concepts;

export extern "C++"
{
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
                // Both run modulo twice the capacity (so full and empty
                // differ): a tail that wrapped is 2 * capacity ahead. It
                // read capacity - (tail - head), more than the capacity once
                // the indices wrapped: Free() underflowed and Write() copied
                // far past both buffers (seen after 128KiB through a pipe)
                auto head = m_Head.Load();
                auto tail = m_Tail.Load(MemoryOrder::eRelaxed);
                return (tail >= head) ? (tail - head)
                                      : (2 * m_Capacity - (head - tail));
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
}
