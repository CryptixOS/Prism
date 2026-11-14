/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>

#include <string.h>

#include "Corpus.hpp"
import Prism.String.Formatter;
import Prism.Test.Test;

using namespace Prism;

// Prism::Format against the output of the patched fmt 10.2, recorded by
// Meta/format-golden/generate.sh. The walk below must visit the corpus in the
// same order as Meta/format-golden/Generate.cpp.
// G() keeps the length, so outputs with a NUL byte compare correctly.
#define G(s) StringView(s, sizeof(s) - 1)
static constexpr StringView kGolden[] = {
#include "Golden.inc"
};
#undef G
static constexpr usize kGoldenCount = sizeof(kGolden) / sizeof(kGolden[0]);

namespace
{
    constexpr usize kReported = 25;

    struct Walker
    {
        usize Index      = 0;
        usize Mismatches = 0;

        void  Check(StringView what, StringView got)
        {
            usize index = Index++;
            if (index >= kGoldenCount)
            {
                if (index == kGoldenCount)
                    FAIL("Golden.inc has fewer records than the corpus: "
                         "rerun Meta/format-golden/generate.sh");
                return;
            }
            StringView want = kGolden[index];
            if (got.Size() == want.Size()
                && memcmp(got.Raw(), want.Raw(), want.Size()) == 0)
                return;
            // Report the first few; the rest would only be noise.
            if (Mismatches++ < kReported)
                FAIL(Format("#{} {}\n  prism: [{}]\n  fmt:   [{}]", index, what,
                            got, want));
        }
    };

    template <typename T>
    void Integers(Walker& walker, StringView type)
    {
        for (const char* spec : Golden::kIntegerSpecs)
            for (long long value : Golden::kIntegerValues)
                walker.Check(Format("{} with ({}){}", spec, type, value),
                             Format(Runtime(spec), static_cast<T>(value)));
    }
} // namespace

TEST_CASE(matches_fmt_10_2)
{
    Walker walker;

    Integers<signed char>(walker, "signed char");
    Integers<short>(walker, "short");
    Integers<int>(walker, "int");
    Integers<long long>(walker, "long long");
    Integers<unsigned char>(walker, "unsigned char");
    Integers<unsigned>(walker, "unsigned");
    Integers<unsigned long long>(walker, "unsigned long long");

    for (const char* spec : Golden::kCharCodeSpecs)
    {
        for (int code : Golden::kCharCodes)
            walker.Check(Format("{} with {}", spec, code),
                         Format(Runtime(spec), code));
        for (int code : Golden::kCharCodes)
            walker.Check(Format("{} with (unsigned char){}", spec, code),
                         Format(Runtime(spec), static_cast<unsigned char>(code)));
    }
    for (const char* spec : Golden::kCharSpecs)
        for (char c : Golden::kChars)
            walker.Check(Format("{} with char {:d}", spec, c),
                         Format(Runtime(spec), c));
    for (const char* spec : Golden::kBoolSpecs)
        for (bool b : Golden::kBools)
            walker.Check(Format("{} with {}", spec, b),
                         Format(Runtime(spec), b));
    for (const char* spec : Golden::kStringSpecs)
        for (const char* s : Golden::kStrings)
            walker.Check(Format("{} with {:?}", spec, s),
                         Format(Runtime(spec), s));
    for (const char* spec : Golden::kPointerSpecs)
        for (unsigned long long p : Golden::kPointers)
            walker.Check(Format("{} with pointer {:#x}", spec, p),
                         Format(Runtime(spec), reinterpret_cast<const void*>(
                                                   static_cast<upointer>(p))));

#define GOLDEN_MIXED(...) walker.Check(#__VA_ARGS__, Format(__VA_ARGS__));
    GOLDEN_MIXED_CASES(GOLDEN_MIXED)
#undef GOLDEN_MIXED

    if (walker.Index != kGoldenCount)
        FAIL(Format("the corpus has {} records, Golden.inc {}: rerun "
                    "Meta/format-golden/generate.sh",
                    walker.Index, kGoldenCount));
    if (walker.Mismatches > kReported)
        FAIL(Format("... {} mismatches in total", walker.Mismatches));
}
