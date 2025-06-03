/*
 * Created by v1tr10l7 on 01.06.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Details/Conditional.inl>

namespace Prism
{
    template <typename T>
    struct IsVoid : IsSame<void, typename RemoveCv<T>::Type>
    {
    };
    template <typename T>
    struct IsNullPointer : IsSame<std::nullptr_t, typename RemoveCv<T>::Type>
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
        : public IsFloatingPointHelper<typename RemoveCv<T>::Type>::Type
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
    struct IsArithmetic;
    template <typename T>
    struct IsFundamental
        : IntegralConstant<
              bool,
              IsArithmetic<T>::Value || IsVoid<T>::Value
                  || IsSame<std::nullptr_t, typename RemoveCv<T>::Type>::Value>
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
    template <typename T>
    struct IsStandardLayout : public BooleanConstant<__is_standard_layout(T)>
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
    // template <typename T>
    // struct IsImplicitLifetime : BooleanConstant<__is_implicit_lifetime(T)>
    //{
    // };

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
    struct IsBoundedArray : BooleanConstant<__is_bounded_array(T)>
    {
    };
    template <typename T>
    struct IsUnboundedArray : BooleanConstant<__is_unbounded_array(T)>
    {
    };
    template <typename T>
    struct IsScopedEnum : BooleanConstant<__is_scoped_enum(T)>
    {
    };

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
#ifdef __cpp_lib_is_virtual_base_of
    template <typename Base, typename Derived>
    struct IsVirtualBaseOf
        : BooleanConstant<__builtin_is_virtual_base_of(Base, Derived)>
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
    template <typename T, typename U>
    struct IsLayoutCompatible : BooleanConstant<__is_layout_compatible(T, U)>
    {
    };

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
    // template <typename T>
    // constexpr bool IsImplicitLifetimeV = IsImplicitLifetime<T>::Value;
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
#ifdef __cpp_lib_is_virtual_base_of
    template <typename Base, typename Derived>
    constexpr bool IsVirtualBaseOfV = IsVirtualBaseOf<Base, Derived>::Value;
#endif
    template <typename From, typename To>
    constexpr bool IsConvertibleV = IsConvertible<From, To>::Value;
    template <typename From, typename To>
    constexpr bool IsNoThrowConvertibleV
        = IsNoThrowConvertible<From, To>::Value;
    template <typename T, typename U>
    constexpr bool IsLayoutCompatibleV = IsLayoutCompatible<T, U>::Value;

    template <typename T, usize N = 0>
    constexpr usize ExtentV = Extent<T, N>::Value;
}; // namespace Prism
