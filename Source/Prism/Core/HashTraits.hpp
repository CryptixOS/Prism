/*
 * Created by v1tr10l7 on 14.08.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Algorithm/FNV1aHash.hpp>
#include <Prism/Core/Concepts.hpp>

namespace Prism
{
    template <typename Result, typename Arg>
    struct HashBase
    {
    };

    template <typename T>
    struct Hash;

    template <typename T, typename = void>
    constexpr bool IsHashEnabledFor = false;
    template <typename T>
    constexpr bool
        IsHashEnabledFor<T, VoidType<decltype(Hash<T>()(DeclVal<T>()))>>
        = true;

    template <typename T, bool = true>
    struct HashEnum : public HashBase<usize, T>
    {
        usize operator()(T value) const PM_NOEXCEPT
        {
            using Type = typename UnderlyingType<T>::Type;
            return Hash<Type>{}(static_cast<Type>(value));
        }
    };

    template <typename T>
    struct NotHashable
    {
        NotHashable(NotHashable&&) = delete;
        ~NotHashable()             = delete;
    };
    template <typename T>
    struct Hash : ConditionalType<IsEnumV<T>, HashEnum<T>, NotHashable<T>>
    {
    };
    template <typename T>
    struct Hash<T*> : HashBase<usize, T*>
    {
        usize operator()(T value) const PM_NOEXCEPT
        {
            return static_cast<usize>(value);
        }
    };

#define DefineTrivialHash(type)                                                \
    template <>                                                                \
    struct Hash<type> : public HashBase<usize, type>                           \
    {                                                                          \
        usize operator()(type value) const PM_NOEXCEPT                         \
        {                                                                      \
            return static_cast<usize>(value);                                  \
        }                                                                      \
    };

    DefineTrivialHash(bool);
    DefineTrivialHash(char);
    DefineTrivialHash(signed char);
    DefineTrivialHash(unsigned char);
    DefineTrivialHash(wchar_t);
    DefineTrivialHash(char8_t);
    DefineTrivialHash(char16_t);
    DefineTrivialHash(char32_t);
    DefineTrivialHash(short);
    DefineTrivialHash(int);
    DefineTrivialHash(long);
    DefineTrivialHash(long long);
    DefineTrivialHash(unsigned short);
    DefineTrivialHash(unsigned int);
    DefineTrivialHash(unsigned long);
    DefineTrivialHash(unsigned long long);
#undef DefineTrivialHash

    namespace Murmur
    {
        usize Hash2(u8* input, usize length, const u64 seed);
    }; // namespace Murmur

#if PRISM_TARGET_CRYPTIX == 0
    template <>
    struct Hash<float> : public HashBase<usize, float>
    {
        usize operator()(float value) const PM_NOEXCEPT
        {
            return value != 0x0f
                     ? FNV1a::Hash(reinterpret_cast<u8*>(&value), sizeof(float))
                     : 0;
        }
    };
    template <>
    struct Hash<double> : public HashBase<usize, double>
    {
        usize operator()(double value) const PM_NOEXCEPT
        {
            return value != 0x0f ? FNV1a::Hash(reinterpret_cast<u8*>(&value),
                                               sizeof(double))
                                 : 0;
        }
    };
    template <>
    struct Hash<long double> : public HashBase<usize, long double>
    {
        usize operator()(long double value) const PM_NOEXCEPT
        {
            return value != 0x0f ? FNV1a::Hash(reinterpret_cast<u8*>(&value),
                                               sizeof(long double))
                                 : 0;
        }
    };
#endif

    template <typename Hash>
    struct IsFastHash : public TrueType
    {
    };

    template <>
    struct IsFastHash<Hash<long double>> : public FalseType
    {
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Hash;
using Prism::IsFastHash;
using Prism::IsHashEnabledFor;
#endif
