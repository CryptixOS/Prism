/*
 * Created by v1tr10l7 on 02.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#include <Prism/Core/Details/CVRefTraits.inl>
#include <Prism/Core/Details/TransformationTraits.inl>
#include <Prism/Core/Details/TypeProperties.inl>

namespace Prism
{
    template <typename T, typename U>
    struct ReferenceConvertsFromTemporary;

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

    namespace Details
    {
        template <typename T>
        struct MemberPointerClass
        {
        };
        template <typename M, typename C>
        struct MemberPointerClass<M C::*>
        {
            using Type = C;
        };

        template <typename... Ts>
        struct FirstTypeOf
        {
        };
        template <typename T, typename... Ts>
        struct FirstTypeOf<T, Ts...>
        {
            using Type = T;
        };

        template <typename T>
        inline constexpr bool IsReferenceWrapperV
            = IsSpecializationOfV<RemoveCvRefType<T>, ReferenceWrapper>;

        /// The object argument of a pointer to member can be used directly
        /// (it is, or derives from, the class) rather than dereferenced.
        template <typename MemberPointer, typename T>
        inline constexpr bool IsDirectMemberAccessV = IsBaseOfV<
            typename MemberPointerClass<RemoveCvType<MemberPointer>>::Type,
            RemoveCvRefType<T>>;

        // INVOKE(f, t1, t2, ..., tN), [func.require]. Every overload has a
        // SFINAE friendly return type and a precise noexcept specification,
        // so InvokeResult and IsNoThrowInvocable can both be derived from it.

        // (1.1) (t1.*f)(t2, ..., tN)
        template <typename F, typename T, typename... Args>
            requires IsMemberFunctionPointerV<DecayType<F>>
                  && IsDirectMemberAccessV<DecayType<F>, T>
        constexpr auto InvokeDispatch(F&& f, T&& t, Args&&... args) PM_NOEXCEPT(
            PM_NOEXCEPT((static_cast<T&&>(t).*f)(static_cast<Args&&>(args)...)))
            -> decltype((static_cast<T&&>(t).*f)(static_cast<Args&&>(args)...))
        {
            return (static_cast<T&&>(t).*f)(static_cast<Args&&>(args)...);
        }
        // (1.2) (t1.get().*f)(t2, ..., tN)
        template <typename F, typename T, typename... Args>
            requires IsMemberFunctionPointerV<DecayType<F>>
                  && (!IsDirectMemberAccessV<DecayType<F>, T>)
                  && IsReferenceWrapperV<T>
        constexpr auto InvokeDispatch(F&& f, T&& t, Args&&... args)
            PM_NOEXCEPT(PM_NOEXCEPT((t.Get().*f)(static_cast<Args&&>(args)...)))
                -> decltype((t.Get().*f)(static_cast<Args&&>(args)...))
        {
            return (t.Get().*f)(static_cast<Args&&>(args)...);
        }
        // (1.3) ((*t1).*f)(t2, ..., tN)
        template <typename F, typename T, typename... Args>
            requires IsMemberFunctionPointerV<DecayType<F>>
                  && (!IsDirectMemberAccessV<DecayType<F>, T>)
                  && (!IsReferenceWrapperV<T>)
        constexpr auto InvokeDispatch(F&& f, T&& t, Args&&... args)
            PM_NOEXCEPT(PM_NOEXCEPT(((*static_cast<T&&>(t)).*f)(
                static_cast<Args&&>(args)...)))
                -> decltype(((*static_cast<T&&>(t)).*f)(
                    static_cast<Args&&>(args)...))
        {
            return ((*static_cast<T&&>(t)).*f)(static_cast<Args&&>(args)...);
        }
        // (1.4) t1.*f
        template <typename F, typename T>
            requires IsMemberObjectPointerV<DecayType<F>>
                  && IsDirectMemberAccessV<DecayType<F>, T>
        constexpr auto InvokeDispatch(F&& f, T&& t)
            PM_NOEXCEPT(PM_NOEXCEPT(static_cast<T&&>(t).*f))
                -> decltype(static_cast<T&&>(t).*f)
        {
            return static_cast<T&&>(t).*f;
        }
        // (1.5) t1.get().*f
        template <typename F, typename T>
            requires IsMemberObjectPointerV<DecayType<F>>
                  && (!IsDirectMemberAccessV<DecayType<F>, T>)
                  && IsReferenceWrapperV<T>
        constexpr auto InvokeDispatch(F&& f, T&& t)
            PM_NOEXCEPT(PM_NOEXCEPT(t.Get().*f)) -> decltype(t.Get().*f)
        {
            return t.Get().*f;
        }
        // (1.6) (*t1).*f
        template <typename F, typename T>
            requires IsMemberObjectPointerV<DecayType<F>>
                  && (!IsDirectMemberAccessV<DecayType<F>, T>)
                  && (!IsReferenceWrapperV<T>)
        constexpr auto InvokeDispatch(F&& f, T&& t)
            PM_NOEXCEPT(PM_NOEXCEPT((*static_cast<T&&>(t)).*f))
                -> decltype((*static_cast<T&&>(t)).*f)
        {
            return (*static_cast<T&&>(t)).*f;
        }
        // (1.7) f(t1, t2, ..., tN)
        template <typename F, typename... Args>
            requires(!IsMemberPointerV<DecayType<F>>)
        constexpr auto InvokeDispatch(F&& f, Args&&... args)
            PM_NOEXCEPT(PM_NOEXCEPT(static_cast<F&&>(f)(
                static_cast<Args&&>(args)...)))
                -> decltype(static_cast<F&&>(f)(static_cast<Args&&>(args)...))
        {
            return static_cast<F&&>(f)(static_cast<Args&&>(args)...);
        }

        template <typename AlwaysVoid, typename, typename...>
        struct InvokeResult
        {
        };
        template <typename F, typename... Args>
        struct InvokeResult<decltype(void(Details::InvokeDispatch(
                                DeclVal<F>(), DeclVal<Args>()...))),
                            F, Args...>
        {
            using Type = decltype(Details::InvokeDispatch(DeclVal<F>(),
                                                          DeclVal<Args>()...));
        };
    } // namespace Details

    /**
     * Tag describing how INVOKE dispatches a call (used by Delegate).
     */
    template <typename F, typename... Args>
    constexpr auto GetInvokeTag(F&&, Args&&...)
    {
        using Fn = DecayType<F>;
        if constexpr (IsMemberFunctionPointerV<Fn> || IsMemberObjectPointerV<Fn>)
        {
            using Object = typename Details::FirstTypeOf<Args...>::Type;
            constexpr bool direct
                = Details::IsDirectMemberAccessV<Fn, Object>
               || Details::IsReferenceWrapperV<Object>;

            if constexpr (IsMemberFunctionPointerV<Fn>)
            {
                if constexpr (direct) return InvokeMemFunRef{};
                else return InvokeMemFunDeref{};
            }
            else if constexpr (direct) return InvokeMemObjRef{};
            else return InvokeMemObjDeref{};
        }
        else return InvokeOther{};
    }

    template <typename F, typename... ArgTypes>
    struct InvokeResult : Details::InvokeResult<void, F, ArgTypes...>
    {
        using InvokeType
            = decltype(GetInvokeTag(DeclVal<F>(), DeclVal<ArgTypes>()...));
    };
    template <typename F, typename... ArgTypes>
    using InvokeResultType = typename InvokeResult<F, ArgTypes...>::Type;

    /// Deprecated in C++17, removed in C++20; kept for compatibility.
    template <typename>
    struct ResultOf;
    template <typename F, typename... ArgTypes>
    struct ResultOf<F(ArgTypes...)> : InvokeResult<F, ArgTypes...>
    {
    };
    template <typename T>
    using ResultOfType = typename ResultOf<T>::Type;
    template <typename T>
    using ResulfOfType = ResultOfType<T>;

    namespace Details
    {
        template <typename Result, typename Ret, bool = IsVoid<Ret>::Value,
                  typename = void>
        struct IsInvocableHelper : FalseType
        {
            using NoThrowConv = FalseType;
        };
        // INVOKE<void>: any result can be discarded.
        template <typename Result, typename Ret>
        struct IsInvocableHelper<Result, Ret, true,
                                 VoidType<typename Result::Type>> : TrueType
        {
            using NoThrowConv = TrueType;
        };
        // INVOKE<R>: the result must implicitly convert to R, and binding a
        // reference R to a temporary is ill-formed (C++23, P2255).
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
            static BooleanConstant<!Dangle> Test(int);
            template <typename T, bool = false>
            static FalseType Test(...);

            template <typename T, bool NoThrow = noexcept(Convert<T>(Get())),
                      typename = decltype(Convert<T>(Get()))>
            static BooleanConstant<NoThrow> TestNoThrow(int);
            template <typename T>
            static FalseType TestNoThrow(...);

          public:
            using Type = decltype(Test<Ret>(1));
            static constexpr bool Value = Type::Value;
            using NoThrowConv
                = BooleanConstant<Value && decltype(TestNoThrow<Ret>(1))::Value>;
        };

        template <typename Fn, typename... Args>
        concept NoThrowInvocable = requires {
            {
                Details::InvokeDispatch(DeclVal<Fn>(), DeclVal<Args>()...)
            } noexcept;
        };
    } // namespace Details

    template <typename Fn, typename... ArgTypes>
    struct IsInvocable
        : BooleanConstant<Details::IsInvocableHelper<
              InvokeResult<Fn, ArgTypes...>, void>::Value>
    {
        static_assert(IsCompleteOrUnbounded(TypeIdentity<Fn>{}),
                      "Fn must be a complete class or an unbounded array");
        static_assert(
            (IsCompleteOrUnbounded(TypeIdentity<ArgTypes>{}) && ...),
            "each argument Type must be a complete class or an unbounded "
            "array");
    };
    template <typename R, typename Fn, typename... ArgTypes>
    struct IsInvocableR
        : BooleanConstant<Details::IsInvocableHelper<
              InvokeResult<Fn, ArgTypes...>, R>::Value>
    {
        static_assert(IsCompleteOrUnbounded(TypeIdentity<Fn>{}),
                      "Fn must be a complete class or an unbounded array");
        static_assert(
            (IsCompleteOrUnbounded(TypeIdentity<ArgTypes>{}) && ...),
            "each argument Type must be a complete class or an unbounded "
            "array");
        static_assert(IsCompleteOrUnbounded(TypeIdentity<R>{}),
                      "R must be a complete class or an unbounded array");
    };

    template <typename Fn, typename... ArgTypes>
    struct IsNoThrowInvocable
