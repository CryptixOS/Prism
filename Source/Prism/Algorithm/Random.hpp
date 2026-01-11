/*
 * Created by v1tr10l7 on 27.04.2025.
 * Copyright (C) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Bits.hpp>
#include <Prism/Core/Core.hpp>
#include <Prism/Core/Iterator.hpp>
#include <Prism/Core/Limits.hpp>
#include <Prism/Utility/Math.hpp>

namespace Prism::Algorithm
{
    template <typename InputIt1, typename InputIt2>
    constexpr bool Equal(InputIt1 first1, InputIt1 last1, InputIt2 first2)
    {
        for (; first1 != last1; ++first1, ++first2)
            if (!(*first1 == *first2)) return false;

        return true;
    }
    template <typename InputIt1, typename InputIt2, typename BinaryPred>
    constexpr bool Equal(InputIt1 first1, InputIt1 last1, InputIt2 first2,
                         BinaryPred p)
    {
        for (; first1 != last1; ++first1, ++first2)
            if (!p(*first1, *first2)) return false;

        return true;
    }

    namespace Details
    {
        // random-access iterator implementation (allows quick range size
        // detection)
        template <typename RandomIt1, typename RandomIt2>
        constexpr bool Equal(RandomIt1 first1, RandomIt1 last1,
                             RandomIt2 first2, RandomIt2 last2,
                             RandomAccessIteratorTag, RandomAccessIteratorTag)
        {
            if (last1 - first1 != last2 - first2) return false;

            for (; first1 != last1; ++first1, ++first2)
                if (!(*first1 == *first2)) return false;

            return true;
        }

        template <typename InputIt1, typename InputIt2>
        constexpr bool Equal(InputIt1 first1, InputIt1 last1, InputIt2 first2,
                             InputIt2 last2, InputIteratorTag, InputIteratorTag)
        {
            for (; first1 != last1 && first2 != last2; ++first1, ++first2)
                if (!(*first1 == *first2)) return false;

            return first1 == last1 && first2 == last2;
        }
        template <typename RandomIt1, typename RandomIt2, typename BinaryPred>
        constexpr bool Equal(RandomIt1 first1, RandomIt1 last1,
                             RandomIt2 first2, RandomIt2 last2, BinaryPred p,
                             RandomAccessIteratorTag, RandomAccessIteratorTag)
        {
            if (last1 - first1 != last2 - first2) return false;

            for (; first1 != last1; ++first1, ++first2)
                if (!p(*first1, *first2)) return false;

            return true;
        }

        template <typename InputIt1, typename InputIt2, typename BinaryPred>
        constexpr bool Equal(InputIt1 first1, InputIt1 last1, InputIt2 first2,
                             InputIt2 last2, BinaryPred p, InputIteratorTag,
                             InputIteratorTag)
        {
            for (; first1 != last1 && first2 != last2; ++first1, ++first2)
                if (!p(*first1, *first2)) return false;

            return first1 == last1 && first2 == last2;
        }

        template <typename SSeq, typename Engine>
        struct IsSeedSequence
        {
            static constexpr const bool Value
                = !IsConvertible<SSeq, typename Engine::ResultType>::Value
               && !IsSame<RemoveCvType<SSeq>, Engine>::Value;
        };

        template <typename UType, UType X, usize R>
        struct Log2Impl;

        template <unsigned long long X, usize R>
        struct Log2Impl<unsigned long long, X, R>
        {
            static const usize Value
                = X & (static_cast<unsigned long long>(1) << R)
                    ? R
                    : Log2Impl<unsigned long long, X, R - 1>::Value;
        };

        template <unsigned long long X>
        struct Log2Impl<unsigned long long, X, 0>
        {
            static const usize Value = 0;
        };

        template <usize R>
        struct Log2Impl<unsigned long long, 0, R>
        {
            static const usize Value = R + 1;
        };

        template <__uint128_t X, usize R>
        struct Log2Impl<__uint128_t, X, R>
        {
            static const usize Value
                = (X >> 64)
                    ? (64 + Log2Impl<unsigned long long, (X >> 64), 63>::Value)
                    : Log2Impl<unsigned long long, X, 63>::Value;
        };

        template <typename UType, UType X>
        struct Log2
        {
            static const usize Value
                = Log2Impl<typename Conditional<
                               sizeof(UType) <= sizeof(unsigned long long),
                               unsigned long long, __uint128_t>::Type,
                           X, sizeof(UType) * __CHAR_BIT__ - 1>::Value;
        };
    }; // namespace Details

    template <typename InputIt1, typename InputIt2>
    constexpr bool Equal(InputIt1 first1, InputIt1 last1, InputIt2 first2,
                         InputIt2 last2)
    {
        return Details::Equal(
            first1, last1, first2, last2,
            typename IteratorTraits<InputIt1>::IteratorCategory(),
            typename IteratorTraits<InputIt2>::IteratorCategory());
    }
    template <typename InputIt1, typename InputIt2, typename BinaryPred>
    constexpr bool Equal(InputIt1 first1, InputIt1 last1, InputIt2 first2,
                         InputIt2 last2, BinaryPred p)
    {
        return Details::Equal(
            first1, last1, first2, last2, p,
            typename IteratorTraits<InputIt1>::IteratorCategory(),
            typename IteratorTraits<InputIt2>::IteratorCategory());
    }

    /**
     * @brief Random number engine based on Mersenne Twister algorithm
     * @param W The power of two that determines the range of values generated
     * by the engine
     * @param N The degree of recurrence
     * @param M The middle word, an offset used in the recurrence relation
     * defining the state
     * @param R The number of bits of the lower bit-mask, also known as the
     * twist value
     * @param A The conditional xor-mask, i.e. the coefficients of the rational
     * normal form twist matrix
     * @param U,D,S,B,T,C,L The 1st to 7th components of the bit-scrambling
     * (tempering) matrix
     * @param F The initialization multiplier
     */
    template <typename Type, usize W, usize N, usize M, usize R, Type A,
              usize U, Type D, usize S, Type B, usize T, Type C, usize L,
              Type F>
    class MersenneTwisterEngine;

    template <typename Type, usize W, usize N, usize M, usize R, Type A,
              usize U, Type D, usize S, Type B, usize T, Type C, usize L,
              Type F>
    class MersenneTwisterEngine
    {
      public:
        using ResultType               = Type;

        static constexpr ResultType DT = NumericLimits<ResultType>::Digits;

        static constexpr ResultType MinValue = 0;
        static constexpr ResultType MaxValue
            = W == DT ? ResultType(~0) : (ResultType(1) << W) - ResultType(1);

        // engine characteristics
        static constexpr usize      WordSize                 = W;
        static constexpr usize      StateSize                = N;
        static constexpr usize      ShiftSize                = M;
        static constexpr usize      MaskBits                 = R;
        static constexpr ResultType XorMask                  = A;
        static constexpr usize      TemperingU               = U;
        static constexpr ResultType TemperingD               = D;
        static constexpr usize      TemperingS               = S;
        static constexpr ResultType TemperingB               = B;
        static constexpr usize      TemperingT               = T;
        static constexpr ResultType TemperingC               = C;
        static constexpr usize      TemperingL               = L;
        static constexpr ResultType InitializationMultiplier = F;

        static constexpr ResultType Min() { return MinValue; }
        static constexpr ResultType Max() { return MaxValue; }
        static constexpr ResultType DefaultSeed = 5489u;

        // constructors and seeding functions
        MersenneTwisterEngine()
            : MersenneTwisterEngine(DefaultSeed)
        {
        }
        explicit MersenneTwisterEngine(ResultType sd) { Seed(sd); }

        template <typename Sseq>
        explicit MersenneTwisterEngine(
            Sseq& q, typename EnableIf<Details::IsSeedSequence<
                         Sseq, MersenneTwisterEngine>::Value>::Type* = 0)
        {
            Seed(q);
        }
        void Seed(ResultType sd = DefaultSeed);
        template <typename Sseq>
        typename EnableIf<
            Details::IsSeedSequence<Sseq, MersenneTwisterEngine>::Value,
            void>::Type
        Seed(Sseq& q)
        {
            Seed(q, IntegralConstant<unsigned, 1 + (W - 1) / 32>());
        }

        ResultType operator()();

        void       Discard(unsigned long long z)
        {
            for (; z; --z) operator()();
        }

        template <typename UType, usize WP, usize NP, usize MP, usize RP,
                  UType AP, usize UP, UType DP, usize SP, UType BP, usize TP,
                  UType CP, usize LP, UType FP>
        friend bool
        operator==(const MersenneTwisterEngine<UType, WP, NP, MP, RP, AP, UP,
                                               DP, SP, BP, TP, CP, LP, FP>& x,
                   const MersenneTwisterEngine<UType, WP, NP, MP, RP, AP, UP,
                                               DP, SP, BP, TP, CP, LP, FP>& y);

        template <typename UType, usize WP, usize NP, usize MP, usize RP,
                  UType AP, usize UP, UType DP, usize SP, UType BP, usize TP,
                  UType CP, usize LP, UType FP>
        friend bool
        operator!=(const MersenneTwisterEngine<UType, WP, NP, MP, RP, AP, UP,
                                               DP, SP, BP, TP, CP, LP, FP>& x,
                   const MersenneTwisterEngine<UType, WP, NP, MP, RP, AP, UP,
                                               DP, SP, BP, TP, CP, LP, FP>& y);

      private:
        ResultType m_X[N];
        usize      m_I = 0;

        static_assert(0 < M, "Mersenne Twister Engine - Invalid parameters");
        static_assert(M <= N, "Mersenne Twister Engine - Invalid parameters");
        static_assert(W <= DT, "Mersenne Twister Engine - Invalid parameters");
        static_assert(2 <= W, "Mersenne Twister Engine - Invalid parameters");
        static_assert(R <= W, "Mersenne Twister Engine - Invalid parameters");
        static_assert(U <= W, "Mersenne Twister Engine - Invalid parameters");
        static_assert(S <= W, "Mersenne Twister Engine - Invalid parameters");
        static_assert(T <= W, "Mersenne Twister Engine - Invalid parameters");
        static_assert(L <= W, "Mersenne Twister Engine - Invalid parameters");
        static_assert(MinValue < MaxValue,
                      "Mersenne Twister Engine - Invalid parameters");
        static_assert(A <= MaxValue,
                      "Mersenne Twister Engine - Invalid parameters");
        static_assert(B <= MaxValue,
                      "Mersenne Twister Engine - Invalid parameters");
        static_assert(C <= MaxValue,
                      "Mersenne Twister Engine - Invalid parameters");
        static_assert(D <= MaxValue,
                      "Mersenne Twister Engine - Invalid parameters");
        static_assert(F <= MaxValue,
                      "Mersenne Twister Engine - Invalid parameters");

        template <typename Sseq>
        void Seed(Sseq& q, IntegralConstant<unsigned, 1>);
        template <typename Sseq>
        void Seed(Sseq& q, IntegralConstant<unsigned, 2>);

        template <usize Count>
            static typename EnableIf
            < Count<W, ResultType>::Type ShiftLeft(ResultType x)
        {
            return (x << Count) & MaxValue;
        }

        template <usize Count>
        static typename EnableIf<(Count >= W), ResultType>::Type
        ShiftLeft(ResultType)
        {
            return ResultType(0);
        }

        template <usize Count>
            static typename EnableIf
            < Count<DT, ResultType>::Type ShiftRight(ResultType x)
        {
            return x >> Count;
        }

        template <usize Count>
        static typename EnableIf<(Count >= DT), ResultType>::Type
        ShiftRight(ResultType)
        {
            return ResultType(0);
        }
    };

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    constexpr const usize MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b,
                                                t, c, l, f>::WordSize;

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    constexpr const usize MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b,
                                                t, c, l, f>::StateSize;

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    constexpr const usize MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b,
                                                t, c, l, f>::ShiftSize;

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    constexpr const usize MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b,
                                                t, c, l, f>::MaskBits;

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    constexpr const typename MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s,
                                                   b, t, c, l, f>::ResultType
        MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b, t, c, l,
                              f>::XorMask;

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    constexpr const usize MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b,
                                                t, c, l, f>::TemperingU;

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    constexpr const typename MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s,
                                                   b, t, c, l, f>::ResultType
        MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b, t, c, l,
                              f>::TemperingD;

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    constexpr const usize MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b,
                                                t, c, l, f>::TemperingS;

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    constexpr const typename MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s,
                                                   b, t, c, l, f>::ResultType
        MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b, t, c, l,
                              f>::TemperingB;

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    constexpr const usize MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b,
                                                t, c, l, f>::TemperingT;

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    constexpr const typename MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s,
                                                   b, t, c, l, f>::ResultType
        MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b, t, c, l,
                              f>::TemperingC;

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    constexpr const usize MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b,
                                                t, c, l, f>::TemperingL;

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    constexpr const typename MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s,
                                                   b, t, c, l, f>::ResultType
        MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b, t, c, l,
                              f>::InitializationMultiplier;

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    constexpr const typename MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s,
                                                   b, t, c, l, f>::ResultType
        MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b, t, c, l,
                              f>::DefaultSeed;

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>

    void
    MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b, t, c, l, f>::Seed(
        ResultType sd)
    {
        m_X[0] = sd & MaxValue;
        for (usize i = 1; i < n; ++i)
            m_X[i] = (f * (m_X[i - 1] ^ ShiftRight<w - 2>(m_X[i - 1])) + i)
                   & MaxValue;
        m_I = 0;
    }

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    template <typename Sseq>
    void
    MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b, t, c, l, f>::Seed(
        Sseq& q, IntegralConstant<unsigned, 1>)
    {
        const unsigned k = 1;
        u32            ar[n * k];
        q.Generate(ar, ar + n * k);
        for (usize i = 0; i < n; ++i)
            m_X[i] = static_cast<ResultType>(ar[i] & MaxValue);
        const ResultType mask
            = r == DT ? ResultType(~0) : (ResultType(1) << r) - ResultType(1);
        m_X = 0;
        if ((m_X[0] & ~mask) == 0)
        {
            for (usize i = 1; i < n; ++i)
                if (m_X[i] != 0) return;
            m_X[0] = ResultType(1) << (w - 1);
        }
    }

    template <typename Type, usize w, usize n, usize m, usize r, Type a,
              usize u, Type d, usize s, Type b, usize t, Type c, usize l,
              Type f>
    template <typename Sseq>
    void
    MersenneTwisterEngine<Type, w, n, m, r, a, u, d, s, b, t, c, l, f>::Seed(
        Sseq& q, IntegralConstant<unsigned, 2>)
    {
        const unsigned k = 2;
        u32            ar[n * k];
        q.Generate(ar, ar + n * k);
        for (usize i = 0; i < n; ++i)
            m_X[i] = static_cast<ResultType>(
                (ar[2 * i] + ((u64)ar[2 * i + 1] << 32)) & MaxValue);
        const ResultType mask
            = r == DT ? ResultType(~0) : (ResultType(1) << r) - ResultType(1);
        m_X = 0;
        if ((m_X[0] & ~mask) == 0)
        {
            for (usize i = 1; i < n; ++i)
                if (m_X[i] != 0) return;
            m_X[0] = ResultType(1) << (w - 1);
        }
    }

    template <typename Type, usize W, usize N, usize M, usize R, Type A,
              usize U, Type D, usize S, Type B, usize T, Type C, usize L,
              Type F>
    Type MersenneTwisterEngine<Type, W, N, M, R, A, U, D, S, B, T, C, L,
                               F>::operator()()
    {
        const usize      j    = (m_I + 1) % N;
        const ResultType mask = R == NumericLimits<ResultType>::Digits
                                  ? ResultType(~0)
                                  : (ResultType(1) << R) - ResultType(1);

        const ResultType YP   = (m_X[m_I] & ~mask) | (m_X[j] & mask);
        const usize      k    = (m_I + M) % N;
        m_X[m_I]              = m_X[k] ^ ShiftRight<1>(YP) ^ (A * (YP & 1));
        ResultType z          = m_X[m_I] ^ (ShiftRight<U>(m_X[m_I]) & D);
        m_I                   = j;
        z ^= ShiftLeft<S>(z) & B;
        z ^= ShiftLeft<T>(z) & C;
        z ^= ShiftRight<L>(z);
        return z;
    }

    // Equality operator implementation
    template <typename Type, usize W, usize N, usize M, usize R, Type A,
              usize U, Type D, usize S, Type B, usize T, Type C, usize L,
              Type F>
    bool operator==(const MersenneTwisterEngine<Type, W, N, M, R, A, U, D, S, B,
                                                T, C, L, F>& x,
                    const MersenneTwisterEngine<Type, W, N, M, R, A, U, D, S, B,
                                                T, C, L, F>& y)
    {
        if (x.m_I == y.m_I) { return Equal(x.m_X, x.m_X + N, y.m_X); }
        if (x.m_I == 0 || y.m_I == 0)
        {
            usize j = Min(N - x.m_I, N - y.m_I);
            if (!Equal(x.m_X + x.m_I, x.m_X + x.m_I + j, y.m_X + y.m_I))
                return false;
            if (x.m_I == 0) return Equal(x.m_X + j, x.m_X + N, y.m_X);
            return Equal(x.m_X, x.m_X + (N - j), y.m_X + j);
        }
        if (x.m_I < y.m_I)
        {
            usize j = N - y.m_I;
            if (!Equal(x.m_X + x.m_I, x.m_X + (x.m_I + j), y.m_X + y.m_I))
                return false;
            if (!Equal(x.m_X + (x.m_I + j), x.m_X + N, y.m_X)) return false;
            return Equal(x.m_X, x.m_X + x.m_I, y.m_X + (N - (x.m_I + j)));
        }
        usize j = N - x.m_I;
        if (!Equal(y.m_X + y.m_I, y.m_X + (y.m_I + j), x.m_X + x.m_I))
            return false;
        if (!Equal(y.m_X + (y.m_I + j), y.m_X + N, x.m_X)) return false;
        return Equal(y.m_X, y.m_X + y.m_I, x.m_X + (N - (y.m_I + j)));
    }

    template <typename Type, usize WP, usize NP, usize MP, usize RP, Type AP,
              usize UP, Type DP, usize SP, Type BP, usize TP, Type CP, usize LP,
              Type FP>
    inline bool
    operator!=(const MersenneTwisterEngine<Type, WP, NP, MP, RP, AP, UP, DP, SP,
                                           BP, TP, CP, LP, FP>& x,
               const MersenneTwisterEngine<Type, WP, NP, MP, RP, AP, UP, DP, SP,
                                           BP, TP, CP, LP, FP>& y)
    {
        return !(x == y);
    }

    using mt19937
        = MersenneTwisterEngine<uint_fast32_t, 32, 624, 397, 31, 0x9908b0df, 11,
                                0xffffffff, 7, 0x9d2c5680, 15, 0xefc60000, 18,
                                1812433253>;
    using mt19937_64 = MersenneTwisterEngine<
        uint_fast64_t, 64, 312, 156, 31, 0xb5026f5aa96619e9ull, 29,
        0x5555555555555555ull, 17, 0x71d67fffeda60000ull, 37,
        0xfff7eee000000000ull, 43, 6364136223846793005ull>;

    template <typename Gen>
    concept UniformRandomBitGenerate
        = IsInvocableV<Gen&> && IsUnsignedV<InvokeResultType<Gen&>>
       && requires {
              { Gen::Min() } -> SameAs<InvokeResultType<Gen&>>;
              { Gen::Max() } -> SameAs<InvokeResultType<Gen&>>;
              requires BooleanConstant<(Gen::Min() < Gen::Max())>::Value;
          };

    template <typename IntType = int>
    class UniformIntDistribution
    {
        static_assert(IsIntegralV<IntType>,
                      "template argument must be an integral type");

      public:
        typedef IntType ResultType;
        struct ParameterType
        {
            typedef UniformIntDistribution<IntType> distribution_type;

            ParameterType()
                : ParameterType(0)
            {
            }

            explicit ParameterType(IntType a,
                                   IntType b = IntegerTraits<IntType>::Max)
                : m_A(a)
                , m_B(b)
            {
                assert(m_A <= m_B);
            }

            ResultType  A() const { return m_A; }

            ResultType  B() const { return m_B; }

            friend bool operator==(const ParameterType& p1,
                                   const ParameterType& p2)
            {
                return p1.m_A == p2.m_A && p1.m_B == p2.m_B;
            }

            friend bool operator!=(const ParameterType& p1,
                                   const ParameterType& p2)
            {
                return !(p1 == p2);
            }

          private:
            IntType m_A;
            IntType m_B;
        };

      public:
        UniformIntDistribution()
            : UniformIntDistribution(0)
        {
        }

        explicit UniformIntDistribution(IntType a,
                                        IntType b = IntegerTraits<IntType>::Max)
            : m_Parameter(a, b)
        {
        }
        explicit UniformIntDistribution(const ParameterType& p)
            : m_Parameter(p)
        {
        }

        void          Reset() {}
        ResultType    A() const { return m_Parameter.A(); }
        ResultType    B() const { return m_Parameter.B(); }

        ParameterType Parameter() const { return m_Parameter; }
        void Parameter(const ParameterType& param) { m_Parameter = param; }

        ResultType Min() const { return A(); }
        ResultType Max() const { return B(); }

        template <typename UniformRandomBitGenerator>
        ResultType operator()(UniformRandomBitGenerator& urng)
        {
            return operator()(urng, m_Parameter);
        }

        template <typename UniformRandomBitGenerator>
        ResultType operator()(UniformRandomBitGenerator& urng,
                              const ParameterType&       p);

        template <typename ForwardIterator, typename UniformRandomBitGenerator>
        void DoGenerate(ForwardIterator f, ForwardIterator t,
                        UniformRandomBitGenerator& urng)
        {
            DoGenerate(f, t, urng, m_Parameter);
        }

        template <typename ForwardIterator, typename UniformRandomBitGenerator>
        void DoGenerate(ForwardIterator f, ForwardIterator t,
                        UniformRandomBitGenerator& urng, const ParameterType& p)
        {
            GenerateInternal(f, t, urng, p);
        }

        template <typename UniformRandomBitGenerator>
        void DoGenerate(ResultType* f, ResultType* t,
                        UniformRandomBitGenerator& urng, const ParameterType& p)
        {
            GenerateInternal(f, t, urng, p);
        }

        friend bool operator==(const UniformIntDistribution& d1,
                               const UniformIntDistribution& d2)
        {
            return d1.m_Parameter == d2.m_Parameter;
        }

      private:
        template <typename ForwardIterator, typename UniformRandomBitGenerator>
        void          GenerateInternal(ForwardIterator f, ForwardIterator t,
                                       UniformRandomBitGenerator& urng,
                                       const ParameterType&       p);

        ParameterType m_Parameter;

        template <typename W, typename Urbg, typename U>
        static U Nd(Urbg& g, U range)
        {
            using UTraits = IntegerTraits<U>;
            using WTraits = IntegerTraits<W>;
            static_assert(!UTraits::IsSigned, "U must be unsigned");
            static_assert(!WTraits::IsSigned, "W must be unsigned");
            static_assert(WTraits::Digits == (2 * UTraits::Digits),
                          "W must be twice as wide as U");

            W product = W(g()) * W(range);
            U low     = U(product);
            if (low < range)
            {
                U threshold = -range % range;
                while (low < threshold)
                {
                    product = W(g()) * W(range);
                    low     = U(product);
                }
            }
            return product >> UTraits::Digits;
        }
    };

    template <typename IntType>
    template <typename UniformRandomBitGenerator>
    typename UniformIntDistribution<IntType>::ResultType
    UniformIntDistribution<IntType>::operator()(UniformRandomBitGenerator& urng,
                                                const ParameterType& parameter)
    {
        using GResultType = UniformRandomBitGenerator::ResultType;
        using UType       = typename MakeUnsigned<ResultType>::Type;
        using UCType      = typename CommonType<GResultType, UType>::Type;

        constexpr UCType urngMin = UniformRandomBitGenerator::Min();
        constexpr UCType urngMax = UniformRandomBitGenerator::Max();
        static_assert(urngMin < urngMax,
                      "Uniform random bit generator must define min() < max()");
        constexpr UCType urngRange = urngMax - urngMin;

        const UCType     urange = UCType(parameter.B()) - UCType(parameter.A());

        UCType           result;
        if (urngRange > urange)
        {
            const UCType uerange = urange + 1; // urange can be zero

#if defined __UINT64_TYPE__ && defined __UINT32_TYPE__
    #if __SIZEOF_INT128__
            if constexpr (urngRange == __UINT64_MAX__)
            {
                u64 u64erange = uerange;
                result = __extension__ Nd<unsigned __int128>(urng, u64erange);
            }
            else
    #endif
                if constexpr (urngRange == __UINT32_MAX__)
            {
                u32 u32erange = uerange;
                result        = Nd<__UINT64_TYPE__>(urng, u32erange);
            }
            else
#endif
            {
                const UCType scaling = urngRange / uerange;
                const UCType past    = uerange * scaling;
                do result = UCType(urng()) - urngMin;
                while (result >= past);
                result /= scaling;
            }
        }
        else if (urngRange < urange)
        {
            UCType tmp;
            do {
                const UCType uerngrange = urngRange + 1;
                tmp                     = (uerngrange
                       * operator()(urng,
                                    ParameterType(0, urange / uerngrange)));
                result                  = tmp + (UCType(urng()) - urngMin);
            } while (result > urange || result < tmp);
        }
        else result = UCType(urng()) - urngMin;

        return result + parameter.A();
    }

    template <typename IntType>
    template <typename ForwardIterator, typename UniformRandomBitGenerator>
    void UniformIntDistribution<IntType>::GenerateInternal(
        ForwardIterator f, ForwardIterator t, UniformRandomBitGenerator& urng,
        const ParameterType& parameter)
    {
        // requires(ForwardIteratorConcept<ForwardIterator>)
        using GResultType = typename UniformRandomBitGenerator::ResultType;
        using UType       = typename MakeUnsigned<ResultType>::Type;
        using UCType      = typename CommonType<GResultType, UType>::Type;

        static_assert(urng.Min() < urng.Max(),
                      "Uniform random bit generator must define min() < max()");

        constexpr UCType urngMin   = urng.Min();
        constexpr UCType urngMax   = urng.Max();
        constexpr UCType urngRange = urngMax - urngMin;
        const UCType     urange = UCType(parameter.B()) - UCType(parameter.A());

        UCType           result;

        if (urngRange > urange)
        {
            if (Math::IsPowerOfTwo(urngRange + 1)
                && Math::IsPowerOfTwo(urange + 1))
            {
                while (f != t)
                {
                    result = UCType(urng()) - urngMin;
                    *f++   = (result & urange) + parameter.A();
                }
            }
            else
            {
                const UCType uerange = urange + 1; // urange can be zero
                const UCType scaling = urngRange / uerange;
                const UCType past    = uerange * scaling;
                while (f != t)
                {
                    do result = UCType(urng()) - urngMin;
                    while (result >= past);
                    *f++ = result / scaling + parameter.A();
                }
            }
        }
        else if (urngRange < urange)
        {
            UCType tmp;
            while (f != t)
            {
                do {
                    constexpr UCType uerngrange = urngRange + 1;
                    tmp                         = (uerngrange
                           * operator()(urng,
                                        ParameterType(0, urange / uerngrange)));
                    result = tmp + (UCType(urng()) - urngMin);
                } while (result > urange || result < tmp);
                *f++ = result;
            }
        }
        else
            while (f != t) *f++ = UCType(urng()) - urngMin + parameter.A();
    }
}; // namespace Prism::Algorithm

#if PRISM_USE_NAMESPACE != 0
namespace Algorithm = Prism::Algorithm;
#endif
