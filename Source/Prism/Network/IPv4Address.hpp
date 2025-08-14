/*
 * Created by v1tr10l7 on 12.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Array.hpp>
#include <Prism/Memory/Endian.hpp>

#include <Prism/String/Formatter.hpp>
#include <Prism/String/StringUtils.hpp>

namespace Prism
{
    class [[gnu::packed]] IPv4Address
    {
      public:
        constexpr IPv4Address() = default;

        constexpr IPv4Address(u32 seg1, u32 seg2, u32 seg3, u32 seg4)
            : IPv4Address(ToArray({seg1, seg2, seg3, seg4}))
        {
        }
        constexpr IPv4Address(Array<u32, 4> segments)
            : IPv4Address((segments[3] << 24) | (segments[2] << 16)
                          | (segments[1] << 8) | segments[0])
        {
        }
        constexpr IPv4Address(StringView ip)
            : IPv4Address(StringUtils::ToNumber<u32>(ip))
        {
        }
        constexpr IPv4Address(NetworkOrdered<u32> address)
            : m_Address(address)
        {
        }

        constexpr IPv4Address& operator=(Array<u32, 4> segments)
        {
            return (*this = (segments[3] << 24) | (segments[2] << 16)
                          | segments[1] << 8 | segments[0]),
                 *this;
        }
        constexpr IPv4Address& operator=(StringView ip)
        {
            return (*this = StringUtils::ToNumber<u32>(ip)), *this;
        }

        constexpr IPv4Address& operator=(NetworkOrdered<u32> ip)
        {
            return (m_Address = ip), *this;
        }

        constexpr NetworkOrdered<u32> Raw() const { return m_Address; }
        constexpr    operator NetworkOrdered<u32>() const { return m_Address; }

        constexpr u8 operator[](isize index) const
        {
            assert(index >= 0 && index < 4);

            constexpr usize BitsPerByte = 8;
            const u32       bitsToShift = BitsPerByte * index;

            return (m_Address >> bitsToShift) & 0xff;
        }

        String ToString() const
        {
            auto str = Format(
                "{:.4}.{:.4}.{:.4}.{:.4}",
                operator[](0), operator[](1), operator[](2), operator[](3));

            return String(str.data(), str.size());
        }
        constexpr             operator String() const { return ToString(); }

        friend constexpr bool operator==(const IPv4Address& lhs,
                                         const IPv4Address& rhs)
            = default;
        friend constexpr bool operator!=(const IPv4Address& lhs,
                                         const IPv4Address& rhs)
            = default;

      private:
        u32 m_Address = 0;
    };

    static_assert(sizeof(IPv4Address) == 4);
}; // namespace Prism

#if PRISM_DISABLE_FMT == 0
template <>
struct fmt::formatter<Prism::IPv4Address> : fmt::formatter<fmt::string_view>
{
    template <typename FormatContext>
    auto format(const Prism::IPv4Address ip, FormatContext& ctx) const
    {
        return ip;
    }
};
#endif

// hash support
template <>
struct Hash<Prism::IPv4Address>
{
    [[nodiscard]] Prism::usize
    operator()(const Prism::IPv4Address& ip) const PM_NOEXCEPT
    {
        using namespace Prism;
        u32 key = ip.Raw().Load();

#if PRISM_TARGET_CRYPTIX != 0
        return Hash<Prism::u32>{}(key);
#else
        usize       length = sizeof(key);
        const usize seed   = 0xc70f6907ul;

        return Hash::MurmurHash2(reinterpret_cast<const char*>(&key), length,
                                 seed);
#endif
    }
};

#if PRISM_TARGET_CRYPTIX != 0
using Prism::IPv4Address;
#endif
