/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

export module Prism.Algorithm.SipHash;

export import Prism.Core.HashTraits;
export import Prism.String.String;
export import Prism.String.StringView;

/**
 * SipHash-2-4 (Aumasson & Bernstein): a keyed hash that is fast on short
 * inputs and that, with a secret key, an attacker can't find collisions for.
 * Use it for hash-table keys that come from outside (file names, network
 * input, user-supplied strings), so nobody can flood one bucket.
 *
 *   SipHash::Hash(key, data, size)   with an explicit 128-bit key
 *   SipHasher<T>                     hashes with the process-wide key;
 *                                    UnorderedMap<String, V, SipHasher<String>>
 *   SipHash::SetGlobalKey(key)       sets that key. Call it once at startup
 *                                    with real entropy (the kernel's RNG,
 *                                    getrandom()); until then the key is a
 *                                    fixed, public value.
 */

export extern "C++"
{
    namespace Prism
    {
        namespace SipHash
        {
            struct Key
            {
                u64 K0 = 0;
                u64 K1 = 0;
            };

            namespace Details
            {
                PM_ALWAYS_INLINE constexpr u64 Rotl(u64 x, u32 bits)
                {
                    return (x << bits) | (x >> (64 - bits));
                }
                struct State
                {
                    u64                V0, V1, V2, V3;

                    constexpr void Round()
                    {
                        V0 += V1;
                        V1 = Rotl(V1, 13);
                        V1 ^= V0;
                        V0 = Rotl(V0, 32);
                        V2 += V3;
                        V3 = Rotl(V3, 16);
                        V3 ^= V2;
                        V0 += V3;
                        V3 = Rotl(V3, 21);
                        V3 ^= V0;
                        V2 += V1;
                        V1 = Rotl(V1, 17);
                        V1 ^= V2;
                        V2 = Rotl(V2, 32);
                    }
                    constexpr void Compress(u64 m)
                    {
                        V3 ^= m;
                        Round();
                        Round();
                        V0 ^= m;
                    }
                };
                constexpr u64 LoadLittleEndian(const u8* p, usize n)
                {
                    u64 value = 0;
                    for (usize i = 0; i < n; ++i)
                        value |= static_cast<u64>(p[i]) << (8 * i);
                    return value;
                }

                inline Key s_GlobalKey = {0x0706050403020100ull,
                                          0x0f0e0d0c0b0a0908ull};
            } // namespace Details

            /// SipHash-2-4 of `size` bytes at `data` with `key`.
            constexpr u64 Hash(Key key, const u8* data, usize size)
            {
                Details::State s{key.K0 ^ 0x736f6d6570736575ull,
                                 key.K1 ^ 0x646f72616e646f6dull,
                                 key.K0 ^ 0x6c7967656e657261ull,
                                 key.K1 ^ 0x7465646279746573ull};

                usize          whole = size - size % 8;
                for (usize i = 0; i < whole; i += 8)
                    s.Compress(Details::LoadLittleEndian(data + i, 8));

                u64 last = static_cast<u64>(size & 0xff) << 56;
                last |= Details::LoadLittleEndian(data + whole, size - whole);
                s.Compress(last);

                s.V2 ^= 0xff;
                for (usize i = 0; i < 4; ++i) s.Round();
                return s.V0 ^ s.V1 ^ s.V2 ^ s.V3;
            }
            inline u64 Hash(Key key, StringView string)
            {
                return Hash(key, reinterpret_cast<const u8*>(string.Raw()),
                            string.Size());
            }

            /// The key SipHasher uses.
            inline void SetGlobalKey(Key key) { Details::s_GlobalKey = key; }
            inline Key  GlobalKey() { return Details::s_GlobalKey; }
        } // namespace SipHash

        /// Hasher for keys from untrusted sources, with SipHash's global key.
        template <typename T>
        struct SipHasher;

        template <typename C, typename Traits>
        struct SipHasher<BasicStringView<C, Traits>>
        {
            usize operator()(BasicStringView<C, Traits> string) const
            {
                return static_cast<usize>(SipHash::Hash(
                    SipHash::GlobalKey(),
                    reinterpret_cast<const u8*>(string.Raw()),
                    string.Size() * sizeof(C)));
            }
        };
        template <typename C, typename Traits>
        struct SipHasher<BasicString<C, Traits>>
            : SipHasher<BasicStringView<C, Traits>>
        {
        };
        template <typename T>
            requires IsIntegralV<T>
        struct SipHasher<T>
        {
            usize operator()(T value) const
            {
                u8 bytes[sizeof(T)];
                for (usize i = 0; i < sizeof(T); ++i)
                    bytes[i] = static_cast<u8>(static_cast<u64>(value) >> (8 * i));
                return static_cast<usize>(
                    SipHash::Hash(SipHash::GlobalKey(), bytes, sizeof(T)));
            }
        };
    } // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    namespace SipHash = Prism::SipHash;
    using Prism::SipHasher;
#endif
}
