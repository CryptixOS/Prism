/*
 * Created By V1tr10l7 On 01.06.2025.
 * Copyright (C) 2024-2025, Szymon Zemke <V1tr10l7@Proton.Me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

namespace Prism
{
    template <typename T, T V>
    struct IntegralConstant;

    template <bool B>
    using BooleanConstant = IntegralConstant<bool, B>;
    using TrueType        = BooleanConstant<true>;
    using falseType       = BooleanConstant<false>;

    // Primary Type Categories
    template <typename T>
    struct IsVoid;
    template <typename T>
    struct IsNullPointer;
    template <typename T>
    struct IsIntegral;
    template <typename T>
    struct IsFloatingPoint;
    template <typename T>
    struct IsArray;
    template <typename T>
    struct IsPointer;
    template <typename T>
    struct IsLValueReference;
    template <typename T>
    struct IsRValueReference;
    template <typename T>
    struct IsMemberObjectPointer;
    template <typename T>
    struct IsMemberFunctionPointer;
    template <typename T>
    struct IsEnum;
    template <typename T>
    struct IsUnion;
    template <typename T>
    struct IsTypename;
    template <typename T>
    struct IsFunction;

    // Composite Type Categories
    template <typename T>
    struct IsReference;
    template <typename T>
    struct IsArithmetic;
    template <typename T>
    struct IsFundamental;
    template <typename T>
    struct IsObject;
    template <typename T>
    struct IsScalar;
    template <typename T>
    struct IsCompound;
    template <typename T>
    struct IsMemberPointer;

    // Type Properties
    template <typename T>
    struct IsConst;
    template <typename T>
    struct IsVolatile;
    template <typename T>
    struct IsTrivial;
    template <typename T>
    struct IsTriviallyCopyable;
    template <typename T>
    struct IsStandardLayout;
    template <typename T>
    struct IsEmpty;
    template <typename T>
    struct IsPolymorphic;
    template <typename T>
    struct IsAbstract;
    template <typename T>
    struct IsFinal;
    template <typename T>
    struct IsAggregate;

    template <typename T>
    struct IsSigned;
    template <typename T>
    struct IsUnsigned;
    template <typename T>
    struct IsBoundedArray;
    template <typename T>
    struct IsUnboundedArray;
    template <typename T>
    struct IsScopedEnum;

    template <typename T, typename... Args>
    struct IsConstructible;
    template <typename T>
    struct IsDefaultConstructible;
    template <typename T>
    struct IsCopyConstructible;
    template <typename T>
    struct IsMoveConstructible;

    template <typename T, typename U>
    struct IsAssignable;
    template <typename T>
    struct IsCopyAssignable;
    template <typename T>
    struct IsMoveAssignable;

    template <typename T, typename U>
    struct IsSwappableWith;
    template <typename T>
    struct IsSwappable;

    template <typename T>
    struct IsDestructible;

    template <typename T, typename... Args>
    struct IsTriviallyConstructible;
    template <typename T>
    struct IsTriviallyDefaultConstructible;
    template <typename T>
    struct IsTriviallyCopyConstructible;
    template <typename T>
    struct IsTriviallyMoveConstructible;

    template <typename T, typename U>
    struct IsTriviallyAssignable;
    template <typename T>
    struct IsTriviallyCopyAssignable;
    template <typename T>
    struct IsTriviallyMoveAssignable;
    template <typename T>
    struct IsTriviallyDestructible;

    template <typename T, typename... Args>
    struct IsNoThrowConstructible;
    template <typename T>
    struct IsNoThrowDefaultConstructible;
    template <typename T>
    struct IsNoThrowCopyConstructible;
    template <typename T>
    struct IsNoThrowMoveConstructible;

    template <typename T, typename U>
    struct IsNoThrowAssignable;
    template <typename T>
    struct IsNoThrowCopyAssignable;
    template <typename T>
    struct IsNoThrowMoveAssignable;

    template <typename T, typename U>
    struct IsNoThrowSwappableWith;
    template <typename T>
    struct IsNoThrowSwappable;

    template <typename T>
    struct IsNoThrowDestructible;

    template <typename T>
    struct HasVirtualDestructor;

    template <typename T>
    struct HasUniqueObjectRepresentations;

    template <typename T, typename U>
    struct ReferenceConstructsFromTemporary;
    template <typename T, typename U>
    struct ReferenceConvertsFromTemporary;

    // Type Property Queries
    template <typename T>
    struct AlignmentOf;
    template <typename T>
    struct Rank;
    template <typename T, u32 I = 0>
    struct Extent;

    // Type Relations
    template <typename T, typename U>
    struct IsSame;
    template <typename Base, typename Derived>
    struct IsBaseOf;
    template <typename Base, typename Derived>
    struct IsVirtualBaseOf;
    template <typename From, typename To>
    struct IsConvertible;
    template <typename From, typename To>
    struct IsNoThrowConvertible;
    template <typename T, typename U>
    struct IsLayoutCompatible;
    template <typename Base, typename Derived>
    struct IsPointerInterconvertibleBaseOf;

    template <typename Fn, typename... ArgTypes>
    struct IsInvocable;
    template <typename R, typename Fn, typename... ArgTypes>
    struct IsInvocableR;

    template <typename Fn, typename... ArgTypes>
    struct IsNoThrowInvocable;
    template <typename R, typename Fn, typename... ArgTypes>
    struct IsNoThrowInvocableR;

    // const-Volatile Modifications
    template <typename T>
    struct RemoveConst;
    template <typename T>
    struct RemoveVolatile;
    template <typename T>
    struct RemoveCv;
    template <typename T>
    struct AddConst;
    template <typename T>
    struct AddVolatile;
    template <typename T>
    struct AddCv;

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

    // Reference Modifications
    template <typename T>
    struct RemoveReference;
    template <typename T>
    struct AddLValueReference;
    template <typename T>
    struct AddRValueReference;

    template <typename T>
    using RemoveReferenceType = typename RemoveReference<T>::Type;
    template <typename T>
    using AddLValueReferenceType = typename AddLValueReference<T>::Type;
    template <typename T>
    using AddRValueReferenceType = typename AddRValueReference<T>::Type;

    // Sign Modifications
    template <typename T>
    struct MakeSigned;
    template <typename T>
    struct MakeUnsigned;

    template <typename T>
    using MakeSignedType = typename MakeSigned<T>::Type;
    template <typename T>
    using MakeUnsignedType = typename MakeUnsigned<T>::Type;

    // Array Modifications
    template <typename T>
    struct RemoveExtent;
    template <typename T>
    struct RemoveAllExtents;

    template <typename T>
    using RemoveExtentT = typename RemoveExtent<T>::Type;
    template <typename T>
    using RemoveAllExtentsT = typename RemoveAllExtents<T>::Type;

    // Pointer Modifications
    template <typename T>
    struct RemovePointer;
    template <typename T>
    struct AddPointer;

    template <typename T>
    using RemovePointerT = typename RemovePointer<T>::Type;
    template <typename T>
    using AddPointerT = typename AddPointer<T>::Type;

    // Other Transformations
    template <typename T>
    struct TypeIdentity;
    template <typename T>
    struct RemoveCvReference;
    template <typename T>
    struct Decay;
    template <bool, typename T = void>
    struct EnableIf;
    template <bool, typename T, typename F>
    struct Conditional;
    template <typename... T>
    struct CommonType;
    template <typename T, typename U, template <typename> typename TQual,
              template <typename> typename UQual>
    struct BasicCommonReference
    {
    };
    template <typename... T>
    struct CommonReference;
    template <typename T>
    struct UnderlyingType;
    template <typename Fn, typename... ArgTypes>
    struct InvokeResult;
    template <typename T>
    struct UnwrapReference;
    template <typename T>
    struct UnwrapRefDecay;

    template <typename T>
    using TypeIdentityT = typename TypeIdentity<T>::Type;
    template <typename T>
    using RemoveCvReferenceType = typename RemoveCvReference<T>::Type;
    template <typename T>
    using DecayType = typename Decay<T>::Type;
    template <bool B, typename T = void>
    using EnableIfType = typename EnableIf<B, T>::Type;
    template <bool B, typename T, typename F>
    using ConditionalType = typename Conditional<B, T, F>::Type;
    template <typename... T>
    using CommonTypeT = typename CommonType<T...>::Type;
    template <typename... T>
    using CommonReferenceType = typename CommonReference<T...>::Type;
    template <typename T>
    using UnderlyingTypeT = typename UnderlyingType<T>::Type;
    template <typename Fn, typename... ArgTypes>
    using InvokeResultType = typename InvokeResult<Fn, ArgTypes...>::Type;
    template <typename T>
    using UnwrapReferenceType = typename UnwrapReference<T>::Type;
    template <typename T>
    using UnwrapRefDecayType = typename UnwrapRefDecay<T>::Type;
    template <typename...>
    using VoidT = void;

    // Logical Operator Traits
    template <typename... B>
    struct Conjunction;
    template <typename... B>
    struct Disjunction;
    template <typename B>
    struct Negation;

    // Primary Type Categories
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
    inline constexpr bool IsPointerV = IsPointer<T>::Value;
    template <typename T>
    inline constexpr bool IsLValueReferenceV = IsLValueReference<T>::Value;
    template <typename T>
    inline constexpr bool IsRValueReferenceV = IsRValueReference<T>::Value;
    template <typename T>
    inline constexpr bool IsMemberObjectPointerV
        = IsMemberObjectPointer<T>::Value;
    template <typename T>
    inline constexpr bool IsMemberFunctionPointerV
        = IsMemberFunctionPointer<T>::Value;
    template <typename T>
    inline constexpr bool IsEnumV = IsEnum<T>::Value;
    template <typename T>
    inline constexpr bool IsUnionV = IsUnion<T>::Value;
    template <typename T>
    inline constexpr bool IsTypenameV = IsTypename<T>::Value;
    template <typename T>
    inline constexpr bool IsFunctionV = IsFunction<T>::Value;

    // Composite Type Categories
    template <typename T>
    inline constexpr bool IsReferenceV = IsReference<T>::Value;
    template <typename T>
    inline constexpr bool IsArithmeticV = IsArithmetic<T>::Value;
    template <typename T>
    inline constexpr bool IsFundamentalV = IsFundamental<T>::Value;
    template <typename T>
    inline constexpr bool IsObjectV = IsObject<T>::Value;
    template <typename T>
    inline constexpr bool IsScalarV = IsScalar<T>::Value;
    template <typename T>
    inline constexpr bool IsCompoundV = IsCompound<T>::Value;
    template <typename T>
    inline constexpr bool IsMemberPointerV = IsMemberPointer<T>::Value;

    // Type Properties
    template <typename T>
    inline constexpr bool IsConstV = IsConst<T>::Value;
    template <typename T>
    inline constexpr bool IsVolatileV = IsVolatile<T>::Value;
    template <typename T>
    inline constexpr bool IsTrivialV = IsTrivial<T>::Value;
    template <typename T>
    inline constexpr bool IsTriviallyCopyableV = IsTriviallyCopyable<T>::Value;
    template <typename T>
    inline constexpr bool IsStandardLayoutV = IsStandardLayout<T>::Value;
    template <typename T>
    inline constexpr bool IsEmptyV = IsEmpty<T>::Value;
    template <typename T>
    inline constexpr bool IsPolymorphicV = IsPolymorphic<T>::Value;
    template <typename T>
    inline constexpr bool IsAbstractV = IsAbstract<T>::Value;
    template <typename T>
    inline constexpr bool IsFinalV = IsFinal<T>::Value;
    template <typename T>
    inline constexpr bool IsAggregateV = IsAggregate<T>::Value;
    template <typename T>
    inline constexpr bool IsSignedV = IsSigned<T>::Value;
    template <typename T>
    inline constexpr bool IsUnsignedV = IsUnsigned<T>::Value;
    template <typename T>
    inline constexpr bool IsBoundedArrayV = IsBoundedArray<T>::Value;
    template <typename T>
    inline constexpr bool IsUnboundedArrayV = IsUnboundedArray<T>::Value;
    template <typename T>
    inline constexpr bool IsScopedEnumV = IsScopedEnum<T>::Value;
    template <typename T, typename... Args>
    inline constexpr bool IsConstructibleV = IsConstructible<T, Args...>::Value;
    template <typename T>
    inline constexpr bool IsDefaultConstructibleV
        = IsDefaultConstructible<T>::Value;
    template <typename T>
    inline constexpr bool IsCopyConstructibleV = IsCopyConstructible<T>::Value;
    template <typename T>
    inline constexpr bool IsMoveConstructibleV = IsMoveConstructible<T>::Value;
    template <typename T, typename U>
    inline constexpr bool IsAssignableV = IsAssignable<T, U>::Value;
    template <typename T>
    inline constexpr bool IsCopyAssignableV = IsCopyAssignable<T>::Value;
    template <typename T>
    inline constexpr bool IsMoveAssignableV = IsMoveAssignable<T>::Value;
    template <typename T, typename U>
    inline constexpr bool IsSwappableWithV = IsSwappableWith<T, U>::Value;
    template <typename T>
    inline constexpr bool IsSwappableV = IsSwappable<T>::Value;
    template <typename T>
    inline constexpr bool IsDestructibleV = IsDestructible<T>::Value;
    template <typename T, typename... Args>
    inline constexpr bool IsTriviallyConstructibleV
        = IsTriviallyConstructible<T, Args...>::Value;
    template <typename T>
    inline constexpr bool IsTriviallyDefaultConstructibleV
        = IsTriviallyDefaultConstructible<T>::Value;
    template <typename T>
    inline constexpr bool IsTriviallyCopyConstructibleV
        = IsTriviallyCopyConstructible<T>::Value;
    template <typename T>
    inline constexpr bool IsTriviallyMoveConstructibleV
        = IsTriviallyMoveConstructible<T>::Value;
    template <typename T, typename U>
    inline constexpr bool IsTriviallyAssignableV
        = IsTriviallyAssignable<T, U>::Value;
    template <typename T>
    inline constexpr bool IsTriviallyCopyAssignableV
        = IsTriviallyCopyAssignable<T>::Value;
    template <typename T>
    inline constexpr bool IsTriviallyMoveAssignableV
        = IsTriviallyMoveAssignable<T>::Value;
    template <typename T>
    inline constexpr bool IsTriviallyDestructibleV
        = IsTriviallyDestructible<T>::Value;
    template <typename T, typename... Args>
    inline constexpr bool IsNoThrowConstructibleV
        = IsNoThrowConstructible<T, Args...>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowDefaultConstructibleV
        = IsNoThrowDefaultConstructible<T>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowCopyConstructibleV
        = IsNoThrowCopyConstructible<T>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowMoveConstructibleV
        = IsNoThrowMoveConstructible<T>::Value;
    template <typename T, typename U>
    inline constexpr bool IsNoThrowAssignableV
        = IsNoThrowAssignable<T, U>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowCopyAssignableV
        = IsNoThrowCopyAssignable<T>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowMoveAssignableV
        = IsNoThrowMoveAssignable<T>::Value;
    template <typename T, typename U>
    inline constexpr bool IsNoThrowSwappableWithV
        = IsNoThrowSwappableWith<T, U>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowSwappableV = IsNoThrowSwappable<T>::Value;
    template <typename T>
    inline constexpr bool IsNoThrowDestructibleV
        = IsNoThrowDestructible<T>::Value;
    template <typename T>
    inline constexpr bool HasVirtualDestructorV
        = HasVirtualDestructor<T>::Value;
    template <typename T>
    inline constexpr bool HasUniqueObjectRepresentationsV
        = HasUniqueObjectRepresentations<T>::Value;
    template <typename T, typename U>
    inline constexpr bool ReferenceConstructsFromTemporaryV
        = ReferenceConstructsFromTemporary<T, U>::Value;
    template <typename T, typename U>
    inline constexpr bool ReferenceConvertsFromTemporaryV
        = ReferenceConvertsFromTemporary<T, U>::Value;

    // Type Property Queries
    template <typename T>
    inline constexpr usize AlignmentOfV = AlignmentOf<T>::Value;
    template <typename T>
    inline constexpr usize RankV = Rank<T>::Value;
    template <typename T, u32 I = 0>
    inline constexpr usize ExtentV = Extent<T, I>::Value;

    // Type Relations
    template <typename T, typename U>
    inline constexpr bool IsSameV = IsSame<T, U>::Value;
    template <typename Base, typename Derived>
    inline constexpr bool IsBaseOfV = IsBaseOf<Base, Derived>::Value;
    template <typename Base, typename Derived>
    inline constexpr bool IsVirtualBaseOfV
        = IsVirtualBaseOf<Base, Derived>::Value;
    template <typename From, typename To>
    inline constexpr bool IsConvertibleV = IsConvertible<From, To>::Value;
    template <typename From, typename To>
    inline constexpr bool IsNoThrowConvertibleV
        = IsNoThrowConvertible<From, To>::Value;
    template <typename T, typename U>
    inline constexpr bool IsLayoutCompatibleV = IsLayoutCompatible<T, U>::Value;
    template <typename Base, typename Derived>
    inline constexpr bool IsPointerInterconvertibleBaseOfV
        = IsPointerInterconvertibleBaseOf<Base, Derived>::Value;
    template <typename Fn, typename... ArgTypes>
    inline constexpr bool IsInvocableV = IsInvocable<Fn, ArgTypes...>::Value;
    template <typename R, typename Fn, typename... ArgTypes>
    inline constexpr bool IsInvocableRV
        = IsInvocableR<R, Fn, ArgTypes...>::Value;
    template <typename Fn, typename... ArgTypes>
    inline constexpr bool IsNoThrowInvocableV
        = IsNoThrowInvocable<Fn, ArgTypes...>::Value;
    template <typename R, typename Fn, typename... ArgTypes>
    inline constexpr bool IsNoThrowInvocableRV
        = IsNoThrowInvocableR<R, Fn, ArgTypes...>::Value;

    // Logical Operator Traits
    template <typename... B>
    inline constexpr bool ConjunctionV = Conjunction<B...>::Value;
    template <typename... B>
    inline constexpr bool DisjunctionV = Disjunction<B...>::Value;
    template <typename B>
    inline constexpr bool NegationV = Negation<B>::Value;

    // Member Relationships
    template <typename S, typename M>
    constexpr bool
    IsPointerInterconvertibleWithTypename(M S::* Member) PM_NOEXCEPT;
    template <typename S1, typename S2, typename M1, typename M2>
    constexpr bool IsCorrespondingMember(M1 S1::* Member1,
                                         M2 S2::* Member2) PM_NOEXCEPT;
    // Constant Evaluation Context
    constexpr bool IsConstantEvaluated() PM_NOEXCEPT;
    consteval bool IsWithinLifetime(const auto*) PM_NOEXCEPT;
}; // namespace Prism
