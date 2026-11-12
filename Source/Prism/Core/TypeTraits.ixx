/*
 * Created by v1tr10l7 on 06.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

#include <functional>

export module Prism.Core.TypeTraits;

export extern "C++"
{
    namespace Prism
    {
        template <typename T, T V>
        struct IntegralConstant
        {
            static constexpr T Value = V;
            using ValueType          = T;
            using Type               = IntegralConstant;

            constexpr           operator ValueType() const { return Value; }
            constexpr ValueType operator()() const { return Value; }
        };
        using TrueType  = IntegralConstant<bool, true>;
        using FalseType = IntegralConstant<bool, false>;

        template <bool Boolean>
        using BooleanConstant = IntegralConstant<bool, Boolean>;

        template <typename...>
        using VoidType = void;
        template <typename T>
        struct TypeIdentity
        {
            using Type = T;
        };
        template <typename T>
        using TypeIdentityType = TypeIdentity<T>::Type;

        template <typename T, typename = void>
        struct AddRValueReference;

        // template <typename T>
        // typename AddRValueReference<T>::Type DeclVal() PM_NOEXCEPT
        // {
        //     static_assert(false, "declval not allowed in an evaluated context");
        // }
        //
        template <typename T, typename U = T&&>
        U DoDeclVal(i32);

        template <typename T>
        T DoDeclVal(long);
        template <typename T>
        auto DeclVal() PM_NOEXCEPT -> decltype(DoDeclVal<T>(0));

        template <typename T>
        struct DeclValProtector
        {
            static const bool Stop = false;
        };
        template <typename T>
        auto DeclVal() PM_NOEXCEPT -> decltype(DoDeclVal<T>(0))
        {
            static_assert(DeclValProtector<T>::Stop, "DeclVal() must not be used!");
            return DoDeclVal<T>(0);
        }

        template <typename T, usize = sizeof(T)>
        constexpr TrueType IsCompleteOrUnbounded(TypeIdentity<T>)
        {
            return {};
        }
        template <typename T>
        struct IsArrayUnknownBounds : public FalseType
        {
        };
        template <typename T>
        struct IsArrayUnknownBounds<T[]> : public TrueType
        {
        };

        template <typename T>
        struct IsReference;
        template <typename T>
        struct IsFunction;
        template <typename T>
        struct IsVoid;
        template <typename... Args>
        struct Disjunction;
        template <typename TypeId, typename NestedType = typename TypeId::Type>
        constexpr Disjunction<IsReference<NestedType>, IsFunction<NestedType>,
                              IsVoid<NestedType>,
                              IsArrayUnknownBounds<NestedType>>::Type
        IsCompleteOrUnbounded(TypeId)
        {
            return {};
        }
    }; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
    using Prism::DeclVal;
#endif

    namespace Prism
    {
        template <typename T>
        struct RemoveConst
        {
            using Type = T;
        };
        template <typename T>
        struct RemoveConst<const T>
        {
            using Type = T;
        };
        template <typename T>
        struct RemoveVolatile
        {
            using Type = T;
        };
        template <typename T>
        struct RemoveVolatile<volatile T>
        {
            using Type = T;
        };
        template <typename T>
        struct RemoveCv
        {
            using Type = T;
        };
        template <typename T>
        struct RemoveCv<const T>
        {
            using Type = T;
        };
        template <typename T>
        struct RemoveCv<volatile T>
        {
            using Type = T;
        };
        template <typename T>
        struct RemoveCv<const volatile T>
        {
            using Type = T;
        };

        template <typename T>
        struct AddConst
        {
            using Type = const T;
        };
        template <typename T>
        struct AddVolatile
        {
            using Type = volatile T;
        };
        template <typename T>
        struct AddCv
        {
            using Type = const volatile T;
        };

        template <typename T>
        using RemoveConstType = typename RemoveConst<T>::Type;
        template <typename T>
        using RemoveVolatileType = typename RemoveVolatile<T>::Type;
        template <typename T>
        using RemoveCvType = typename RemoveCv<T>::Type;

        template <typename T>
        using AddConstType = typename AddConst<T>::Type;
        template <typename T>
        using AddVolatileType = typename AddVolatile<T>::Type;
        template <typename T>
        using AddCvType = typename AddCv<T>::Type;

        template <typename T>
        struct RemoveReference
        {
            using Type = T;
        };
        template <typename T>
        struct RemoveReference<T&>
        {
            using Type = T;
        };
        template <typename T>
        struct RemoveReference<T&&>
        {
            using Type = T;
        };
        template <typename T, typename = void>
        struct AddLValueReference
        {
            using Type = T;
        };
        template <typename T>
        struct AddLValueReference<T, VoidType<T&>>
        {
            using Type = T&;
        };
        template <typename T, typename>
        struct AddRValueReference
        {
            using Type = T;
        };
        template <typename T>
        struct AddRValueReference<T, VoidType<T&&>>
        {
            using Type = T&&;
        };

        template <typename T>
        using RemoveReferenceType = typename RemoveReference<T>::Type;
        template <typename T>
        using AddLValueReferenceType = typename AddLValueReference<T>::Type;
        template <typename T>
        using AddRValueReferenceType = typename AddRValueReference<T>::Type;

        template <typename T>
        struct RemoveCvRef
        {
            using Type = RemoveCvType<RemoveReferenceType<T>>;
        };
        template <typename T>
        using RemoveCvRefType = RemoveCvRef<T>::Type;

        template <typename T>
        struct IsLValueReference : FalseType
        {
        };
        template <typename T>
        struct IsLValueReference<T&> : TrueType
        {
        };
        template <typename T>
        struct IsRValueReference : FalseType
        {
        };
        template <typename T>
        struct IsRValueReference<T&&> : TrueType
        {
        };
        template <typename T>
        struct IsReference : FalseType
        {
        };
        template <typename T>
        struct IsReference<T&> : TrueType
        {
        };
        template <typename T>
        struct IsReference<T&&> : TrueType
        {
        };

        template <typename T>
        struct IsPod
            : public IntegralConstant<bool, __is_pod(T) || IsVoid<T>::Value>
        {
        };

        template <typename T>
        struct HasTrivialDestructor : public IntegralConstant<bool, IsPod<T>::Value>
        {
        };

        template <typename T>
        inline constexpr bool IsLValueReferenceV = IsLValueReference<T>::Value;
        template <typename T>
        inline constexpr bool IsRValueReferenceV = IsRValueReference<T>::Value;
        template <typename T>
        inline constexpr bool IsReferenceV = IsReference<T>::Value;
        template <typename T>
        inline constexpr bool IsPodV = IsPod<T>::Value;
        template <typename T>
        inline constexpr bool HasTrivialDestructorV
            = HasTrivialDestructor<T>::Value;

        template <typename T>
        using RemoveConstType = typename RemoveConst<T>::Type;
    }; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    using Prism::RemoveConstType;
    using Prism::RemoveCvRefType;
    using Prism::RemoveReference;
    using Prism::RemoveReferenceType;
#endif

    namespace Prism
    {
        template <bool B, typename T, typename F>
        struct Conditional
        {
            using Type = T;
        };
        template <typename T, typename F>
        struct Conditional<false, T, F>
        {
            using Type = F;
        };
        template <bool B, typename T, typename F>
        using ConditionalType = typename Conditional<B, T, F>::Type;

        template <bool B, typename T = void>
        struct EnableIf
        {
        };
        template <typename T>
        struct EnableIf<true, T>
        {
            using Type = T;
        };
        template <bool B, typename T = void>
        using EnableIfType = typename EnableIf<B, T>::Type;

        template <typename T, typename U>
        struct IsSame : FalseType
        {
        };
        template <typename T>
        struct IsSame<T, T> : TrueType
        {
        };
        template <typename T, typename U>
        inline constexpr bool IsSameV = IsSame<T, U>::Value;

        template <typename... Args>
        struct Conjunction : TrueType
        {
        };
        template <typename B1>
        struct Conjunction<B1> : B1
        {
        };
        template <typename B1, typename... Bn>
        struct Conjunction<B1, Bn...>
            : ConditionalType<bool(B1::Value), Conjunction<Bn...>, B1>
        {
        };
        template <typename... Args>
        struct Disjunction : FalseType
        {
        };
        template <typename B1>
        struct Disjunction<B1> : B1
        {
        };
        template <typename B1, typename... Bn>
        struct Disjunction<B1, Bn...>
            : ConditionalType<bool(B1::Value), B1, Disjunction<Bn...>>
        {
        };
        template <typename B>
        struct Negation : BooleanConstant<!bool(B::Value)>
        {
        };
        template <typename T>
        constexpr RemoveReferenceType<T>&& Move(T&& object) PM_NOEXCEPT
        {
            return static_cast<RemoveReferenceType<T>&&>(object);
        }

        // FIXME(v1tr10l7): Why is it here?
        template <typename T>
        constexpr T&& Forward(RemoveReferenceType<T>& value) PM_NOEXCEPT
        {
            return static_cast<T&&>(value);
        }
        template <typename T>
        constexpr T&& Forward(RemoveReferenceType<T>&& value) PM_NOEXCEPT
        {
            static_assert(!IsLValueReferenceV<T>,
                          "Don't forward an rvalue as an lvalue");
            return static_cast<T&&>(value);
        }
        template <typename... Args>
        inline constexpr bool ConjunctionV = Conjunction<Args...>::Value;
        template <typename... Args>
        inline constexpr bool DisjunctionV = Disjunction<Args...>::Value;
        template <typename T>
        inline constexpr bool NegationV = Negation<T>::Value;
    }; // namespace Prism

    namespace Prism
    {
        template <typename T>
        struct IsVoid : IsSame<void, typename RemoveCv<T>::Type>
        {
        };
        template <typename T>
        struct IsNullPointer : IsSame<NullType, typename RemoveCv<T>::Type>
        {
        };
        template <typename T>
            struct IsIntegral : BooleanConstant < requires(T t, T* p, void (*f)(T))
        {
            reinterpret_cast<T>(t);
            f(0);
            p + t;
        }>{};
        template <typename>
        struct IsFloatingPointHelper : public FalseType
        {
        };
        template <>
        struct IsFloatingPointHelper<float> : public TrueType
        {
        };
        template <>
        struct IsFloatingPointHelper<double> : public TrueType
        {
        };
        template <>
        struct IsFloatingPointHelper<long double> : public TrueType
        {
        };
#ifdef __STDCPP_FLOAT16_T__
        template <>
        struct IsFloatingPointHelper<_Float16> : public TrueType
        {
        };
#endif
#ifdef __STDCPP_FLOAT32_T__
        template <>
        struct IsFloatingPointHelper<_Float32> : public TrueType
        {
        };
#endif
#ifdef __STDCPP_FLOAT64_T__
        template <>
        struct IsFloatingPointHelper<_Float64> : public TrueType
        {
        };
#endif

#ifdef __STDCPP_FLOAT128_T__
        template <>
        struct IsFloatingPointHelper<_Float128> : public TrueType
        {
        };
#endif
#ifdef __STDCPP_BFLOAT16_T__
        template <>
        struct IsFloatingPointHelper<__gnu_cxx::__bfloat16_t> : public TrueType
        {
        };
#endif

#if !defined(__STRICT_ANSI__) && defined(_GLIBCXX_USE_FLOAT128)
        template <>
        struct IsFloatingPointHelper<__float128> : public TrueType
        {
        };
#endif

        template <typename T>
        struct IsFloatingPoint
            : public IsFloatingPointHelper<typename RemoveCv<T>::Type>
        {
        };
        template <typename T>
        struct IsArray : FalseType
        {
        };
        template <typename T>
        struct IsArray<T[]> : TrueType
        {
        };
        template <typename T, usize N>
        struct IsArray<T[N]> : TrueType
        {
        };
        template <typename T>
        struct IsEnum : public BooleanConstant<__is_enum(T)>
        {
        };

        template <typename T>
        struct IsUnion : public BooleanConstant<__is_union(T)>
        {
        };

        namespace Details
        {
            template <typename T>
            IntegralConstant<bool, !IsUnion<T>::Value> Test(i32 T::*);

            template <typename>
            FalseType Test(...);
        } // namespace Details
        template <typename T>
        struct IsClass : decltype(Details::Test<T>(nullptr))
        {
        };

        template <typename T>
        struct IsReference;
        template <typename T>
        struct IsConst;
        template <typename T>
        struct IsFunction : IntegralConstant<bool, !IsConst<const T>::Value
                                                       && !IsReference<T>::Value>
        {
        };
        template <typename T>
        struct IsPointer : FalseType
        {
        };
        template <typename T>
        struct IsPointer<T*> : TrueType
        {
        };
        template <typename T>
        struct IsPointer<T* const> : TrueType
        {
        };
        template <typename T>
        struct IsPointer<T* volatile> : TrueType
        {
        };
        template <typename T>
        struct IsPointer<T* const volatile> : TrueType
        {
        };

        template <typename T>
        struct IsMemberPointer;
        template <typename T>
        struct IsMemberFunctionPointer;

        template <typename T>
        struct IsMemberObjectPointer
            : IntegralConstant<bool, IsMemberPointer<T>::Value
                                         && !IsMemberFunctionPointer<T>::Value>
        {
        };
        template <typename T>
        struct IsMemberFunctionPointerHelper : FalseType
        {
        };
        template <typename T, typename U>
        struct IsMemberFunctionPointerHelper<T U::*> : IsFunction<T>
        {
        };
        template <typename T>
        struct IsMemberFunctionPointer
            : IsMemberFunctionPointerHelper<typename RemoveCv<T>::Type>
        {
        };

        template <typename T>
        struct RemovePointer;
        template <typename T>
        struct IsFunctionPointer
            : Conjunction<IsPointer<T>, IsFunction<typename RemovePointer<T>::Type>>
        {
        };

        template <typename T>
        struct IsFunctionReference : FalseType
        {
        };

        template <typename T>
            requires IsReferenceV<T> && IsFunction<RemoveReferenceType<T>>::Value
        struct IsFunctionReference<T> : TrueType
        {
        };

        template <typename T>
        struct IsArithmetic;
        template <typename T>
        struct IsFundamental
            : IntegralConstant<
                  bool, IsArithmetic<T>::Value || IsVoid<T>::Value
                            || IsSame<NullType, typename RemoveCv<T>::Type>::Value>
        {
        };
        template <typename T>
        struct IsArithmetic
            : IntegralConstant<bool,
                               IsIntegral<T>::Value || IsFloatingPoint<T>::Value>
        {
        };
        template <typename T>
        struct IsScalar
            : IntegralConstant<bool, IsArithmetic<T>::Value || IsEnum<T>::Value
                                         || IsPointer<T>::Value
                                         || IsMemberPointer<T>::Value
                                         || IsNullPointer<T>::Value>
        {
        };
        template <typename T>
        struct IsObject
            : IntegralConstant<bool, IsScalar<T>::Value || IsArray<T>::Value
                                         || IsUnion<T>::Value || IsClass<T>::Value>
        {
        };
        template <typename T>
        struct IsCompound : IntegralConstant<bool, !IsFundamental<T>::Value>
        {
        };
        template <typename T>
        struct IsMemberPointerHelper : FalseType
        {
        };
        template <typename T, typename U>
        struct IsMemberPointerHelper<T U::*> : TrueType
        {
        };
        template <typename T>
        struct IsMemberPointer : IsMemberPointerHelper<typename RemoveCv<T>::Type>
        {
        };

        template <typename T>
        struct IsConst : FalseType
        {
        };
        template <typename T>
        struct IsConst<const T> : TrueType
        {
        };
        template <typename T>
        struct IsVolatile : FalseType
        {
        };
        template <typename T>
        struct IsVolatile<volatile T> : TrueType
        {
        };
        template <typename T>
        struct IsTriviallyCopyable
            : public BooleanConstant<__is_trivially_copyable(T)>
        {
        };
        /// Trivially copyable and trivially default constructible.
        template <typename T>
        struct IsTrivial
            : public BooleanConstant<__is_trivially_copyable(T)
                                     && __is_trivially_constructible(T)>
        {
        };
        template <typename T>
        struct IsStandardLayout : public BooleanConstant<__is_standard_layout(T)>
        {
        };
        template <typename T, template <typename...> typename U>
        struct IsSpecializationOf : public BooleanConstant<false>
        {
        };
        template <template <typename...> typename U, typename... Us>
        struct IsSpecializationOf<U<Us...>, U> : public BooleanConstant<true>
        {
        };

        template <typename T>
        struct RemoveAllExtents;

        template <typename T>
        struct HasUniqueObjectRepresentations
            : BooleanConstant<__has_unique_object_representations(
                  RemoveCvType<typename RemoveAllExtents<T>::Type>)>
        {
        };
        template <typename T>
        struct IsEmpty : BooleanConstant<__is_empty(T)>
        {
        };
        template <typename T>
        struct IsPolymorphic : BooleanConstant<__is_polymorphic(T)>
        {
        };
        template <typename T>
        struct IsAbstract : BooleanConstant<__is_abstract(T)>
        {
        };
        template <typename T>
        struct IsFinal : BooleanConstant<__is_final(T)>
        {
        };
        template <typename T>
        struct IsAggregate : BooleanConstant<__is_aggregate(RemoveCvType<T>)>
        {
        };
#if PrismHasBuiltin(__builtin_is_implicit_lifetime)
    #define PRISM_HAS_IS_IMPLICIT_LIFETIME 1
        /// Objects of type T may be implicitly created (C++23).
        template <typename T>
        struct IsImplicitLifetime
            : BooleanConstant<__builtin_is_implicit_lifetime(T)>
        {
        };
#endif

        template <typename T, bool = IsArithmetic<T>::Value>
        struct IsSignedHelper : FalseType
        {
        };
        template <typename T>
        struct IsSignedHelper<T, true> : BooleanConstant<T(-1) < T(0)>
        {
        };

        template <typename T>
        struct IsSigned : IsSignedHelper<T>::Type
        {
        };
        template <typename T>
        struct IsUnsigned
            : Conjunction<IsArithmetic<T>, Negation<IsSigned<T>>>::Type
        {
        };
        template <typename T>
        struct IsBoundedArray : FalseType
        {
        };
        template <typename T, usize N>
        struct IsBoundedArray<T[N]> : TrueType
        {
        };
        template <typename T>
        struct IsUnboundedArray : FalseType
        {
        };
        template <typename T>
        struct IsUnboundedArray<T[]> : TrueType
        {
        };
#if PrismHasBuiltin(__is_scoped_enum)
        template <typename T>
        struct IsScopedEnum : BooleanConstant<__is_scoped_enum(T)>
        {
        };
#else
        // A scoped enumeration doesn't implicitly convert to its underlying type.
        template <typename T, bool = __is_enum(T)>
        struct IsScopedEnumHelper : FalseType
        {
        };
        template <typename T>
        struct IsScopedEnumHelper<T, true>
            : BooleanConstant<!requires(void (*f)(__underlying_type(T)), T t) {
                  f(t);
              }>
        {
        };
        template <typename T>
        struct IsScopedEnum : IsScopedEnumHelper<T>
        {
        };
#endif

        namespace Details
        {
            template <typename B>
            TrueType TestPointerConversion(const volatile B*);
            template <typename>
            FalseType TestPointerConversion(const volatile void*);

            template <typename B, typename D>
            auto TestIsBaseOf(int)
                -> decltype(TestPointerConversion<B>(static_cast<D*>(nullptr)));
            template <typename, typename>
            auto TestIsBaseOf(...) -> TrueType;
            template <typename T>
            auto TestReturnable(int)
                -> decltype(void(static_cast<T (*)()>(nullptr)), TrueType{});
            template <typename>
            auto TestReturnable(...) -> FalseType;

            template <typename From, typename To>
            auto TestImplicitlyConvertible(int)
                -> decltype(void(DeclVal<void (&)(To)>()(DeclVal<From>())),
                            TrueType{});
            template <typename, typename>
            auto TestImplicitlyConvertible(...) -> FalseType;
        } // namespace Details

        template <typename Base, typename Derived>
        struct IsBaseOf : BooleanConstant<__is_base_of(Base, Derived)>
        {
        };
#if PrismHasBuiltin(__builtin_is_virtual_base_of)
    #define PRISM_HAS_IS_VIRTUAL_BASE_OF 1
        template <typename Base, typename Derived>
        struct IsVirtualBaseOf
            : BooleanConstant<__builtin_is_virtual_base_of(Base, Derived)>
        {
        };
#endif
#if PrismHasBuiltin(__is_pointer_interconvertible_base_of)
    #define PRISM_HAS_IS_POINTER_INTERCONVERTIBLE_BASE_OF 1
        template <typename Base, typename Derived>
        struct IsPointerInterconvertibleBaseOf
            : BooleanConstant<__is_pointer_interconvertible_base_of(Base, Derived)>
        {
        };
#endif
        template <typename From, typename To>
        struct IsConvertible
            : IntegralConstant<
                  bool, (decltype(Details::TestReturnable<To>(0))::Value
                         && decltype(Details::TestImplicitlyConvertible<From, To>(
                             0))::Value)
                            || (IsVoid<From>::Value && IsVoid<To>::Value)>
        {
        };

        template <typename From, typename To,
                  bool
                  = Disjunction<IsVoid<From>, IsFunction<To>, IsArray<To>>::Value>
        struct IsNtConvertibleHelper : IsVoid<To>
        {
        };

        template <typename From, typename To>
        class IsNtConvertibleHelper<From, To, false>
        {
            template <typename To1>
            static void TestAux(To1) noexcept;

            template <typename From1, typename To1>
            static BooleanConstant<noexcept(TestAux<To1>(DeclVal<From1>()))>
            Test(int);

            template <typename, typename>
            static FalseType Test(...);

          public:
            using Type = decltype(Test<From, To>(0));
        };
        template <typename From, typename To>
        struct IsNoThrowConvertible : public IsNtConvertibleHelper<From, To>::Type
        {
        };
    /*
        template <typename From, typename To>
        struct IsNoThrowConvertible
            : BooleanConstant<IsNoThrowConvertibleV(From, To)>
        {
        };
        */
