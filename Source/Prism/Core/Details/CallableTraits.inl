/*
 * Created by v1tr10l7 on 02.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Details/CVRefTraits.inl>
#include <Prism/Core/Details/TransformationTraits.inl>
#include <Prism/Core/Details/TypeProperties.inl>

namespace Prism
{
    template <typename T, typename U>
    struct ReferenceConvertsFromTemporary;

    template <typename Functor, typename... Args>
    inline constexpr typename InvokeResult<Functor, Args...>::Type
    Invoke(Functor&& fn, Args&&... args)
        PM_NOEXCEPT(IsNoThrowInvocable<Functor, Args...>::Value);

    namespace Details
    {
        template <typename T>
        struct InvokeImpl
        {
            template <typename F, typename... Args>
            static auto Call(F&& f, Args&&... args)
                -> decltype(Forward<F>(f)(Forward<Args>(args)...));
        };

        template <typename B, typename MT>
        struct InvokeImpl<MT B::*>
        {
            template <typename T, typename Td = typename Decay<T>::Type,
                      typename
                      = typename EnableIf<IsBaseOf<B, Td>::Value>::Type>
            static auto Get(T&& t) -> T&&;

            template <typename T, typename Td = typename Decay<T>::Type,
                      typename
                      = typename EnableIf<IsReferenceWrapper<Td>::Value>::Type>
            static auto Get(T&& t) -> decltype(t.Get());

            template <typename T, typename Td = typename Decay<T>::Type,
                      typename
                      = typename EnableIf<!IsBaseOf<B, Td>::Value>::Type,
                      typename
                      = typename EnableIf<!IsReferenceWrapper<Td>::Value>::Type>
            static auto Get(T&& t) -> decltype(*Forward<T>(t));
            template <typename T, typename... Args, typename MT1,
                      typename
                      = typename EnableIf<IsFunction<MT1>::Value>::Type>
            static auto Call(MT1 B::* pmf, T&& t, Args&&... args)
                -> decltype((InvokeImpl::Get(Forward<T>(t))
                             .*pmf)(Forward<Args>(args)...));
            template <typename T>
            static auto Call(MT B::* pmd, T&& t)
                -> decltype(InvokeImpl::Get(Forward<T>(t)).*pmd);
        };
        template <typename F, typename... Args,
                  typename Fd = typename Decay<F>::Type>
        auto Invoke(F&& f, Args&&... args)
            -> decltype(InvokeImpl<Fd>::Call(Forward<F>(f),
                                             Forward<Args>(args)...));
        template <typename AlwaysVoid, typename, typename...>
        struct InvokeResult
        {
        };
        template <typename F, typename... Args>
        struct InvokeResult<decltype(void(Details::Invoke(DeclVal<F>(),
                                                          DeclVal<Args>()...))),
                            F, Args...>
        {
            using Type
                = decltype(Details::Invoke(DeclVal<F>(), DeclVal<Args>()...));
        };
    } // namespace Details

    template <typename>
    struct ResultOf;
    template <typename F, typename... ArgTypes>
    struct ResultOf<F(ArgTypes...)>
        : Details::InvokeResult<void, F, ArgTypes...>
    {
    };

    template <typename F, typename... ArgTypes>
    struct InvokeResult : Details::InvokeResult<void, F, ArgTypes...>
    {
    };
    template <typename T>
    using ResulfOfType = typename ResultOf<T>::Type;
    template <typename F, typename... ArgTypes>
    using InvokeResultType = typename InvokeResult<F, ArgTypes...>::Type;

    struct InvokeMemFunRef
    {
    };
    struct InvokeMemFunDeref
    {
    };
    struct InvokeMemObjRef
    {
    };
    struct InvokeMemObjDeref
    {
    };
    struct InvokeOther
    {
    };
    template <typename T, typename U = RemoveCvRefType<T>>
    struct InvUnwrap
    {
        using Type = T;
    };

    template <typename T, typename U>
    struct InvUnwrap<T, ReferenceWrapper<U>>
    {
        using Type = U&;
    };

    template <typename Fn, typename T, typename... Args>
    constexpr bool CallIsNtHelper(InvokeMemFunRef)
    {
        using U = typename InvUnwrap<T>::Type;
        return noexcept((DeclVal<U>().*DeclVal<Fn>())(DeclVal<Args>()...));
    }
    template <typename Fn, typename T, typename... _Args>
    constexpr bool CallIsNtHelper(InvokeMemFunDeref)
    {
        return noexcept(((*DeclVal<T>()).*DeclVal<Fn>())(DeclVal<_Args>()...));
    }

    template <typename Fn, typename T>
    constexpr bool CallIsNtHelper(InvokeMemObjRef)
    {
        using U = typename InvUnwrap<T>::Type;
        return noexcept(DeclVal<U>().*DeclVal<Fn>());
    }

    template <typename Fn, typename T>
    constexpr bool CallIsNtHelper(InvokeMemObjDeref)
    {
        return noexcept((*DeclVal<T>()).*DeclVal<Fn>());
    }

    template <typename Fn, typename... _Args>
    constexpr bool CallIsNtHelper(InvokeOther)
    {
        return noexcept(DeclVal<Fn>()(DeclVal<_Args>()...));
    }

    template <typename Result, typename Fn, typename... Args>
    struct CallIsNt : BooleanConstant<std::__call_is_nt<Fn, Args...>(
                          typename Result::InvokeType{})>
    {
    };

    template <typename Result, typename Ret, bool = IsVoid<Ret>::Value,
              typename = void>
    struct IsInvocableHelper : FalseType
    {
        using NoThrowConv = FalseType;
    };
    template <typename Result, typename Ret>
    struct IsInvocableHelper<Result, Ret, true, VoidType<typename Result::Type>>
        : TrueType
    {
        using NoThrowConv = TrueType;
    };
    template <typename Result, typename Ret>
    struct IsInvocableHelper<Result, Ret, false,
                             VoidType<typename Result::Type>>
    {
      private:
        using ResultType = typename Result::Type;

        static ResultType Get() noexcept;

        template <typename T>
        static void Convert(TypeIdentityType<T>) noexcept;
        template <typename T, bool NoThrow = noexcept(Convert<T>(Get())),
                  typename = decltype(Convert<T>(Get())),
#if PrismHasBuiltin(__reference_converts_from_temporary)
                  bool Dangle
                  = __reference_converts_from_temporary(T, ResultType)
#else
                  bool Dangle = false
#endif
                  >
        static BooleanConstant<NoThrow && !Dangle> Test(int);
        template <typename T, bool = false>
        static FalseType Test(...);

      public:
        using Type        = decltype(Test<Ret, true>(1));
        using NoThrowConv = decltype(Test<Ret>(1));
    };

    template <typename Fn, typename... ArgTypes>
    struct IsNoThrowInvocable
#if PrismHasBuiltin(__is_nothrow_invocable)
        : public BooleanConstant<__is_nothrow_invocable(Fn, ArgTypes...)>
#else
        : Conjunction<IsInvocableHelper<InvokeResult<Fn, ArgTypes...>, void>,
                      CallIsNt<Fn, ArgTypes...>>::Type
#endif
    {
        static_assert(IsCompleteOrUnbounded(TypeIdentity<Fn>{}),
                      "Fn must be a complete class or an unbounded array");
        static_assert(
            (IsCompleteOrUnbounded(TypeIdentity<ArgTypes>{}) && ...),
            "each argument Type must be a complete class or an unbounded "
            "array");
    };
    template <typename Fn, typename... Args>
    inline constexpr bool IsNoThrowInvocableV
        = IsNoThrowInvocable<Fn, Args...>::Value;

    /*template <typename Fn, typename... ArgTypes>
    struct IsInvocable
        : IsInvocableHelper<InvokeResult<Fn, ArgTypes...>, void>::Type
    {
    };
    */
    template <typename F, typename... ArgTypes>
    struct IsInvocable
        : IsInvocableHelper<InvokeResult<F, ArgTypes...>, void>::Type
    {
        static_assert(IsCompleteOrUnbounded(TypeIdentity<F>{}),
                      "Fn must be a complete class or an unbounded array");
        static_assert(
            (IsCompleteOrUnbounded(TypeIdentity<ArgTypes>{}) && ...),
            "each argument Type must be a complete class or an unbounded "
            "array");
    };

    template <typename R, typename Fn, typename... Args>
    struct IsInvocableR : IsInvocableHelper<InvokeResult<Fn, Args...>, R>::Type
    {
    };

    template <typename Result, typename Ret>
    using IsNtInvocableHelper =
        typename IsInvocableHelper<Result, Ret>::NoThrowConv;

    template <typename Result, typename Fn, typename... Args>
    struct CallIsNoThrow : BooleanConstant<CallIsNtHelper<Fn, Args...>(
                               typename Result::InvokeType{})>
    {
    };
    template <typename Fn, typename... Args>
    using CallIsNoThrowHelper
        = CallIsNoThrow<InvokeResult<Fn, Args...>, Fn, Args...>;
    template <typename R, typename Fn, typename... Args>
    struct IsNoThrowInvocableR
        : Conjunction<IsNtInvocableHelper<InvokeResult<Fn, Args...>, R>,
                      CallIsNoThrow<Fn, Args...>>::Type
    {
        static_assert(IsCompleteOrUnbounded(TypeIdentity<Fn>{}),
                      "Fn must be a complete class or an unbounded array");
        static_assert(
            (IsCompleteOrUnbounded(TypeIdentity<Args>{}) && ...),
            "each argument Type must be a complete class or an unbounded "
            "array");
        static_assert(IsCompleteOrUnbounded(TypeIdentity<R>{}),
                      "Ret must be a complete class or an unbounded array");
    };
    template <typename R, typename Fn, typename... Args>
    inline constexpr bool IsNoThrowInvocableRV
        = IsNoThrowInvocableR<R, Fn, Args...>::Value;
}; // namespace Prism
