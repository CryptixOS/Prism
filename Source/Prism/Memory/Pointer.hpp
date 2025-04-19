/*
 * Created by v1tr10l7 on 19.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>
#if PRISM_TARGET_CRYPTIX == 1
namespace BootInfo
{
    extern u64 GetHHDMOffset();
};
#endif

namespace Prism
{
    using VirtAddr = std::uintptr_t;

    template <typename T>
    concept PointerType = std::is_pointer_v<T>;
    template <typename T>
    concept PointerHolder = requires {
        requires(PointerType<T>) || std::unsigned_integral<T>
                    || std::is_same_v<T, struct Pointer>;
    };

    struct Pointer
    {

        inline constexpr Pointer() = default;
        inline constexpr Pointer(std::nullptr_t) { m_Pointer = 0; }

        template <PointerHolder T = VirtAddr>
        inline constexpr Pointer(const T pointer)
        {
            if constexpr (std::is_pointer_v<T>)
                m_Pointer = reinterpret_cast<VirtAddr>(pointer);
            else m_Pointer = pointer;
        }

        template <PointerHolder T>
        inline constexpr Pointer& operator=(T addr)
        {
            if constexpr (std::is_pointer_v<T>)
                m_Pointer = reinterpret_cast<VirtAddr>(addr);
            else m_Pointer = addr;

            return *this;
        }

        inline constexpr operator VirtAddr() const { return m_Pointer; }
        template <PointerType T>
        inline constexpr operator T() const
        {
            return reinterpret_cast<T>(m_Pointer);
        }
        inline constexpr operator bool() const { return IsValid(); }

        template <typename T>
        inline constexpr T* As() const
        {
            return reinterpret_cast<T*>(m_Pointer);
        }
        template <typename T = VirtAddr>
        inline constexpr T Raw() const
        {
            return T(m_Pointer);
        }

        inline constexpr auto& operator->() { return *As<u64>(); }
        inline constexpr auto& operator*() { return *As<u64>(); }

        inline constexpr bool  IsValid() const { return m_Pointer != 0; }

#ifdef PRISM_TARGET_CRYPTIX
        inline VirtAddr HigherHalfOffset() const
        {
            return BootInfo::GetHHDMOffset();
        }
        inline constexpr bool IsHigherHalf() const
        {
            return m_Pointer >= HigherHalfOffset();
        }

        template <PointerHolder T = VirtAddr>
        inline constexpr T ToHigherHalf() const
        {
            auto higherHalf
                = IsHigherHalf() ? m_Pointer : m_Pointer + HigherHalfOffset();

            if constexpr (std::is_pointer_v<T>)
                return reinterpret_cast<T>(higherHalf);
            else if constexpr (std::is_same_v<T, Pointer>) return higherHalf;
            else return static_cast<T>(higherHalf);
        }
        inline constexpr Pointer ToHigherHalf() const
        {
            return ToHigherHalf<VirtAddr>();
        }

        template <PointerHolder T = VirtAddr>
        inline constexpr T FromHigherHalf() const
        {
            auto lowerHalf
                = IsHigherHalf() ? m_Pointer - HigherHalfOffset() : m_Pointer;

            if constexpr (std::is_pointer_v<T>)
                return reinterpret_cast<T>(lowerHalf);
            else if constexpr (std::is_same_v<T, Pointer>) return lowerHalf;
            else return static_cast<T>(lowerHalf);
        }
        template <>
        inline constexpr Pointer FromHigherHalf() const
        {
            return FromHigherHalf<VirtAddr>();
        }
#endif

        template <PointerHolder T = VirtAddr>
        inline constexpr T Offset(usize offset) const
        {
            auto addr = m_Pointer + offset;

            if constexpr (std::is_pointer_v<T>)
                return reinterpret_cast<T>(addr);
            else return addr;
        }

        inline constexpr auto operator<=>(const Pointer& other) const = default;
        inline constexpr auto operator<=>(const usize& other) const
        {
            return m_Pointer <=> other;
        }

        inline constexpr Pointer& operator+=(usize bytes)
        {
            m_Pointer += bytes;

            return *this;
        }
        inline constexpr Pointer& operator-=(Pointer rhs)
        {
            m_Pointer -= rhs.Raw();
            return *this;
        }

        inline constexpr Pointer& operator|=(Pointer rhs)
        {
            m_Pointer |= rhs.m_Pointer;

            return *this;
        }
        inline constexpr Pointer& operator&=(Pointer rhs)
        {
            m_Pointer &= rhs.m_Pointer;

            return *this;
        }
        inline constexpr Pointer& operator<<=(Pointer rhs)
        {
            m_Pointer <<= rhs.m_Pointer;

            return *this;
        }
        inline constexpr Pointer& operator>>=(Pointer rhs)
        {
            m_Pointer >>= rhs.m_Pointer;

            return *this;
        }

        Pointer& operator++()
        {
            ++m_Pointer;
            return *this;
        }
        Pointer operator++(int)
        {
            Pointer ret = *this;
            ++m_Pointer;
            return ret;
        }

      private:
        VirtAddr m_Pointer = 0;
    };

    template <>
    constexpr Pointer Pointer::Offset<Pointer>(usize offset) const
    {
        return m_Pointer + offset;
    }

    inline constexpr Pointer operator+(Pointer lhs, Pointer rhs)
    {
        return lhs.Raw() + rhs.Raw();
    }
    inline constexpr Pointer operator-(Pointer lhs, Pointer rhs)
    {
        return lhs.Raw() - rhs.Raw();
    }
    inline constexpr Pointer operator%(Pointer& lhs, usize rhs)
    {
        return lhs.Raw() % rhs;
    }
    template <PointerHolder T>
    inline constexpr Pointer operator|(Pointer& lhs, const T rhs)
    {
        if constexpr (std::is_same_v<T, Pointer>) return lhs.Raw() | rhs.Raw();

        return lhs.Raw() | rhs.Raw();
    }
    inline constexpr Pointer operator&(Pointer lhs, const usize rhs)
    {
        return lhs.Raw() & rhs;
    }
    inline constexpr Pointer operator&(Pointer lhs, const Pointer rhs)
    {
        return lhs.Raw() & rhs.Raw();
    }
    inline constexpr Pointer operator<<(Pointer& lhs, uintptr_t rhs)
    {
        return lhs.Raw() << rhs;
    }
    inline constexpr Pointer operator>>(Pointer& lhs, uintptr_t rhs)
    {
        return lhs.Raw() >> rhs;
    }
}; // namespace Prism

constexpr Prism::Pointer operator""_virt(unsigned long long address)
{
    return address;
}

#if PRISM_TARGET_CRYPTIX == 1
using Prism::Pointer;
using Prism::PointerHolder;
using Prism::PointerType;
using Prism::VirtAddr;
#endif

template <>
struct fmt::formatter<Prism::Pointer> : fmt::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const Prism::Pointer addr, FormatContext& ctx) const
    {
        return fmt::formatter<std::string>::format(
            fmt::format("{}", addr.Raw()), ctx);
    }
};
