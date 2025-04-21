/*
 * Created by v1tr10l7 on 21.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

namespace Prism
{
    template <typename C>
    struct CharTraits
    {
        using CharType   = C;
        using IntType    = std::wint_t;
        using OffsetType = i64;
        using PosType    = i64;

        constexpr static void Assign(CharType&       c1,
                                     const CharType& c2) PM_NOEXCEPT
        {
            if (std::is_constant_evaluated())
                std::construct_at(std::addressof(c1), c2);
            else c1 = c2;
        }
        constexpr static CharType* Assign(CharType* dest, usize count,
                                          CharType ch)
        {
            if (count == 0) return dest;
            if (std::is_constant_evaluated()) return Assign(dest, count, ch);

            return static_cast<CharType*>(std::memset(dest, ch, count));
        }
        constexpr static bool Equal(CharType lhs, CharType rhs) PM_NOEXCEPT
        {
            return static_cast<unsigned char>(lhs)
                == static_cast<unsigned char>(rhs);
        }
        constexpr static bool Less(CharType lhs, CharType rhs) PM_NOEXCEPT
        {
            return static_cast<unsigned char>(lhs)
                 < static_cast<unsigned char>(rhs);
        }
        constexpr static CharType* Move(CharType* dest, const CharType* src,
                                        usize count)
        {
            assert(src && dest);
            if (count == 0) return dest;
            if (std::is_constant_evaluated())
            {
                if (__builtin_constant_p(src < dest) && dest > src
                    && dest < (src + count))
                {
                    do {
                        --count;
                        Assign(dest[count], src[count]);
                    } while (count > 0);
                }
                else copy(dest, src, count);
                return dest;
            }

            std::memmove(dest, src, count * sizeof(CharType));
            return dest;
        }
        constexpr static CharType* Copy(CharType* dest, const CharType* src,
                                        usize count)
        {
            if (count == 0) return dest;
            if (std::is_constant_evaluated())
            {
                for (usize i = 0; i < count; ++i)
                    std::construct_at(dest + i, src[i]);
                return dest;
            }
            std::memcpy(dest, src, count * sizeof(CharType));
            return dest;
        }
        constexpr static i32 Compare(const CharType* lhs, const CharType* rhs,
                                     usize count)
        {
            for (usize i = 0; i < count; ++i)
                if (Less(lhs[i], rhs[i])) return -1;
                else if (Less(rhs[i], lhs[i])) return 1;
            return 0;
        }
        constexpr static usize Length(const CharType* string)
        {
            usize i = 0;
            while (!Equal(string[i], CharType())) ++i;

            return i;
        }
        static constexpr const CharType* Find(const CharType* string,
                                              usize count, const CharType& ch)
        {
            for (usize i = 0; i < count; ++i)
                if (Equal(string[i], ch)) return string + i;
            return 0;
        }
        constexpr static CharType ToCharType(const i32& c)
        {
            return static_cast<CharType>(c);
        }

        constexpr static IntType ToIntType(const CharType& c)
        {
            return static_cast<IntType>(c);
        }
        constexpr static bool EqualIntType(const IntType& c1, const i32& c2)
        {
            return c1 == c2;
        }
        constexpr static IntType IsEof() { return -1; }

        constexpr static IntType IsNotEof(const IntType& c)
        {
            return !EqualIntType(c, IsEof()) ? c : ToIntType(CharType());
        }
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::CharTraits;
#endif