#if PrismHasBuiltin(__is_layout_compatible)
    #define PRISM_HAS_IS_LAYOUT_COMPATIBLE 1
        template <typename T, typename U>
        struct IsLayoutCompatible : BooleanConstant<__is_layout_compatible(T, U)>
        {
        };
#endif

        template <typename T>
        struct AlignmentOf : IntegralConstant<usize, __alignof__(T)>
        {
        };
        template <typename T>
        struct Rank : IntegralConstant<usize, 0>
        {
        };
        template <typename T>
        struct Rank<T[]> : IntegralConstant<usize, Rank<T>::Value>
        {
        };
        template <typename T, usize N>
        struct Rank<T[N]> : IntegralConstant<usize, Rank<T>::Value + 1>
        {
        };
        template <typename T>
        constexpr usize RankV = Rank<T>::Value;

        template <typename T, usize N = 0>
        struct Extent : IntegralConstant<usize, 0>
        {
        };
        template <typename T>
        struct Extent<T[], 0> : IntegralConstant<usize, 0>
        {
        };
        template <typename T, usize N>
        struct Extent<T[], N> : Extent<T, N - 1>
        {
        };
        template <typename T, usize I>
        struct Extent<T[I], 0> : IntegralConstant<usize, I>
        {
        };
        template <typename T, usize I, usize N>
        struct Extent<T[I], N> : Extent<T, N - 1>
        {
        };

        template <typename T>
        inline constexpr bool IsVoidV = IsVoid<T>::Value;
        template <typename T>
        inline constexpr bool IsNullPointerV = IsNullPointer<T>::Value;
        template <typename T>
        inline constexpr bool IsIntegralV = IsIntegral<T>::Value;
        template <typename T>
        inline constexpr bool IsFloatingPointV = IsFloatingPoint<T>::Value;
        template <typename T>
        inline constexpr bool IsArrayV = IsArray<T>::Value;
        template <typename T>
        inline constexpr bool IsEnumV = IsEnum<T>::Value;
        template <typename T>
        inline constexpr bool IsUnionV = IsUnion<T>::Value;
        template <typename T>
        inline constexpr bool IsClassV = IsClass<T>::Value;
        template <typename T>
        inline constexpr bool IsFunctionV = IsFunction<T>::Value;
        template <typename T>
        inline constexpr bool IsPointerV = IsPointer<T>::Value;
        template <typename T>
        inline constexpr bool IsMemberObjectPointerV
            = IsMemberObjectPointer<T>::Value;
        template <typename T>
        inline constexpr bool IsMemberFunctionPointerV
            = IsMemberFunctionPointer<T>::Value;
        template <typename T>
        inline constexpr bool IsFunctionPointerV = IsFunctionPointer<T>::Value;
        template <typename T>
        inline constexpr bool IsFunctionReferenceV = IsFunctionReference<T>::Value;

        template <typename T>
        inline constexpr bool IsFundamentalV = IsFundamental<T>::Value;
        template <typename T>
        inline constexpr bool IsArithmeticV = IsArithmetic<T>::Value;
        template <typename T>
        inline constexpr bool IsScalarV = IsScalar<T>::Value;
        template <typename T>
        inline constexpr bool IsObjectV = IsObject<T>::Value;
        template <typename T>
        inline constexpr bool IsCompoundV = IsCompound<T>::Value;
        template <typename T>
        inline constexpr bool IsMemberPointerV = IsMemberPointer<T>::Value;

        template <typename T>
        inline constexpr bool IsConstV = IsConst<T>::Value;
        template <typename T>
        inline constexpr bool IsVolatileV = IsVolatile<T>::Value;
        template <typename T>
        inline constexpr bool IsTriviallyCopyableV = IsTriviallyCopyable<T>::Value;
        template <typename T>
        inline constexpr bool IsStandardLayoutV = IsStandardLayout<T>::Value;
        template <typename T, template <typename...> typename U>
        inline constexpr bool IsSpecializationOfV
            = IsSpecializationOf<T, U>::Value;
        template <typename T>
        constexpr bool HasUniqueObjectRepresentationsV
            = HasUniqueObjectRepresentations<T>::Value;
        template <typename T>
        constexpr bool IsEmptyV = IsEmpty<T>::Value;
        template <typename T>
        constexpr bool IsPolymorphicV = IsPolymorphic<T>::Value;
        template <typename T>
        constexpr bool IsAbstractV = IsAbstract<T>::Value;
        template <typename T>
        constexpr bool IsFinalV = IsFinal<T>::Value;
        template <typename T>
        constexpr bool IsAggregateV = IsAggregate<T>::Value;
