/*
 * Created by v1tr10l7 on 26.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Core.hpp>

namespace Prism
{
    template <typename... Types>
    struct TupleStorage;
    template <typename T, typename... Types>
    struct TupleStorage<T, Types...>
    {
        constexpr TupleStorage() = default;
        constexpr TupleStorage(T value, Types... rest)
            : Value(Forward<T>(value))
            , Rest(Forward<Types>(rest)...)
        {
        }
        template <typename... OtherTypes>
        constexpr TupleStorage(const TupleStorage<OtherTypes...>& other)
            : Value(other.Value)
            , Rest(other.Rest)
        {
        }
        template <typename... OtherTypes>
        constexpr TupleStorage(TupleStorage<OtherTypes...>&& other)
            : Value(Move(other.Value))
            , Rest(Move(other.Rest))
        {
        }

        T                      Value;
        TupleStorage<Types...> Rest;
    };
    template <>
    struct TupleStorage<>
    {
    };

    template <usize Index, typename... Types>
    struct TupleArgumentType;
    template <usize Index, typename T, typename... Types>
    struct TupleArgumentType<Index, T, Types...>
    {
        using Type = typename TupleArgumentType<Index - 1, Types...>::Type;
    };
    template <typename T, typename... Types>
    struct TupleArgumentType<0, T, Types...>
    {
        using Type = T;
    };
    template <usize Index, typename... Types>
    struct TupleAccessor;
    template <usize Index, typename T, typename... Types>
    struct TupleAccessor<Index, T, Types...>
    {
        static typename TupleArgumentType<Index - 1, Types...>::Type&
        Access(TupleStorage<T, Types...>& storage)
        {
            return TupleAccessor<Index - 1, Types...>::Access(storage.Rest);
        }
        static const typename TupleArgumentType<Index - 1, Types...>::Type&
        Access(const TupleStorage<T, Types...>& storage)
        {
            return TupleAccessor<Index - 1, Types...>::Access(storage.Rest);
        }
    };
    template <typename T, typename... Types>
    struct TupleAccessor<0, T, Types...>
    {
        static T& Access(TupleStorage<T, Types...>& storage)
        {
            return storage.Value;
        }
        static const T& Access(const TupleStorage<T, Types...>& storage)
        {
            return storage.Value;
        }
    };

    template <typename... Types>
    class Tuple
    {
      public:
        Tuple() = default;

        Tuple(Types... args)
            : m_Storage(Forward<Types>(args)...)
        {
        }

        template <typename... UTypes>
        friend class Tuple;

        template <usize Index, typename... UTypes>
        struct TupleIsConstructible
        {
            static constexpr bool Value
                = IsConstructible<
                      typename TupleArgumentType<Index, Types...>::Type,
                      typename TupleArgumentType<Index, UTypes...>::Type>::Value
               && TupleIsConstructible<Index - 1, UTypes...>::Value;
        };

        template <typename... UTypes>
        struct TupleIsConstructible<0, UTypes...>
        {
            static constexpr bool Value = IsConstructible<
                typename TupleArgumentType<0, Types...>::Type,
                typename TupleArgumentType<0, UTypes...>::Type>::Value;
        };

        template <typename... UTypes,
                  typename = EnableIfType<TupleIsConstructible<
                      sizeof...(UTypes) - 1, const UTypes&...>::Value>>
        Tuple(const Tuple<UTypes...>& other)
            : m_Storage(other.m_Storage)
        {
        }

        template <typename... UTypes,
                  typename = EnableIfType<TupleIsConstructible<
                      sizeof...(UTypes) - 1, UTypes&&...>::Value>>
        Tuple(Tuple<UTypes...>&& other)
            : m_Storage(Move(other.m_Storage))
        {
        }

        template <usize Index>
        typename TupleArgumentType<Index, Types...>::Type& Get()
        {
            return TupleAccessor<Index, Types...>::Access(m_Storage);
        }
        template <usize Index>
        const typename TupleArgumentType<Index, Types...>::Type& Get() const
        {
            return TupleAccessor<Index, Types...>::Access(m_Storage);
        }

      private:
        TupleStorage<Types...> m_Storage;
    };

    template <typename T>
    struct TupleSize;
    template <typename T, typename U = RemoveCvType<T>,
              typename = typename EnableIfType<IsSame<T, U>::Value>::Type,
              usize    = TupleSize<T>::Value>
    using EnableIfHasTupleSize = T;

    template <typename T>
    struct TupleSize<const EnableIfHasTupleSize<T>> : public TupleSize<T>
    {
    };

    template <typename T>
    struct TupleSize<volatile EnableIfHasTupleSize<T>> : public TupleSize<T>
    {
    };

    template <typename T>
    struct TupleSize<const volatile EnableIfHasTupleSize<T>>
        : public TupleSize<T>
    {
    };

    template <typename T>
    inline constexpr usize TupleSizeV = TupleSize<T>::Value;
    template <typename... Types>
    struct TupleSize<Tuple<Types...>>
    {
        static constexpr usize Value = sizeof...(Types);
    };

    template <usize Index, typename T>
    struct TupleElement;

    template <usize Index, typename T>
    using TupleElementType = typename TupleElement<Index, T>::Type;

    template <usize Index, typename T>
    struct TupleElement<Index, const T>
    {
        using Type = const TupleElementType<Index, T>;
    };

    template <usize Index, typename T>
    struct TupleElement<Index, volatile T>
    {
        using Type = volatile TupleElementType<Index, T>;
    };

    template <usize Index, typename T>
    struct TupleElement<Index, const volatile T>
    {
        using Type = const volatile TupleElementType<Index, T>;
    };

    template <usize I, typename... Types>
    struct TupleElement<I, Tuple<Types...>>
    {
        using Type = typename TupleArgumentType<I, Types...>::Type;
    };

    template <>
    class Tuple<>
    {
    };

    template <typename... Types>
    Tuple<RemoveReferenceType<Types>...> MakeTuple(Types&&... args)
    {
        return Tuple<RemoveReferenceType<Types>...>(Forward<Types>(args)...);
    }

    template <typename F, typename... Args, usize... I>
    auto Apply(F functor, const Tuple<Args...>& args, IndexSequence<I...>)
    {
        return functor(args.template Get<I>()...);
    }

    template <typename F, typename... Args, usize... I>
    auto Apply(F functor, Tuple<Args...>&& args, IndexSequence<I...>)
    {
        return functor(Move(args.template Get<I>())...);
    }

    // Turns a set of Tuple-like types into a Tuple
    template <usize, typename, typename, usize>
    struct TupleBuilder;

    template <usize Index, typename TupleT, typename... Types, usize Size>
    struct TupleBuilder<Index, Tuple<Types...>, TupleT, Size>
        : TupleBuilder<
              Index + 1,
              Tuple<Types..., typename TupleElement<Index, TupleT>::Type>,
              TupleT, Size>
    {
    };

    template <usize Size, typename TupleT, typename... Types>
    struct TupleBuilder<Size, Tuple<Types...>, TupleT, Size>
    {
        using Type = Tuple<Types...>;
    };

    template <typename T>
    struct DoMakeTuple
        : public TupleBuilder<0, Tuple<>, RemoveReferenceType<T>,
                              TupleSize<RemoveReferenceType<T>>::Value>
    {
    };

    template <typename...>
    struct TupleCombiner;

    template <>
    struct TupleCombiner<>
    {
        using Type = Tuple<>;
    };

    template <typename... Ts>
    struct TupleCombiner<Tuple<Ts...>>
    {
        using Type = Tuple<Ts...>;
    };

    template <typename... T1, typename... T2, typename... Remainder>
    struct TupleCombiner<Tuple<T1...>, Tuple<T2...>, Remainder...>
    {
        using Type =
            typename TupleCombiner<Tuple<T1..., T2...>, Remainder...>::Type;
    };

    template <typename... Tuples>
    struct TupleConcatenationResult
    {
        using Type =
            typename TupleCombiner<typename DoMakeTuple<Tuples>::Type...>::Type;
    };

    template <typename...>
    struct MakeIndicesFromFirst;

    template <>
    struct MakeIndicesFromFirst<>
    {
        using Type = MakeIndexSequence<0>;
    };

    template <typename Tuple, typename... Tuples>
    struct MakeIndicesFromFirst<Tuple, Tuples...>
    {
        using Type = MakeIndexSequence<
            TupleSize<typename RemoveReferenceType<Tuple>::Type>::Value>;
    };

    // Performs the actual concatenation for TupleConcatenate
    template <typename Ret, typename Indices, typename... Tuples>
    struct TupleConcatenator;

    template <typename Ret, usize... Indices, typename Tuple,
              typename... Tuples>
    struct TupleConcatenator<Ret, IndexSequence<Indices...>, Tuple, Tuples...>
    {
        template <typename... Res>
        static constexpr Ret DoConcatenate(Tuple&& tp, Tuples&&... tps,
                                           Res&&... res)
        {
            using Index = typename MakeIndicesFromFirst<Tuples...>::Type;
            using Next  = TupleConcatenator<Ret, Index, Tuples...>;
            return Next::DoConcatenate(Forward<Tuples>(tps)...,
                                       Forward<Res>(res)...,
                                       Move(tp.template Get<Indices>())...);
        }
    };

    template <typename Ret>
    struct TupleConcatenator<Ret, IndexSequence<>>
    {
        template <typename... Res>
        static constexpr Ret DoConcatenate(Res&&... res)
        {
            return Ret(Forward<Res>(res)...);
        }
    };

    template <typename F, typename... Args>
    auto Apply(F functor, const Tuple<Args...>& args)
    {
        return Apply(Move(functor), args, IndexSequenceFor<Args...>());
    }

    template <typename F, typename... Args>
    auto Apply(F functor, Tuple<Args...>&& args)
    {
        return Apply(Move(functor), Move(args), IndexSequenceFor<Args...>());
    }

    template <typename... Tuples,
              typename Ret = typename TupleConcatenationResult<Tuples...>::Type>
    Ret TupleConcatenate(Tuples&&... args)
    {
        using Index        = typename MakeIndicesFromFirst<Tuples...>::Type;
        using Concatenator = TupleConcatenator<Ret, Index, Tuples...>;
        return Concatenator::DoConcat(Forward<Tuples>(args)...);
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Apply;
using Prism::Tuple;
using Prism::TupleConcatenate;
#endif
