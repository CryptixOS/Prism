/*
 * Created by v1tr10l7 on 27.01.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Array.hpp>
#include <Prism/Core/Integer128.hpp>

namespace Prism
{
    class UUID
    {
      public:
        constexpr UUID(u128 uuid)
            : m_UUID(uuid)
        {
        }
        constexpr UUID(u64 uuid)
            : m_UUID(uuid)
        {
        }
        constexpr UUID(const Array<u8, 16>& uuid) { Assign(uuid); }
        constexpr UUID(u64 high, u64 low)
            : m_UUID(high, low)
        {
        }

        constexpr UUID(const UUID& other) { *this = other; }
        constexpr UUID(UUID&& other) { *this = Move(other); }

        constexpr UUID& operator=(u128 uuid) { return Assign(uuid); }
        constexpr UUID& operator=(u64 uuid) { return Assign(uuid); }
        constexpr UUID& operator=(const Array<u8, 16>& uuid)
        {
            return Assign(uuid);
        }
        constexpr UUID& operator=(const UUID& other)
        {
            return m_UUID = other.m_UUID, *this;
        }
        constexpr UUID& operator=(UUID&& other)
        {
            return m_UUID = Move(other.m_UUID), *this;
        }

        UUID& Assign(u128 uuid) { return (m_UUID = uuid), *this; }
        UUID& Assign(u64 uuid) { return (m_UUID = uuid), *this; }
        UUID& Assign(const Array<u8, 16>& uuid)
        {
            u64 low = 0;
            for (usize i = 0; i < 8; ++i)
                low |= static_cast<u64>(uuid[i]) << (i << 3);

            u64 high = 0;
            for (usize i = 8; i < 16; ++i)
                high |= static_cast<u64>(uuid[i]) << ((i - 8) << 3);

            return Assign(high, low);
        }
        UUID& Assign(u64 high, u64 low)
        {
            return (m_UUID = {high, low}), *this;
        }

        constexpr      operator bool() const { return m_UUID != 0; }

        constexpr u128 Raw() const { return m_UUID; }
        constexpr      operator u128() const { return Raw(); }
        constexpr u64  operator[](usize index) const
        {
            assert(index < 2);
            return m_UUID[index];
        }

        friend constexpr bool operator==(const UUID& lhs, const UUID& rhs)
        {
            return lhs.m_UUID == rhs.m_UUID;
        }
        friend constexpr auto operator<=>(const UUID& lhs, const UUID& rhs)
        {
            return lhs.m_UUID <=> rhs.m_UUID;
        }

      private:
        u128 m_UUID = {0};
    };

    inline constexpr void Swap(UUID& lhs, UUID& rhs)
    {
        lhs = Exchange(lhs, rhs);
    }
}; // namespace Prism

// hash support
template <>
struct std::hash<Prism::UUID>
{
    [[nodiscard]] Prism::usize
    operator()(const Prism::UUID& uuid) const PM_NOEXCEPT
    {
        using namespace Prism;
        u64 high = uuid[0];
        u64 low  = uuid[1];
        u64 key  = low ^ high;

#if PRISM_TARGET_CRYPTIX != 0
        return std::hash<u64>{}(key);
#else
        usize       length = sizeof(key);
        const usize seed   = 0xc70f6907ul;

        return Hash::MurmurHash2(reinterpret_cast<const char*>(&key), length,
                                 seed);
#endif
    }
};

#if PRISM_TARGET_CRYPTIX != 0
using Prism::UUID;
#endif