#ifdef PRISM_HAS_IS_IMPLICIT_LIFETIME
        template <typename T>
        inline constexpr bool IsImplicitLifetimeV = IsImplicitLifetime<T>::Value;
#endif
        template <typename T>
        inline constexpr bool IsTrivialV = IsTrivial<T>::Value;
        template <typename T>
        constexpr bool IsSignedV = IsSigned<T>::Value;
        template <typename T>
        constexpr bool IsUnsignedV = IsUnsigned<T>::Value;
        template <typename T>
        constexpr bool IsBoundedArrayV = IsBoundedArray<T>::Value;
        template <typename T>
        constexpr bool IsUnboundedArrayV = IsUnboundedArray<T>::Value;
        template <typename T>
        constexpr bool IsScopedEnumV = IsScopedEnum<T>::Value;

        template <typename Base, typename Derived>
        constexpr bool IsBaseOfV = IsBaseOf<Base, Derived>::Value;
#ifdef PRISM_HAS_IS_VIRTUAL_BASE_OF
        template <typename Base, typename Derived>
        constexpr bool IsVirtualBaseOfV = IsVirtualBaseOf<Base, Derived>::Value;
#endif
#ifdef PRISM_HAS_IS_POINTER_INTERCONVERTIBLE_BASE_OF
        template <typename Base, typename Derived>
        constexpr bool IsPointerInterconvertibleBaseOfV
            = IsPointerInterconvertibleBaseOf<Base, Derived>::Value;
