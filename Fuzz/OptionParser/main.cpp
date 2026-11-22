/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
// Command lines from the fuzzer: the input is split at NUL bytes into argv,
// and the first argument also serves as the short option string.
#include <Prism/Core/Types.hpp>
import Prism.Utility.OptionParser;

using namespace Prism;

extern "C" int LLVMFuzzerTestOneInput(const u8* data, usize size)
{
    constexpr usize kMaxArgs = 32;
    char            storage[1024];
    if (size >= sizeof(storage)) return 0;
    for (usize i = 0; i < size; ++i) storage[i] = static_cast<char>(data[i]);
    storage[size] = '\0';

    char* argv[kMaxArgs + 1];
    i32   argc = 0;
    argv[argc++] = storage;
    for (usize i = 0; i < size && argc < i32(kMaxArgs); ++i)
        if (storage[i] == '\0') argv[argc++] = storage + i + 1;
    argv[argc] = nullptr;

    i32              flag            = 0;
    const LongOption longOptions[] = {
        {"alpha", LongOption::eNoArgument, nullptr, 'a'},
        {"beta", LongOption::eRequiredArgument, nullptr, 'b'},
        {"bet", LongOption::eOptionalArgument, &flag, 'c'},
        {"gamma", LongOption::eOptionalArgument, nullptr, 'g'},
        {nullptr, 0, nullptr, 0},
    };

    for (bool longOnly : {false, true})
    {
        OptionParser parser;
        parser.PrintErrors = 0;
        i32 index          = -1;
        // Every call consumes at least one character, so this terminates.
        for (usize calls = 0; calls < 4 * size + 8; ++calls)
        {
            i32 c = parser.Next(argc, argv, argv[0], longOptions, &index,
                                longOnly);
            if (c == -1) break;
        }
        if (parser.Index < 0 || parser.Index > argc) __builtin_trap();
    }
    return 0;
}
