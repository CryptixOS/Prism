/*
 * Created by v1tr10l7 on 08.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Memory/Memory.hpp>

namespace Prism
{
    namespace Memory
    {
        Pointer Copy(Pointer destination, const Pointer source, usize count)
        {
            char*       current = destination.As<char>();
            const char* s       = source.As<const char>();

            while (count-- > 0) *current++ = *s++;
            return destination;
        }
        void* CopyAligned(Pointer destination, const Pointer source, usize size)
        {
            u8*       d = destination.As<u8>();
            const u8* s = source.As<const u8>();

            // If not 8-byte aligned, fallback
            if ((destination.Raw() | source.Raw()) & 7)
                return Copy(destination, source, size);

            usize i = 0;
            for (; i + 8 <= size; i += 8)
            {
                reinterpret_cast<u64*>(d)[i / 8]
                    = reinterpret_cast<const u64*>(s)[i / 8];
            }

            for (; i < size; ++i) d[i] = s[i];

            return destination;
        }

        i64 Compare(const Pointer lhs, const Pointer rhs, usize count)
        {
            const unsigned char* s1     = lhs.As<const unsigned char>();
            const unsigned char* s2     = rhs.As<const unsigned char>();

            i64                  result = 0;
            for (; count > 0 && result == 0; ++s1, ++s2, count--)
                result = *s1 - *s2;

            return result;
        }
        Pointer Fill(const Pointer destination, u8 value, usize count)
        {
            char* current = destination.As<char>();

            while (count-- > 0) *current++ = value;
            return destination;
        }
        Pointer Move(Pointer destination, const Pointer source, usize count)
        {
            char*       current = destination.As<char>();
            const char* s       = source.As<const char>();

            if (current >= s) goto reversed;
            for (usize i = 0; i < count; ++i) current[i] = s[i];
            goto end;

        reversed:
            for (usize i = count; i-- > 0;) current[i] = s[i];

        end:
            return destination;
        }
        Pointer ScanForCharacter(const Pointer memory, u8 c, usize size)
        {
            const unsigned char* current = memory.As<unsigned char>();
            while (size-- > 0 && *current != static_cast<unsigned char>(c))
                ++current;

            return size > 0 && *current == static_cast<unsigned char>(c)
                     ? current - 1
                     : nullptr;
        }
    }; // namespace Memory
}; // namespace Prism