#endif
        template <typename From, typename To>
        constexpr bool IsConvertibleV = IsConvertible<From, To>::Value;
        template <typename From, typename To>
        constexpr bool IsNoThrowConvertibleV
            = IsNoThrowConvertible<From, To>::Value;
#ifdef PRISM_HAS_IS_LAYOUT_COMPATIBLE
        template <typename T, typename U>
        constexpr bool IsLayoutCompatibleV = IsLayoutCompatible<T, U>::Value;
#endif

        template <typename T, usize N = 0>
        constexpr usize ExtentV = Extent<T, N>::Value;
    }; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    using Prism::IsAbstractV;
    using Prism::IsAggregateV;
    using Prism::IsArithmeticV;
    using Prism::IsArrayV;
    using Prism::IsClassV;
    using Prism::IsCompoundV;
    using Prism::IsConstV;
    using Prism::IsConvertibleV;
    using Prism::IsEmptyV;
    using Prism::IsEnumV;
    using Prism::IsFinalV;
    using Prism::IsFunctionV;
    using Prism::IsIntegralV;
    using Prism::IsMemberFunctionPointerV;
    using Prism::IsMemberPointerV;
    using Prism::IsObjectV;
    using Prism::IsPointerV;
    using Prism::IsPolymorphicV;
    using Prism::IsReferenceV;
    using Prism::IsScalarV;
    using Prism::IsSignedV;
    using Prism::IsSpecializationOfV;
    using Prism::IsStandardLayoutV;
    using Prism::IsTriviallyCopyableV;
    using Prism::IsUnionV;
    using Prism::IsUnsignedV;
    using Prism::IsVoidV;
    using Prism::IsVolatileV;