#if PrismHasBuiltin(__is_nothrow_invocable)
        : BooleanConstant<__is_nothrow_invocable(Fn, ArgTypes...)>
#else
        : BooleanConstant<Details::NoThrowInvocable<Fn, ArgTypes...>>
#endif
    {
        static_assert(IsCompleteOrUnbounded(TypeIdentity<Fn>{}),
                      "Fn must be a complete class or an unbounded array");
        static_assert(
            (IsCompleteOrUnbounded(TypeIdentity<ArgTypes>{}) && ...),
            "each argument Type must be a complete class or an unbounded "
            "array");
    };
    template <typename R, typename Fn, typename... ArgTypes>
    struct IsNoThrowInvocableR
        : BooleanConstant<Details::NoThrowInvocable<Fn, ArgTypes...>
                          && Details::IsInvocableHelper<
                              InvokeResult<Fn, ArgTypes...>, R>::NoThrowConv::Value>
    {
        static_assert(IsCompleteOrUnbounded(TypeIdentity<Fn>{}),
                      "Fn must be a complete class or an unbounded array");
        static_assert(
            (IsCompleteOrUnbounded(TypeIdentity<ArgTypes>{}) && ...),
            "each argument Type must be a complete class or an unbounded "
            "array");
        static_assert(IsCompleteOrUnbounded(TypeIdentity<R>{}),
                      "R must be a complete class or an unbounded array");
    };

    template <typename Fn, typename... Args>
    inline constexpr bool IsInvocableV = IsInvocable<Fn, Args...>::Value;
    template <typename R, typename Fn, typename... Args>
    inline constexpr bool IsInvocableRV = IsInvocableR<R, Fn, Args...>::Value;
    template <typename Fn, typename... Args>
    inline constexpr bool IsNoThrowInvocableV
        = IsNoThrowInvocable<Fn, Args...>::Value;
    template <typename R, typename Fn, typename... Args>
    inline constexpr bool IsNoThrowInvocableRV
        = IsNoThrowInvocableR<R, Fn, Args...>::Value;

    /**
     * @brief Invokes a callable with the given arguments (std::invoke).
     *
     * Handles function objects, function pointers, pointers to member
     * functions and pointers to data members, applied to objects,
     * references, ReferenceWrapper or pointer-like arguments.
     */
    template <typename Functor, typename... Args>
        requires IsInvocableV<Functor, Args...>
    constexpr InvokeResultType<Functor, Args...>
    Invoke(Functor&& fn, Args&&... args)
        PM_NOEXCEPT(IsNoThrowInvocableV<Functor, Args...>)
    {
        return Details::InvokeDispatch(Forward<Functor>(fn),
                                       Forward<Args>(args)...);
    }
    /**
     * @brief Invokes a callable and converts the result to R (std::invoke_r).
     * When R is void, the result is discarded.
     */
    template <typename R, typename Functor, typename... Args>
        requires IsInvocableRV<R, Functor, Args...>
    constexpr R InvokeR(Functor&& fn, Args&&... args)
        PM_NOEXCEPT(IsNoThrowInvocableRV<R, Functor, Args...>)
    {
        if constexpr (IsVoidV<R>)
            Details::InvokeDispatch(Forward<Functor>(fn),
                                    Forward<Args>(args)...);
        else
            return Details::InvokeDispatch(Forward<Functor>(fn),
                                           Forward<Args>(args)...);
    }
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Invoke;
#endif
