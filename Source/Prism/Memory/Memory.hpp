/*
 * Created by v1tr10l7 on 08.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Memory/Pointer.hpp>

namespace Prism
{
    using byte  = u8;
    using word  = u16;
    using dword = u32;
    using qword = u64;

    template <typename T>
    PM_ALWAYS_INLINE constexpr T* ToAddress(T* addr) PM_NOEXCEPT
    {
        static_assert(!IsFunctionV<T>,
                      "ToAddress argument "
                      "must not be a function pointer");
        return addr;
    }

    template <typename Pointer>
    constexpr auto ToAddress(const Pointer& addr) PM_NOEXCEPT
    {
        // if constexpr (requires { PointerTraits<Pointer>::ToAddress(addr); })
        //     return PointerTraits<Pointer>::ToAddress(addr);
        return ToAddress(addr.operator->());
    }

    template <typename Pointer>
    PM_ALWAYS_INLINE constexpr auto
    ToAddressImpl(const Pointer& addr) PM_NOEXCEPT
    {
        return ToAddress(addr);
    }

    constexpr usize operator""_kib(unsigned long long count)
    {
        return count * 1024;
    }
    constexpr usize operator""_mib(unsigned long long count)
    {
        return count * 1024_kib;
    }
    constexpr usize operator""_gib(unsigned long long count)
    {
        return count * 1024_mib;
    }
    constexpr usize operator""_tib(unsigned long long count)
    {
        return count * 1024_gib;
    }
    constexpr usize operator""_pib(unsigned long long count)
    {
        return count * 1024_tib;
    }

    namespace Memory
    {
        Pointer Copy(Pointer destination, const Pointer source, usize count);
        // Requires both src and dest to be 8-byte aligned
        void*   CopyAligned(Pointer destination, const Pointer source,
                            usize size);

        i64     Compare(const Pointer lhs, const Pointer rhs, usize count);
        Pointer Fill(const Pointer destination, u8 value, usize count);
        Pointer Move(Pointer destination, const Pointer source, usize count);
        Pointer ScanForCharacter(const Pointer memory, u8 c, usize size);
    }; // namespace Memory
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
namespace Memory = Prism::Memory;
using Prism::operator""_kib;
using Prism::operator""_mib;
using Prism::operator""_gib;
using Prism::operator""_tib;
using Prism::operator""_pib;
#endif