#endif

    namespace Prism
    {
        template <typename T>
        struct RemovePointer
        {
            using Type = T;
        };
        template <typename T>
        struct RemovePointer<T*>
        {
            using Type = T;
        };
        template <typename T>
        struct RemovePointer<T* const>
        {
            using Type = T;
        };
        template <typename T>
        struct RemovePointer<T* volatile>
        {
            using Type = T;
        };
        template <typename T>
        struct RemovePointer<T* const volatile>
        {
            using Type = T;
        };
        template <typename T>
        using RemovePointerType = typename RemovePointer<T>::Type;
        namespace Details
        {
            template <typename T>
            struct TypeIdentity
            {
                using Type = T;
            };

            template <typename T>
            auto TryAddPointer(int)
                -> TypeIdentity<typename RemoveReference<T>::Type*>;
            template <typename T>
            auto TryAddPointer(...) -> TypeIdentity<T>;
        } // namespace Details
        template <typename T>
        struct AddPointer : decltype(Details::TryAddPointer<T>(0))
        {
        };
        template <typename T>
        using AddPointerType = typename AddPointer<T>::Type;
        template <typename T>
        using AddPointerType = typename AddPointer<T>::Type;
        template <typename T>
        struct RemoveExtent
        {
            using Type = T;
        };
        template <typename T>
        struct RemoveExtent<T[]>
        {
            using Type = T;
        };
        template <typename T, usize N>
        struct RemoveExtent<T[N]>
        {
            using Type = T;
        };
        template <typename T>
        using RemoveExtentType = typename RemoveExtent<T>::Type;

#if PrismHasBuiltin(__remove_all_extents)
        template <typename T>
        struct RemoveAllExtents
        {
            using Type = __remove_all_extents(T);
        };
#else
        template <typename T>
        struct RemoveAllExtents
        {
            using Type = T;
        };

        template <typename T, usize Size>
        struct RemoveAllExtents<T[Size]>
        {
            using Type = typename RemoveAllExtents<T>::Type;
        };

        template <typename T>
        struct RemoveAllExtents<T[]>
        {
            using Type = typename RemoveAllExtents<T>::Type;
        };
#endif

        template <typename T>
        struct Decay
        {
          private:
            using U = typename RemoveReference<T>::Type;

          public:
            using Type = typename Conditional<
                IsArray<U>::Value,
                typename AddPointer<typename RemoveExtent<U>::Type>::Type,
                typename Conditional<IsFunction<U>::Value,
                                     typename AddPointer<U>::Type,
                                     typename RemoveCv<U>::Type>::Type>::Type;
        };

        namespace Details
        {
            template <typename...>
            using VoidType = void;

            template <typename T, typename U>
            using ConditionalResultType
                = decltype(false ? DeclVal<T>() : DeclVal<U>());

            template <typename, typename, typename = void>
            struct DecayConditionalResult
            {
            };
            template <typename T, typename U>
            struct DecayConditionalResult<T, U,
                                          VoidType<ConditionalResultType<T, U>>>
                : Decay<ConditionalResultType<T, U>>
            {
            };
            template <typename T, typename U, typename = void>
            struct CommonType2 : DecayConditionalResult<const T&, const U&>
            {
            };

            template <typename T, typename U>
            struct CommonType2<T, U, VoidType<ConditionalResultType<T, U>>>
                : DecayConditionalResult<T, U>
            {
            };
        }; // namespace Details

        // TODO(v1tr10l7): Specializations for TupleLike, BasicConstIterator
        template <typename... Args>
        struct CommonType
        {
        };
        template <typename T>
        struct CommonType<T> : CommonType<T, T>
        {
        };
        template <typename T, typename U>
        struct CommonType<T, U>
            : Conditional<IsSame<T, typename Decay<T>::Type>::Value
                              && IsSame<U, typename Decay<U>::Type>::Value,
                          Details::CommonType2<T, U>,
                          CommonType<typename Decay<T>::Type,
                                     typename Decay<U>::Type>>::Type
        {
        };
        namespace Details
        {
            template <typename AlwaysVoid, typename T, typename U,
                      typename... Other>
            struct CommonTypeMulti
            {
            };
            template <typename T, typename U, typename... Other>
            struct CommonTypeMulti<VoidType<typename CommonType<T, U>::Type>, T, U,
                                   Other...>
                : CommonType<typename CommonType<T, U>::Type, Other...>
            {
            };
        }; // namespace Details

        template <typename T, typename U, typename... Other>
        struct CommonType<T, U, Other...>
            : Details::CommonTypeMulti<void, T, U, Other...>
        {
        };

        template <typename Unqualified, bool IsConst, bool IsVolatile>
        struct CvSelector;

        template <typename Unqualified>
        struct CvSelector<Unqualified, false, false>
        {
            using Type = Unqualified;
        };

        template <typename Unqualified>
        struct CvSelector<Unqualified, false, true>
        {
            using Type = volatile Unqualified;
        };

        template <typename Unqualified>
        struct CvSelector<Unqualified, true, false>
        {
            using Type = const Unqualified;
        };

        template <typename Unqualified>
        struct CvSelector<Unqualified, true, true>
        {
            using Type = const volatile Unqualified;
        };

        template <typename Qualified, typename Unqualified,
                  bool IsConst    = IsConst<Qualified>::Value,
                  bool IsVolatile = IsVolatile<Qualified>::Value>
        class MatchCvQualifiers
        {
            using Match = CvSelector<Unqualified, IsConst, IsVolatile>;

          public:
            using Type = typename Match::Type;
        };

        template <typename From, typename To>
        using CopyCv = typename MatchCvQualifiers<From, To>::Type;

        template <typename X, typename Y>
        using ConditionResult
            = decltype(false ? DeclVal<X (&)()>()() : DeclVal<Y (&)()>()());

        template <typename A, typename B, typename = void>
        struct CommonRefImpl
        {
        };

        template <typename A, typename B>
        using CommonRef = typename CommonRefImpl<A, B>::Type;

        template <typename X, typename Y>
        using ConditionResultCvRef = ConditionResult<CopyCv<X, Y>&, CopyCv<Y, X>&>;

        template <typename X, typename Y>
        struct CommonRefImpl<X&, Y&, VoidType<ConditionResultCvRef<X, Y>>>
            : EnableIf<IsReferenceV<ConditionResultCvRef<X, Y>>,
                       ConditionResultCvRef<X, Y>>
        {
        };

        template <typename X, typename Y>
        using CommonRefC = RemoveReferenceType<CommonRef<X&, Y&>>&&;

        template <typename X, typename Y>
        struct CommonRefImpl<
            X&&, Y&&,
            EnableIfType<IsConvertible<X&&, CommonRefC<X, Y>>::Value
                         && IsConvertible<Y&&, CommonRefC<X, Y>>::Value>>
        {
            using Type = CommonRefC<X, Y>;
        };

        template <typename X, typename Y>
        using CommonRefD = CommonRef<const X&, Y&>;

        template <typename X, typename Y>
        struct CommonRefImpl<
            X&&, Y&, EnableIfType<IsConvertible<X&&, CommonRefD<X, Y>>::Value>>
        {
            using Type = CommonRefD<X, Y>;
        };

        template <typename X, typename Y>
        struct CommonRefImpl<X&, Y&&> : CommonRefImpl<Y&&, X&>
        {
        };

        template <typename T, typename U, template <typename> class TQual,
                  template <typename> class UQual>
        struct BasicCommonReference
        {
        };

        template <typename T>
        struct RefX
        {
            template <typename U>
            using Type = CopyCv<T, U>;
        };

        template <typename T>
        struct RefX<T&>
        {
            template <typename U>
            using Type = CopyCv<T, U>&;
        };

        template <typename T>
        struct RefX<T&&>
        {
            template <typename U>
            using Type = CopyCv<T, U>&&;
        };

        template <typename T1, typename T2>
        using BasicCommonRef =
            typename BasicCommonReference<RemoveCvRefType<T1>, RemoveCvRefType<T2>,
                                          RefX<T1>::template Type,
                                          RefX<T2>::template Type>::Type;

        template <typename... T>
        struct CommonReference;

        template <typename... T>
        using CommonReferenceType = typename CommonReference<T...>::Type;

        template <>
        struct CommonReference<>
        {
        };

        template <typename T0>
        struct CommonReference<T0>
        {
            using Type = T0;
        };

        template <typename T1, typename T2, int Bullet = 1, typename = void>
        struct CommonReferenceImpl : CommonReferenceImpl<T1, T2, Bullet + 1>
        {
        };

        // If sizeof...(T) is two ...
        template <typename T1, typename T2>
        struct CommonReference<T1, T2> : CommonReferenceImpl<T1, T2>
        {
        };

        // If T1 and T2 are reference types and COMMON-REF(T1, T2) is well-formed,
        // ...
        template <typename T1, typename T2>
        struct CommonReferenceImpl<T1&, T2&, 1, VoidType<CommonRef<T1&, T2&>>>
        {
            using Type = CommonRef<T1&, T2&>;
        };

        template <typename T1, typename T2>
        struct CommonReferenceImpl<T1&&, T2&&, 1, VoidType<CommonRef<T1&&, T2&&>>>
        {
            using Type = CommonRef<T1&&, T2&&>;
        };

        template <typename T1, typename T2>
        struct CommonReferenceImpl<T1&, T2&&, 1, VoidType<CommonRef<T1&, T2&&>>>
        {
            using Type = CommonRef<T1&, T2&&>;
        };

        template <typename T1, typename T2>
        struct CommonReferenceImpl<T1&&, T2&, 1, VoidType<CommonRef<T1&&, T2&>>>
        {
            using Type = CommonRef<T1&&, T2&>;
        };

        template <typename T1, typename T2>
        struct CommonReferenceImpl<T1, T2, 2, VoidType<BasicCommonRef<T1, T2>>>
        {
            using Type = BasicCommonRef<T1, T2>;
        };

        template <typename T1, typename T2>
        struct CommonReferenceImpl<T1, T2, 3, VoidType<ConditionResult<T1, T2>>>
        {
            using Type = ConditionResult<T1, T2>;
        };

        template <typename T1, typename T2>
        struct CommonReferenceImpl<T1, T2, 4,
                                   VoidType<typename CommonType<T1, T2>::Type>>
        {
            using Type = typename CommonType<T1, T2>::Type;
        };

        template <typename T1, typename T2>
        struct CommonReferenceImpl<T1, T2, 5, void>
        {
        };

        template <typename...>
        struct CommonTypePack
        {
        };

        template <typename, typename, typename = void>
        struct CommonTypeFold;

        template <typename T1, typename T2, typename... Rest>
        struct CommonReference<T1, T2, Rest...>
            : CommonTypeFold<CommonReference<T1, T2>, CommonTypePack<Rest...>>
        {
        };

        template <typename T1, typename T2, typename... Rest>
        struct CommonTypeFold<CommonReference<T1, T2>, CommonTypePack<Rest...>,
                              VoidType<CommonReferenceType<T1, T2>>>
            : public CommonReference<CommonReferenceType<T1, T2>, Rest...>
        {
        };

        template <typename CT, typename R>
        struct CommonTypeFold<CT, R, void>
        {
        };

        // SFINAE friendly: no `Type` member for non-enumeration types.
        template <typename T, bool = IsEnum<T>::Value>
        struct UnderlyingTypeHelper
        {
        };
        template <typename T>
        struct UnderlyingTypeHelper<T, true>
        {
            using Type = __underlying_type(T);
        };
        template <typename T>
        struct UnderlyingType : UnderlyingTypeHelper<T>
        {
        };

        /**
         * Uninitialized storage suitable for an object of size `Length` with
         * alignment `Align` (std::aligned_storage). Prefer
         * `alignas(T) byte storage[sizeof(T)]` in new code.
         */
        template <usize Length, usize Align = __BIGGEST_ALIGNMENT__>
        struct AlignedStorage
        {
            struct Type
            {
                alignas(Align) unsigned char Data[Length];
            };
        };
        template <usize Length, usize Align = __BIGGEST_ALIGNMENT__>
        using AlignedStorageType = typename AlignedStorage<Length, Align>::Type;

        /// Uninitialized storage big and aligned enough for any of `Types`,
        /// and at least `Length` bytes (std::aligned_union).
        template <usize Length, typename... Types>
        struct AlignedUnion
        {
            static_assert(sizeof...(Types) > 0,
                          "AlignedUnion requires at least one type");

          private:
            static constexpr usize MaxOf(usize first, auto... rest)
            {
                ((first = rest > first ? rest : first), ...);
                return first;
            }

          public:
            static constexpr usize AlignmentValue = MaxOf(alignof(Types)...);
            static constexpr usize SizeValue      = MaxOf(Length, sizeof(Types)...);

            struct Type
            {
                alignas(AlignmentValue) unsigned char Data[SizeValue];
            };
        };
        template <usize Length, typename... Types>
        using AlignedUnionType = typename AlignedUnion<Length, Types...>::Type;

        namespace Details
        {
            template <typename T>
            constexpr T& CopyConstruct(T& t) PM_NOEXCEPT
            {
                return t;
            }
            template <typename T>
            void CopyConstruct(T&&) = delete;
        }; // namespace Details

        template <typename Fn, typename... ArgTypes>
        struct InvokeResult;
        template <typename Fn, typename... ArgTypes>
        struct IsNoThrowInvocable;

        template <typename T>
        class ReferenceWrapper
        {
          public:
            using Type = T;

            template <typename U,
                      typename
                      = decltype(Details::CopyConstruct<T>(DeclVal<U>()),
                                 EnableIfType<!IsSameV<ReferenceWrapper,
                                                       RemoveCvRefType<U>>>())>
            constexpr ReferenceWrapper(U&& u) noexcept(
                noexcept(Details::CopyConstruct<T>(Forward<U>(u))))
                : m_Pointer(AddressOf(Details::CopyConstruct<T>(Forward<U>(u))))
            {
            }
            ReferenceWrapper(const ReferenceWrapper&) PM_NOEXCEPT = default;
            ReferenceWrapper& operator=(const ReferenceWrapper& x) PM_NOEXCEPT
                = default;
            constexpr    operator T&() const PM_NOEXCEPT { return *m_Pointer; }
            constexpr T& Get() const PM_NOEXCEPT { return *m_Pointer; }

            template <typename... ArgTypes>
            constexpr typename InvokeResult<T&, ArgTypes...>::Type
            operator()(ArgTypes&&... args) const
                PM_NOEXCEPT(IsNoThrowInvocable<T&, ArgTypes...>::Value)
            {
                return Invoke(Get(), Forward<ArgTypes>(args)...);
            }

          private:
            T* m_Pointer;
        };
        template <typename T>
        ReferenceWrapper(T&) -> ReferenceWrapper<T>;

        template <typename T>
        struct UnwrapReference
        {
            using Type = T;
        };
        template <typename T>
        struct UnwrapReference<ReferenceWrapper<T>>
        {
            using Type = T&;
        };
        template <typename T>
        struct UnwrapReferenceDecay : UnwrapReference<typename Decay<T>::Type>
        {
        };

        template <typename T>
        using DecayType = typename Decay<T>::Type;
        template <typename... Types>
        using CommonTypeType = typename CommonType<Types...>::Type;

        template <typename T>
            requires(IsEnumV<T>)
        using UnderlyingTypeType = typename UnderlyingType<T>::Type;
        template <typename T>
        using UnwrapReferenceType = typename UnwrapReference<T>::Type;
        template <typename T>
        using UnwrapReferenceDecayType = typename UnwrapReferenceDecay<T>::Type;

        template <typename T>
        inline constexpr auto ToUnderlying(T value) PM_NOEXCEPT
        {
            return static_cast<UnderlyingTypeType<T>>(value);
        }

        template <typename T>
        using RemovePointerType = typename RemovePointer<T>::Type;
    }; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    using Prism::RemovePointer;
    using Prism::RemovePointerType;
    using Prism::ToUnderlying;
