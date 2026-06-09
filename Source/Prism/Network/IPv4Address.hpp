/*
 * Created by v1tr10l7 on 12.07.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Array.hpp>
#include <Prism/Core/Platform.hpp>
#include <Prism/Memory/Endian.hpp>

#include <Prism/String/Formatter.hpp>
#include <Prism/String/StringUtils.hpp>

namespace Prism
{
    constexpr bool ParseIPv4(StringView str, NetworkOrdered<u32>& out)
    {
        u32   octets[4] = {};
        usize index     = 0;
        u32   current   = 0;

        for (char c : str)
        {
            if (c >= '0' && c <= '9')
            {
                current = current * 10 + (c - '0');
                if (current > 255) return false;
            }
            else if (c == '.')
            {
                if (index >= 4) return false;
                octets[index++] = current;
                current         = 0;
            }
            else return false;
        }

        if (index != 3) return false;
        octets[3]      = current;

        const u32 host = (octets[0] << 24) | (octets[1] << 16)
                       | (octets[2] << 8) | (octets[3]);

        out.Store(host);
        return true;
    }
    class PM_PACKED IPv4Address
    {
      public:
        constexpr IPv4Address() = default;

        constexpr IPv4Address(u32 seg1, u32 seg2, u32 seg3, u32 seg4)
            : IPv4Address(ToArray({seg1, seg2, seg3, seg4}))
        {
        }
        constexpr IPv4Address(Array<u32, 4> segments)
            : IPv4Address((segments[0] << 24) | (segments[1] << 16)
                          | (segments[2] << 8) | segments[3])
        {
        }
        constexpr explicit IPv4Address(StringView ip)
        {
            NetworkOrdered<u32> parsed{};
            m_Address = NetworkOrdered<u32>{0};
            if (ParseIPv4(ip, parsed)) m_Address = parsed;
        }
        constexpr IPv4Address(NetworkOrdered<u32> address)
            : m_Address(address)
        {
        }

        constexpr IPv4Address& operator=(const Array<u32, 4>& segments)
        {
            m_Address.Store((segments[0] << 24) | (segments[1] << 16)
                            | (segments[2] << 8) | (segments[3]));
            return *this;
        }
        constexpr IPv4Address& operator=(StringView ip)
        {
            NetworkOrdered<u32> parsed{};
            if (ParseIPv4(ip, parsed)) m_Address = parsed;
            return *this;
        }

        constexpr IPv4Address& operator=(NetworkOrdered<u32> ip)
        {
            return (m_Address = ip), *this;
        }

        constexpr NetworkOrdered<u32> Raw() const { return m_Address; }
        constexpr    operator NetworkOrdered<u32>() const { return m_Address; }

        constexpr u8 operator[](usize index) const
        {
            assert(index < 4);

            const u32 net   = m_Address.Load();
            const u32 shift = 24 - (index * 8);

            return (net >> shift) & 0xff;
        }

        constexpr String ToString() const
        {
            auto str = fmt::format(
                "{}.{}.{}.{}", operator[](0), operator[](1), operator[](2),
                                                             operator[](3));

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
        NetworkOrdered<u32> m_Address = 0;
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
struct Prism::Hash<Prism::IPv4Address>
{
    PM_NODISCARD Prism::usize
                 operator()(const Prism::IPv4Address& ip) const PM_NOEXCEPT
    {
        using namespace Prism;
        u32 key = ip.Raw().Load();

#if PM_CONTEXT_KERNEL != 0
        return Hash<Prism::u32>{}(key);
#else
        usize       length = sizeof(key);
        const usize seed   = 0xc70f6907ul;

        return Prism::Murmur::Hash2(reinterpret_cast<u8*>(&key), length, seed);
#endif
    }
};

#if PRISM_USE_NAMESPACE != 0
using Prism::IPv4Address;
#endif
