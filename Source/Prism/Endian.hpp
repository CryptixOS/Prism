/*
 * Created by v1tr10l7 on 14.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

namespace Prism
{
    enum class Endian
    {
        eLittle = __ORDER_LITTLE_ENDIAN__,
        eBig    = __ORDER_BIG_ENDIAN__,
        eNative = __BYTE_ORDER__,
    };
    constexpr static Endian g_HostEndianness = Endian::eNative;

    template <typename T>
    concept IntegralOrUnderlyingIntegral
        = std::is_integral_v<T>
       || (std::is_enum_v<T> && std::is_integral_v<std::underlying_type_t<T>>);
    template <typename T>
    concept IntegralUnderlying
        = std::is_enum_v<T> && std::is_integral_v<std::underlying_type_t<T>>;

    template <Endian New, Endian Old = Endian::eNative, std::integral T>
    inline constexpr T ConvertEndian(T num)
        requires(sizeof(T) <= 8 && std::has_unique_object_representations_v<T>)
    {
        if constexpr (New == Old) return num;

        return std::byteswap(num);
    }
    template <Endian New, Endian Old = Endian::eNative, IntegralUnderlying T>
    inline constexpr T ConvertEndian(T num)
        requires(sizeof(T) <= 8 && std::has_unique_object_representations_v<T>)
    {
        if constexpr (New == Old) return num;

        return static_cast<T>(std::byteswap(std::to_underlying(num)));
    }

    template <Endian New, IntegralOrUnderlyingIntegral T>
    inline constexpr T ToEndian(T num)
    {
        return ConvertEndian<New, Endian::eNative>(num);
    }

    template <Endian Old, IntegralOrUnderlyingIntegral T>
    inline constexpr T FromEndian(T num)
    {
        return ConvertEndian<Endian::eNative, Old>(num);
    }

    template <typename T, Endian E>
    struct [[gnu::packed]] EndianStorage
    {
        inline constexpr EndianStorage() = default;
        inline constexpr EndianStorage(T value)
            : Value(value)
        {
        }

        inline constexpr T Load() const
        {
            return ConvertEndian<Endian::eNative, E>(Value);
        }

        inline constexpr void Store(T value)
        {
            Value = ConvertEndian<E, Endian::eNative>(value);
        }

        constexpr auto operator<=>(const usize other) const
        {
            return ConvertEndian<E, Endian::eNative>(Value) <=> other;
        }
        inline constexpr operator T() { return Value; }

        T                Value;
    };

    template <typename T>
    using LittleEndian = EndianStorage<T, Endian::eLittle>;
    template <typename T>
    using BigEndian = EndianStorage<T, Endian::eBig>;
    template <typename T>
    using NetworkOrdered = BigEndian<T>;
}; // namespace Prism

template <typename T, Prism::Endian E>
struct fmt::formatter<Prism::EndianStorage<T, E>> : fmt::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const Prism::EndianStorage<T, E>& value,
                FormatContext&                    ctx) const
    {
        return fmt::formatter<std::string>::format(
            fmt::format("{}", value.Load()), ctx);
    }
};

#if PRISM_TARGET_CRYPTIX == 1
using Prism::BigEndian;
using Prism::ConvertEndian;
using Prism::Endian;
using Prism::EndianStorage;
using Prism::LittleEndian;
#endif
