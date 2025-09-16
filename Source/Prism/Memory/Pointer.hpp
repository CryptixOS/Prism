/*
 * Created by v1tr10l7 on 19.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Concepts.hpp>
#include <Prism/Core/HashTraits.hpp>

#if PRISM_TARGET_CRYPTIX != 0
namespace MM
{
    extern usize HigherHalfOffset();
};
#endif

namespace Prism
{
    using VirtAddr = pointer;

    template <typename T>
    PM_NODISCARD constexpr T* Launder(T* object) noexcept
    {
        if constexpr (IsSameV<const volatile T, const volatile void>)
            static_assert(!IsSameV<const volatile T, const volatile void>,
                          "Launder argument must not be a void pointer");
        else if constexpr (IsFunctionV<T>)
            static_assert(!IsFunctionV<T>,
                          "Launder argument must not be a function pointer");
        else return __builtin_launder(object);
        return nullptr;
    }

    template <typename T>
    concept PointerType = IsPointerV<T>;
    template <typename T>
    concept PointerHolder = requires {
        requires(PointerType<T>)
                    || UnsignedIntegral<T> || IsSameV<T, struct Pointer>;
    };

    struct Pointer
    {

        inline constexpr Pointer() = default;
        inline constexpr Pointer(NullType) { m_Pointer = 0; }

        template <PointerHolder T = VirtAddr>
        inline constexpr Pointer(const T pointer)
        {
            if constexpr (IsPointerV<T>)
                m_Pointer = reinterpret_cast<VirtAddr>(pointer);
            else m_Pointer = pointer;
        }

        template <PointerHolder T>
        inline constexpr Pointer& operator=(T addr)
        {
            if constexpr (IsPointerV<T>)
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

        template <typename T>
        inline constexpr auto& operator->()
        {
            return *As<T>();
        }
        template <typename T>
        inline constexpr auto& operator*()
        {
            return *As<T>();
        }

        inline constexpr bool IsValid() const { return m_Pointer != 0; }

#if PRISM_TARGET_CRYPTIX != 0
        VirtAddr HigherHalfOffset() const { return MM::HigherHalfOffset(); }
        inline constexpr bool IsHigherHalf() const
        {
            return m_Pointer >= HigherHalfOffset();
        }

        template <PointerHolder T = VirtAddr>
        inline constexpr T ToHigherHalf() const
        {
            auto higherHalf
                = IsHigherHalf() ? m_Pointer : m_Pointer + HigherHalfOffset();

            if constexpr (IsPointerV<T>) return reinterpret_cast<T>(higherHalf);
            else if constexpr (IsSameV<T, Pointer>) return higherHalf;
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

            if constexpr (IsPointerV<T>) return reinterpret_cast<T>(lowerHalf);
            else if constexpr (IsSameV<T, Pointer>) return lowerHalf;
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

            if constexpr (IsPointerV<T>) return reinterpret_cast<T>(addr);
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
        Pointer& operator--()
        {
            --m_Pointer;
            return *this;
        }
        Pointer operator--(int)
        {
            Pointer ret = *this;
            --m_Pointer;
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
        if constexpr (IsSameV<T, Pointer>) return lhs.Raw() | rhs.Raw();

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
    inline constexpr Pointer operator<<(Pointer& lhs, upointer rhs)
    {
        return lhs.Raw() << rhs;
    }
    inline constexpr Pointer operator>>(Pointer& lhs, upointer rhs)
    {
        return lhs.Raw() >> rhs;
    }

    // hash support
    template <>
    struct Hash<Pointer>
    {
        PM_NODISCARD usize operator()(const Pointer& pointer) const PM_NOEXCEPT
        {
            return Hash<Prism::pointer*>{}(pointer.Raw());
        }
    };
}; // namespace Prism

constexpr Prism::Pointer operator""_virt(unsigned long long address)
{
    return address;
}

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Pointer;
using Prism::PointerHolder;
using Prism::PointerType;
using Prism::VirtAddr;
#endif

#if PRISM_DISABLE_FMT == 0
template <>
struct fmt::formatter<Prism::Pointer> : fmt::formatter<fmt::string_view>
{
    template <typename FormatContext>
    auto format(const Prism::Pointer addr, FormatContext& ctx) const
    {
        return fmt::formatter<fmt::string_view>::format(
            fmt::format("{:#x}", addr.Raw()), ctx);
    }
};
#endif
