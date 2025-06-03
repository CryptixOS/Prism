/*
 * Created by v1tr10l7 on 27.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Core/Details/CVRefTraits.inl>
#include <Prism/Core/Details/CallableTraits.inl>
#include <Prism/Core/Details/Conditional.inl>
#include <Prism/Core/Details/Constructibility.inl>
#include <Prism/Core/Details/CoreTraits.inl>
#include <Prism/Core/Details/TransformationTraits.inl>
#include <Prism/Core/Details/TypeProperties.inl>

using namespace Prism;
void TypeTraits_TestIsSame()
{
    static_assert(IsSameV<int, int>, "IsSame<int, int> should be true");
    static_assert(!IsSameV<int, float>, "IsSame<int, float> should be false");
}

void TypeTraits_TestIsConst()
{
    static_assert(IsConstV<const int>, "IsConst<const int> should be true");
    static_assert(!IsConstV<int>, "IsConst<int> should be false");
}

void TypeTraits_TestRemoveConst()
{
    static_assert(IsSameV<RemoveConstType<const int>, int>,
                  "RemoveConst<const int> should be int");
    static_assert(IsSameV<RemoveConstType<int>, int>,
                  "RemoveConst<int> should stay int");
}

void TypeTraits_TestRemoveReference()
{
    static_assert(IsSameV<RemoveReferenceType<int&>, int>,
                  "RemoveReference<int&> should be int");
    static_assert(IsSameV<RemoveReferenceType<int&&>, int>,
                  "RemoveReference<int&&> should be int");
    static_assert(IsSameV<RemoveReferenceType<int>, int>,
                  "RemoveReference<int> should be int");
}

void TypeTraits_TestAddConst()
{
    static_assert(IsSameV<AddConstType<int>, const int>,
                  "AddConst<int> should be const int");
    static_assert(IsSameV<AddConstType<const int>, const int>,
                  "AddConst<const int> should stay const int");
}

void TypeTraits_TestEnableIf()
{
    static_assert(IsSameV<EnableIfType<true, int>, int>,
                  "EnableIf<true, int> should be int");
    // EnableIf<false, int> should not compile, so we can't assert it directly.
}

void TypeTraits_TestIsPointer()
{
    static_assert(IsPointerV<int*>, "IsPointer<int*> should be true");
    static_assert(!IsPointerV<int>, "IsPointer<int> should be false");
}

void TypeTraits_TestDecay()
{
    static_assert(IsSameV<DecayType<int&>, int>, "Decay<int&> should be int");
    static_assert(IsSameV<DecayType<const int&>, int>,
                  "Decay<const int&> should be int");
    static_assert(IsSameV<DecayType<int>, int>, "Decay<int> should be int");
}

int main()
{
    TypeTraits_TestIsSame();
    TypeTraits_TestIsConst();
    TypeTraits_TestRemoveConst();
    TypeTraits_TestRemoveReference();
    TypeTraits_TestAddConst();
    TypeTraits_TestEnableIf();
    TypeTraits_TestIsPointer();
    TypeTraits_TestDecay();

    return 0;
}
