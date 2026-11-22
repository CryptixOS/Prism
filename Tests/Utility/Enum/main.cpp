/*
 * Created by v1tr10l7 on 11.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
import Prism.Test.Test;
import Prism.Utility.Enum;

using namespace Prism;

enum class Color : u8
{
    eRed   = 0,
    eGreen = 1,
    eBlue  = 5,
    eWhite = 255,
};

namespace Nested::Deeper
{
    enum Unscoped : i32
    {
        eNegative = -3,
        eZero     = 0,
        eSeven    = 7,
    };
    enum class Scoped : i16
    {
        eA_1 = 2,
        e_b2 = 3,
    };
} // namespace Nested::Deeper

// Far outside the default range; needs a custom one.
enum class Opcode : u16
{
    eLow  = 0x10,
    eMid  = 0x5b00,
    eHigh = 0x8a00,
};
template <>
struct Prism::EnumRange<Opcode>
{
    static constexpr i64 Min = 0;
    static constexpr i64 Max = 0x8a00;
};

// A custom range that excludes a declared enumerator.
enum class Narrowed : i32
{
    eOne   = 1,
    eTwo   = 2,
    eFifty = 50,
};
template <>
struct Prism::EnumRange<Narrowed>
{
    static constexpr i64 Min = 0;
    static constexpr i64 Max = 10;
};

enum class Aliased : u8
{
    eFirst  = 1,
    eSecond = 1,
    eOther  = 2,
};

enum class Empty : u8
{
};

enum class Wide : u64
{
    eSmall = 3,
};

// Everything is usable at compile time.
static_assert(EnumName(Color::eRed) == "eRed"_sv);
static_assert(EnumName(Color::eWhite) == "eWhite"_sv);
static_assert(EnumName(static_cast<Color>(2)).Empty());
static_assert(EnumContains(Color::eBlue));
static_assert(!EnumContains(static_cast<Color>(4)));
static_assert(EnumCount<Color> == 4);
static_assert(EnumName(Opcode::eHigh) == "eHigh"_sv);
static_assert(EnumCount<Empty> == 0);

TEST_CASE(Enum_ScopedNames)
{
    EXPECT_EQ(EnumName(Color::eRed), "eRed"_sv);
    EXPECT_EQ(EnumName(Color::eGreen), "eGreen"_sv);
    EXPECT_EQ(EnumName(Color::eBlue), "eBlue"_sv);
    EXPECT_EQ(EnumName(Color::eWhite), "eWhite"_sv);
}

TEST_CASE(Enum_UnnamedValues)
{
    for (u32 v : {2u, 3u, 4u, 6u, 128u, 254u})
    {
        auto color = static_cast<Color>(v);
        EXPECT(EnumName(color).Empty());
        EXPECT(!EnumContains(color));
    }
}

TEST_CASE(Enum_NamespacedAndUnscoped)
{
    using namespace Nested::Deeper;
    EXPECT_EQ(EnumName(eNegative), "eNegative"_sv);
    EXPECT_EQ(EnumName(eZero), "eZero"_sv);
    EXPECT_EQ(EnumName(eSeven), "eSeven"_sv);
    EXPECT(!EnumContains(static_cast<Unscoped>(-1)));
    EXPECT_EQ(EnumCount<Unscoped>, 3u);

    EXPECT_EQ(EnumName(Scoped::eA_1), "eA_1"_sv);
    EXPECT_EQ(EnumName(Scoped::e_b2), "e_b2"_sv);
}

TEST_CASE(Enum_CustomRange)
{
    EXPECT_EQ(EnumName(Opcode::eLow), "eLow"_sv);
    EXPECT_EQ(EnumName(Opcode::eMid), "eMid"_sv);
    EXPECT_EQ(EnumName(Opcode::eHigh), "eHigh"_sv);
    EXPECT(!EnumContains(static_cast<Opcode>(0x8a01)));
    EXPECT_EQ(EnumCount<Opcode>, 3u);

    EXPECT(EnumContains(Narrowed::eTwo));
    EXPECT(!EnumContains(Narrowed::eFifty));
    EXPECT(EnumName(Narrowed::eFifty).Empty());
    EXPECT_EQ(EnumCount<Narrowed>, 2u);
}

enum class Large : i32
{
    eInRange    = 127,
    eOutOfRange = 128,
};

TEST_CASE(Enum_DefaultRange)
{
    // 1-byte enums are scanned whole, past PRISM_ENUM_RANGE_MAX.
    EXPECT(EnumContains(Color::eWhite));

    // Wider ones stop at PRISM_ENUM_RANGE_MAX (127 by default).
    EXPECT(EnumContains(Large::eInRange));
    EXPECT(!EnumContains(Large::eOutOfRange));

    // u64 is clamped to what i64 can hold.
    EXPECT_EQ(EnumName(Wide::eSmall), "eSmall"_sv);
}

enum class SignedByte : i8
{
    eMin = -128,
    eMax = 127,
};

TEST_CASE(Enum_SignedByte)
{
    EXPECT_EQ(EnumName(SignedByte::eMin), "eMin"_sv);
    EXPECT_EQ(EnumName(SignedByte::eMax), "eMax"_sv);
    EXPECT_EQ(EnumCount<SignedByte>, 2u);
}

TEST_CASE(Enum_AliasesAndEmpty)
{
    EXPECT(EnumContains(Aliased::eSecond));
    EXPECT_EQ(EnumCount<Aliased>, 2u);
    EXPECT_EQ(EnumName(Aliased::eOther), "eOther"_sv);

    EXPECT(!EnumContains(static_cast<Empty>(0)));
    EXPECT(EnumName(static_cast<Empty>(0)).Empty());
}