#endif

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

    namespace Prism
    {
        template <typename T, typename... Args>
        using IsConstructibleHelper
            = BooleanConstant<__is_constructible(T, Args...)>;
        template <typename T, typename... Args>
        struct IsConstructible : public IsConstructibleHelper<T, Args...>
        {
            // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //               "template argument must be a complete class or an "
            //               "unbounded array!");
        };
        template <typename T, typename... Args>
        using IsTriviallyConstructibleHelper
            = BooleanConstant<__is_trivially_constructible(T, Args...)>;
        template <typename T, typename... Args>
        struct IsTriviallyConstructible
            : public IsTriviallyConstructibleHelper<T, Args...>
        {
            // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //               "template argument must be a complete class or an "
            //               "unbounded array!");
        };
        template <typename T, typename... Args>
        using IsNoThrowConstructibleHelper
            = BooleanConstant<__is_nothrow_constructible(T, Args...)>;
        template <typename T, typename... Args>
        struct IsNoThrowConstructible
            : public IsNoThrowConstructibleHelper<T, Args...>
        {
            // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //               "template argument must be a complete class or an "
            //               "unbounded array!");
        };

        template <typename T>
        struct IsDefaultConstructible : public IsConstructibleHelper<T>
        {
            // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //               "template argument must be a complete class or an "
            //               "unbounded array!");
        };
        template <typename T>
        struct IsTriviallyDefaultConstructible
            : public IsTriviallyConstructibleHelper<T>
        {
            // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //               "template argument must be a complete class or an "
            //               "unbounded array!");
        };
        template <typename T>
        struct IsNoThrowDefaultConstructible : public IsNoThrowConstructible<T>
        {
            // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //               "template argument must be a complete class or an "
            //               "unbounded array!");
        };

        template <typename T>
        struct IsCopyConstructible
            : public IsConstructibleHelper<T, AddLValueReferenceType<const T>>
        {
            // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //               "template argument must be a complete class or an "
            //               "unbounded array!");
        };
        template <typename T>
        struct IsTriviallyCopyConstructible
            : public IsTriviallyConstructibleHelper<T,
                                                    AddLValueReferenceType<const T>>
        {
            // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //               "template argument must be a complete class or an "
            //               "unbounded array!");
        };
        template <typename T>
        struct IsNoThrowCopyConstructible
            : public IsNoThrowConstructibleHelper<T,
                                                  AddLValueReferenceType<const T>>
        {
            // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //               "template argument must be a complete class or an "
            //               "unbounded array!");
        };

        template <typename T>
        struct IsMoveConstructible
            : public IsConstructibleHelper<T, AddRValueReferenceType<T>>
        {
            // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //               "template argument must be a complete class or an "
            //               "unbounded array!");
        };

        template <typename T>
        struct IsTriviallyMoveConstructible
            : public IsTriviallyConstructibleHelper<T, AddRValueReferenceType<T>>
        {
            // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //               "template argument must be a complete class or an "
            //               "unbounded array!");
        };
        template <typename T>
        struct IsNoThrowMoveConstructible
            : public IsNoThrowConstructibleHelper<T, AddRValueReferenceType<T>>
        {
            // static_assert(IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //               "template argument must be a complete class or an "
            //               "unbounded array!");
        };

        template <typename T, typename U>
        using IsAssignableHelper = BooleanConstant<__is_assignable(T, U)>;
        template <typename T, typename U>
        struct IsAssignable : public IsAssignableHelper<T, U>
        {
            // static_assert(
            //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //     "template argument must be a complete class or an unbounded
            //     array");
        };
        template <typename T, typename U>
        using IsTriviallyAssignableHelper
            = BooleanConstant<__is_trivially_assignable(T, U)>;
        template <typename T, typename U>
        struct IsTriviallyAssignable : public IsTriviallyAssignableHelper<T, U>
        {
            // static_assert(
            //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //     "template argument must be a complete class or an unbounded
            //     array");
        };

        template <typename T, typename U>
        using IsNoThrowAssignableHelper
            = BooleanConstant<__is_nothrow_assignable(T, U)>;
        template <typename T, typename U>
        struct IsNoThrowAssignable : public IsNoThrowAssignableHelper<T, U>
        {
            // static_assert(
            //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //     "template argument must be a complete class or an unbounded
            //     array");
        };
        template <typename T>
        struct IsCopyAssignable
            : IsAssignable<AddLValueReferenceType<T>,
                           AddLValueReferenceType<const T>>
        {
            // static_assert(
            //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //     "template argument must be a complete class or an unbounded
            //     array");
        };

        template <typename T>
        struct IsTriviallyCopyAssignable
            : IsTriviallyAssignable<AddLValueReferenceType<T>,
                                    AddLValueReferenceType<const T>>
        {
            // static_assert(
            //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //     "template argument must be a complete class or an unbounded
            //     array");
        };

        template <typename T>
        struct IsNoThrowCopyAssignable
            : IsNoThrowAssignable<AddLValueReferenceType<T>,
                                  AddLValueReferenceType<const T>>
        {
            // static_assert(
            //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //     "template argument must be a complete class or an unbounded
            //     array");
        };
        template <typename T>
        using IsMoveAssignableHelper
            = BooleanConstant<__is_assignable(AddLValueReferenceType<T>,
                                              AddRValueReferenceType<T>)>;
        template <typename T>
        struct IsMoveAssignable : IsMoveAssignableHelper<T>
        {
            // static_assert(
            //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //     "template argument must be a complete class or an unbounded
            //     array");
        };
        template <typename T>
        using IsTriviallyMoveAssignableHelper
            = BooleanConstant<__is_trivially_assignable(
                AddLValueReferenceType<T>, AddRValueReferenceType<T>)>;

        template <typename T>
        struct IsTriviallyMoveAssignable : IsTriviallyMoveAssignableHelper<T>
        {
            // static_assert(
            //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //     "template argument must be a complete class or an unbounded
            //     array");
        };
        template <typename T>
        using IsNoThrowMoveAssignableHelper
            = BooleanConstant<__is_nothrow_assignable(AddLValueReferenceType<T>,
                                                      AddRValueReferenceType<T>)>;
        template <typename T>
        struct IsNoThrowMoveAssignable : IsNoThrowMoveAssignableHelper<T>
        {
            // static_assert(
            //     IsCompleteOrUnbounded(TypeIdentityType<T>{}),
            //     "template argument must be a complete class or an unbounded
            //     array");
        };
        namespace Details
        {
            // std::is_destructible semantics: references are destructible,
            // arrays of known bound are destructible if their element is,
            // void, functions and arrays of unknown bound are not.
            template <typename T>
            struct DestructibleProbe
            {
                static constexpr bool Value = requires(T& t) { t.~T(); };
                static constexpr bool NoThrow
                    = requires(T& t) { { t.~T() } noexcept; };
            };
            template <typename T>
            struct DestructibleProbe<T&>
            {
                static constexpr bool Value   = true;
                static constexpr bool NoThrow = true;
            };
            template <typename T>
            struct DestructibleProbe<T&&>
            {
                static constexpr bool Value   = true;
                static constexpr bool NoThrow = true;
            };
            template <typename T, usize N>
            struct DestructibleProbe<T[N]> : DestructibleProbe<T>
            {
            };
            template <typename T>
            struct DestructibleProbe<T[]>
            {
                static constexpr bool Value   = false;
                static constexpr bool NoThrow = false;
            };
        } // namespace Details

        template <typename T>
        using IsDestructibleHelper
#if PrismHasBuiltin(__is_destructible)
            = BooleanConstant<__is_destructible(T)>;
#else
            = BooleanConstant<Details::DestructibleProbe<T>::Value>;
#endif
        template <typename T>
        struct IsDestructible : IsDestructibleHelper<T>
        {
        };
        template <typename T>
        using IsNoThrowDestructibleHelper
#if PrismHasBuiltin(__is_nothrow_destructible)
            = BooleanConstant<__is_nothrow_destructible(T)>;
#else
            = BooleanConstant<Details::DestructibleProbe<T>::NoThrow>;
#endif
        template <typename T>
        struct IsNoThrowDestructible : IsNoThrowDestructibleHelper<T>
        {
        };
        template <typename T>
        struct HasVirtualDestructor : BooleanConstant<__has_virtual_destructor(T)>
        {
        };
        template <typename T>
        using IsTriviallyDestructibleHelper
#if PrismHasBuiltin(__is_trivially_destructible)
            = BooleanConstant<__is_trivially_destructible(T)>;
#else
            = BooleanConstant<__has_trivial_destructor(T)
                              && IsDestructible<T>::Value>;
#endif

        template <typename T>
        struct IsTriviallyDestructible : IsTriviallyDestructibleHelper<T>
        {
        };
        // Swappability
        //
        // Prism::Swap(T&, T&) is declared in Core.hpp, after these traits, so it
        // can't be found by ordinary lookup here (and never by ADL for
        // fundamental types). Mirror its contract instead: a user-provided Swap
        // found by ADL wins, otherwise a type is swappable when it is move
        // constructible and move assignable; arrays are swappable when their
        // elements are.
        namespace Details::SwapAdl
        {
            // Poison pills: stop unqualified lookup from leaving this namespace.
            template <typename T>
            void Swap(T&, T&) = delete;
            template <typename T, usize N>
            void Swap(T (&)[N], T (&)[N]) = delete;

            template <typename T>
            concept ClassOrEnumLike = __is_class(RemoveReferenceType<T>)
                                   || __is_union(RemoveReferenceType<T>)
                                   || __is_enum(RemoveReferenceType<T>);

            template <typename T, typename U>
            concept AdlSwappable
                = (ClassOrEnumLike<T> || ClassOrEnumLike<U>)
               && requires(T&& t, U&& u) {
                      Swap(static_cast<T&&>(t), static_cast<U&&>(u));
                  };
            template <typename T, typename U>
            concept AdlNoThrowSwappable
                = AdlSwappable<T, U> && requires(T&& t, U&& u) {
                      { Swap(static_cast<T&&>(t), static_cast<U&&>(u)) } noexcept;
                  };

            template <typename T, typename U>
            struct SwapProbe
            {
                static constexpr bool Value   = AdlSwappable<T, U>;
                static constexpr bool NoThrow = AdlNoThrowSwappable<T, U>;
            };
            template <typename T>
            struct SwapProbe<T&, T&>
            {
                static constexpr bool Value
                    = AdlSwappable<T&, T&>
                   || (IsMoveConstructible<T>::Value && IsMoveAssignable<T>::Value);
                static constexpr bool NoThrow
                    = AdlSwappable<T&, T&>
                        ? AdlNoThrowSwappable<T&, T&>
                        : (IsNoThrowMoveConstructible<T>::Value
                           && IsNoThrowMoveAssignable<T>::Value);
            };
            template <typename T, usize N>
            struct SwapProbe<T (&)[N], T (&)[N]>
            {
                static constexpr bool Value   = SwapProbe<T&, T&>::Value;
                static constexpr bool NoThrow = SwapProbe<T&, T&>::NoThrow;
            };
        } // namespace Details::SwapAdl

        template <typename T, typename U>
        struct IsSwappableWith
            : BooleanConstant<Details::SwapAdl::SwapProbe<T, U>::Value
                              && Details::SwapAdl::SwapProbe<U, T>::Value>
        {
        };
        template <typename T>
        struct IsSwappable
            : IsSwappableWith<AddLValueReferenceType<T>, AddLValueReferenceType<T>>
        {
        };

        template <typename T, typename U>
        struct IsNoThrowSwappableWith
            : BooleanConstant<IsSwappableWith<T, U>::Value
                              && Details::SwapAdl::SwapProbe<T, U>::NoThrow
                              && Details::SwapAdl::SwapProbe<U, T>::NoThrow>
        {
        };
        template <typename T>
        struct IsNoThrowSwappable
            : IsNoThrowSwappableWith<AddLValueReferenceType<T>,
                                     AddLValueReferenceType<T>>
        {
        };
#if __has_builtin(__reference_constructs_from_temporary)
        template <typename T, typename U>
        struct ReferenceConstructsFromTemporary
            : BooleanConstant<__reference_constructs_from_temporary(T, U)>
        {
        };
#endif
#if __has_builtin(__reference_converts_from_temporary)
        template <typename T, typename U>
        struct ReferenceConvertsFromTemporary
            : BooleanConstant<__reference_converts_from_temporary(T, U)>
        {
        };
#endif

        template <typename T, typename... Args>
        inline constexpr bool IsConstructibleV = IsConstructible<T, Args...>::Value;
        template <typename T, typename... Args>
        inline constexpr bool IsTriviallyConstructibleV
            = IsTriviallyConstructible<T, Args...>::Value;
        template <typename T, typename... Args>
        inline constexpr bool IsNoThrowConstructibleV
            = IsNoThrowConstructible<T, Args...>::Value;

        template <typename T>
        inline constexpr bool IsDefaultConstructibleV
            = IsDefaultConstructible<T>::Value;
        template <typename T>
        inline constexpr bool IsTriviallyDefaultConstructibleV
            = IsTriviallyDefaultConstructible<T>::Value;
        template <typename T>
        inline constexpr bool IsNoThrowDefaultConstructibleV
            = IsNoThrowDefaultConstructible<T>::Value;

        template <typename T>
        inline constexpr bool IsCopyConstructibleV = IsCopyConstructible<T>::Value;
        template <typename T>
        inline constexpr bool IsTriviallyCopyConstructibleV
            = IsTriviallyCopyConstructible<T>::Value;
        template <typename T>
        inline constexpr bool IsNoThrowCopyConstructibleV
            = IsNoThrowCopyConstructible<T>::Value;

        template <typename T>
        inline constexpr bool IsMoveConstructibleV = IsMoveConstructible<T>::Value;
        template <typename T>
        inline constexpr bool IsTriviallyMoveConstructibleV
            = IsTriviallyMoveConstructible<T>::Value;
        template <typename T>
        inline constexpr bool IsNoThrowMoveConstructibleV
            = IsNoThrowMoveConstructible<T>::Value;

        template <typename T, typename U>
        inline constexpr bool IsAssignableV = IsAssignable<T, U>::Value;
        template <typename T, typename U>
        inline constexpr bool IsTriviallyAssignableV
            = IsTriviallyAssignable<T, U>::Value;
        template <typename T, typename U>
        inline constexpr bool IsNoThrowAssignableV
            = IsNoThrowAssignable<T, U>::Value;

        template <typename T>
        inline constexpr bool IsCopyAssignableV = IsCopyAssignable<T>::Value;
        template <typename T>
        inline constexpr bool IsTriviallyCopyAssignableV
            = IsTriviallyCopyAssignable<T>::Value;
        template <typename T>
        inline constexpr bool IsNoThrowCopyAssignableV
            = IsNoThrowCopyAssignable<T>::Value;

        template <typename T>
        inline constexpr bool IsMoveAssignableV = IsMoveAssignable<T>::Value;
        template <typename T>
        inline constexpr bool IsTriviallyMoveAssignableV
            = IsTriviallyMoveAssignable<T>::Value;
        template <typename T>
        inline constexpr bool IsNoThrowMoveAssignableV
            = IsNoThrowMoveAssignable<T>::Value;

        template <typename T>
        inline constexpr bool IsDestructibleV = IsDestructible<T>::Value;
        template <typename T>
        inline constexpr bool IsTriviallyDestructibleV
            = IsTriviallyDestructible<T>::Value;
        template <typename T>
        inline constexpr bool IsNoThrowDestructibleV
            = IsNoThrowDestructible<T>::Value;
        template <typename T>
        inline constexpr bool HasVirtualDestructorV
            = HasVirtualDestructor<T>::Value;

        template <typename T, typename U>
        inline constexpr bool IsSwappableWithV = IsSwappableWith<T, U>::Value;
        template <typename T>
        inline constexpr bool IsSwappableV = IsSwappable<T>::Value;
        template <typename T, typename U>
        inline constexpr bool IsNoThrowSwappableWithV
            = IsNoThrowSwappableWith<T, U>::Value;
        template <typename T>
        inline constexpr bool IsNoThrowSwappableV = IsNoThrowSwappable<T>::Value;

        template <typename To, typename From>
        using IsArrayConvertible = IsConvertible<From (*)[], To (*)[]>;
    }; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
    using Prism::IsTriviallyDestructible;
    using Prism::IsTriviallyDestructibleV;
#endif

    /**
     * Traits that are not part of <type_traits>, but which libc++ uses
     * internally, or which SerenityOS' AK provides, and are generally useful
     * when writing containers and algorithms.
     */
    namespace Prism
    {
        // -------------------------------------------------------------- relations

        /// T is the same type as one of Ts (AK::IsOneOf).
        template <typename T, typename... Ts>
        struct IsOneOf : Disjunction<IsSame<T, Ts>...>
        {
        };
        /// T and U are the same type, ignoring top-level cv-qualifiers.
        template <typename T, typename U>
        struct IsSameIgnoringCv : IsSame<RemoveCvType<T>, RemoveCvType<U>>
        {
        };
        /// T is one of Ts, ignoring top-level cv-qualifiers.
        template <typename T, typename... Ts>
        struct IsOneOfIgnoringCv : Disjunction<IsSameIgnoringCv<T, Ts>...>
        {
        };

        namespace Details
        {
            template <template <typename...> typename Base, typename... Args>
            TrueType TestTemplateBase(const volatile Base<Args...>*);
            template <template <typename...> typename>
            FalseType TestTemplateBase(const volatile void*);

            template <template <typename...> typename Base, typename Derived>
            auto TestIsTemplateBaseOf(int)
                -> decltype(TestTemplateBase<Base>(DeclVal<Derived*>()));
            template <template <typename...> typename, typename>
            auto TestIsTemplateBaseOf(...) -> FalseType;
        } // namespace Details

        /// Derived is, or publicly and unambiguously derives from, some
        /// specialization of the class template Base (AK::IsTemplateBaseOf).
        template <template <typename...> typename Base, typename Derived>
        struct IsTemplateBaseOf
            : decltype(Details::TestIsTemplateBaseOf<Base, RemoveCvType<Derived>>(
                  0))
        {
        };

        // ---------------------------------------------------- type classification

        /// One of the character types: char, wchar_t, char8_t, char16_t, char32_t.
        template <typename T>
        struct IsCharacter
            : IsOneOf<RemoveCvType<T>, char, wchar_t, char8_t, char16_t, char32_t>
        {
        };
        /// A standard or extended signed integer type: excludes bool and the
        /// character types (libc++'s __libcpp_signed_integer).
        template <typename T>
        struct IsStandardSignedInteger
            : IsOneOf<RemoveCvType<T>, signed char, short, int, long, long long
#ifdef __SIZEOF_INT128__
                      ,
                      __int128
#endif
                      >
        {
        };
        /// A standard or extended unsigned integer type: excludes bool and the
        /// character types (libc++'s __libcpp_unsigned_integer).
        template <typename T>
        struct IsStandardUnsignedInteger
            : IsOneOf<RemoveCvType<T>, unsigned char, unsigned short, unsigned int,
                      unsigned long, unsigned long long
#ifdef __SIZEOF_INT128__
                      ,
                      unsigned __int128
#endif
                      >
        {
        };
        template <typename T>
        struct IsStandardInteger
            : Disjunction<IsStandardSignedInteger<T>, IsStandardUnsignedInteger<T>>
        {
        };

        /// `T&` is a valid type: T is an object type, a function type without
        /// cv/ref qualifiers, or a reference.
        template <typename T>
        struct IsReferenceable : BooleanConstant<requires { typename TypeIdentity<T&>; }>
        {
        };

        // ------------------------------------------------------- transformations

        /// U with the const-qualification of T (AK::CopyConst).
        template <typename T, typename U>
        struct CopyConst
        {
            using Type = ConditionalType<IsConstV<RemoveReferenceType<T>>,
                                         AddConstType<U>, U>;
        };
        template <typename T, typename U>
        using CopyConstType = typename CopyConst<T, U>::Type;

        /// U with the cv-qualifiers of T (libc++'s __copy_cv).
        template <typename T, typename U>
        struct CopyCvQualifiers
        {
            using Type = CopyCv<T, U>;
        };
        template <typename T, typename U>
        using CopyCvType = typename CopyCvQualifiers<T, U>::Type;

        /// U with the cv- and reference-qualifiers of T (libc++'s __copy_cvref).
        template <typename T, typename U>
        struct CopyCvRef
        {
            using Type = CopyCv<T, U>;
        };
        template <typename T, typename U>
        struct CopyCvRef<T&, U>
        {
            using Type = AddLValueReferenceType<CopyCv<T, U>>;
        };
        template <typename T, typename U>
        struct CopyCvRef<T&&, U>
        {
            using Type = AddRValueReferenceType<CopyCv<T, U>>;
        };
        template <typename T, typename U>
        using CopyCvRefType = typename CopyCvRef<T, U>::Type;

        // ------------------------------------------------------- object behaviour

        /**
         * Objects of type T can be relocated (moved to a new address, ending the
         * lifetime of the source) with a plain memcpy.
         *
         * Types can opt in by declaring `using TriviallyRelocatable = TrueType;`
         * or by specializing this trait.
         */
        template <typename T, typename = void>
        struct IsTriviallyRelocatable
#if PrismHasBuiltin(__builtin_is_cpp_trivially_relocatable)
            : BooleanConstant<__builtin_is_cpp_trivially_relocatable(T)>
#else
            : BooleanConstant<__is_trivially_copyable(T)
                              && IsTriviallyDestructible<T>::Value>
#endif
        {
        };
        template <typename T>
        struct IsTriviallyRelocatable<T, VoidType<typename T::TriviallyRelocatable>>
            : BooleanConstant<T::TriviallyRelocatable::Value>
        {
        };

        /**
         * operator== on T and U compares the object representations, so it can
         * be replaced by a memcmp (libc++'s __libcpp_is_trivially_equality_comparable).
         */
        template <typename T, typename U = T>
        struct IsTriviallyEqualityComparable
#if PrismHasBuiltin(__is_trivially_equality_comparable)
            : BooleanConstant<IsSameIgnoringCv<T, U>::Value
                              && __is_trivially_equality_comparable(
                                  RemoveCvType<T>)>
#else
            : BooleanConstant<IsSameIgnoringCv<T, U>::Value
                              && (IsIntegral<T>::Value || IsEnum<T>::Value
                                  || IsPointer<T>::Value)>
#endif
        {
        };

        // ------------------------------------------------------------- detection

        /// Placeholder type for failed detection; can't be created or copied.
        struct Nonesuch
        {
            ~Nonesuch()                     = delete;
            Nonesuch(const Nonesuch&)       = delete;
            void operator=(const Nonesuch&) = delete;
        };

        template <typename Default, template <typename...> typename Op,
                  typename... Args>
        struct DetectedOr
        {
            using Type       = Default;
            using IsDetected = FalseType;
        };
        template <typename Default, template <typename...> typename Op,
                  typename... Args>
            requires requires { typename Op<Args...>; }
        struct DetectedOr<Default, Op, Args...>
        {
            using Type       = Op<Args...>;
            using IsDetected = TrueType;
        };

        /// Op<Args...> if it is well-formed, Default otherwise.
        template <typename Default, template <typename...> typename Op,
                  typename... Args>
        using DetectedOrType = typename DetectedOr<Default, Op, Args...>::Type;
        /// Op<Args...> if it is well-formed, Nonesuch otherwise.
        template <template <typename...> typename Op, typename... Args>
        using DetectedType = DetectedOrType<Nonesuch, Op, Args...>;
        /// Op<Args...> is well-formed.
        template <template <typename...> typename Op, typename... Args>
        using IsDetected = typename DetectedOr<Nonesuch, Op, Args...>::IsDetected;
        /// Op<Args...> is well-formed and is exactly Expected.
        template <typename Expected, template <typename...> typename Op,
                  typename... Args>
        using IsDetectedExact = IsSame<Expected, DetectedType<Op, Args...>>;
        /// Op<Args...> is well-formed and converts to To.
        template <typename To, template <typename...> typename Op,
                  typename... Args>
        using IsDetectedConvertible = IsConvertible<DetectedType<Op, Args...>, To>;

        // ------------------------------------------------------------- utilities

        /// Always false, but dependent on Ts: lets `static_assert(DependentFalse<T>)`
        /// fire only when the enclosing template is instantiated.
        template <typename... Ts>
        inline constexpr bool DependentFalse = false;

        // --------------------------------------------------------- V variables

        template <typename T, typename... Ts>
        inline constexpr bool IsOneOfV = IsOneOf<T, Ts...>::Value;
        template <typename T, typename U>
        inline constexpr bool IsSameIgnoringCvV = IsSameIgnoringCv<T, U>::Value;
        template <typename T, typename... Ts>
        inline constexpr bool IsOneOfIgnoringCvV
            = IsOneOfIgnoringCv<T, Ts...>::Value;
        template <template <typename...> typename Base, typename Derived>
        inline constexpr bool IsTemplateBaseOfV
            = IsTemplateBaseOf<Base, Derived>::Value;
        template <typename T>
        inline constexpr bool IsCharacterV = IsCharacter<T>::Value;
        template <typename T>
        inline constexpr bool IsStandardSignedIntegerV
            = IsStandardSignedInteger<T>::Value;
        template <typename T>
        inline constexpr bool IsStandardUnsignedIntegerV
            = IsStandardUnsignedInteger<T>::Value;
        template <typename T>
        inline constexpr bool IsStandardIntegerV = IsStandardInteger<T>::Value;
        template <typename T>
        inline constexpr bool IsReferenceableV = IsReferenceable<T>::Value;
        template <typename T>
        inline constexpr bool IsTriviallyRelocatableV
            = IsTriviallyRelocatable<T>::Value;
        template <typename T, typename U = T>
        inline constexpr bool IsTriviallyEqualityComparableV
            = IsTriviallyEqualityComparable<T, U>::Value;
        template <template <typename...> typename Op, typename... Args>
        inline constexpr bool IsDetectedV = IsDetected<Op, Args...>::Value;
    }; // namespace Prism

    namespace Prism
    {
        template <typename T, bool>
        struct EnumUnderlying
        {
        };
        template <typename T>
        struct EnumUnderlying<T, true>
        {
            using Type = UnderlyingType<T>;
        };
        template <typename T>
        struct EnumUnderlying<T, false>
        {
            using Type = T;
        };

        template <typename>
        struct IsTupleLikeHelper : FalseType
        {
        };

        template <typename T>
        struct IsTupleLike : public IsTupleLikeHelper<RemoveCvRefType<T>>::Type
        {
        };

        template <typename T>
        struct PromoteEnum
        {
            using Type
                = ConditionalType<IsEnumV<T>,
                                  typename EnumUnderlying<T, IsEnumV<T>>::Type, T>;
        };
        template <typename T>
        using PromoteEnumType = typename PromoteEnum<T>::Type;

        constexpr bool IsConstantEvaluated() PM_NOEXCEPT
        {
            if consteval { return true; }
            return false;
        }
#if PrismHasBuiltin(__builtin_is_within_lifetime)
        /// `ptr` points to an object that is within its lifetime (C++26).
        template <typename T>
        consteval bool IsWithinLifetime(const T* ptr) PM_NOEXCEPT
        {
            return __builtin_is_within_lifetime(ptr);
        }
#endif

        // Member relationships
#if PrismHasBuiltin(__builtin_is_pointer_interconvertible_with_class)
        /// Objects of type S are pointer-interconvertible with their member
        /// `member` (std::is_pointer_interconvertible_with_class).
        template <typename S, typename M>
        constexpr bool IsPointerInterconvertibleWithClass(M S::* member) PM_NOEXCEPT
        {
            return __builtin_is_pointer_interconvertible_with_class(member);
        }
#endif
#if PrismHasBuiltin(__builtin_is_corresponding_member)
        /// `m1` and `m2` refer to corresponding members in the common initial
        /// sequence of S1 and S2 (std::is_corresponding_member).
        template <typename S1, typename S2, typename M1, typename M2>
        constexpr bool IsCorrespondingMember(M1 S1::* m1,
                                             M2 S2::* m2) PM_NOEXCEPT
        {
            return __builtin_is_corresponding_member(m1, m2);
        }
#endif

        template <typename T>
        constexpr bool IsTupleLikeV = IsTupleLike<T>::Value;
    }; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    using Prism::Conditional;
    using Prism::Forward;
    using Prism::IsSameV;
    using Prism::IsTupleLikeV;
    using Prism::Move;
#endif
}
