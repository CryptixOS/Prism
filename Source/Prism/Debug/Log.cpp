/*
 * Created by v1tr10l7 on 21.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Core/Types.hpp>
#include <Prism/Debug/Log.hpp>

#if PRISM_TARGET_CRYPTIX == 1
    #include <Library/Logger.hpp>
#endif

#include <cstdio>
#include <utility>

#include <magic_enum/magic_enum.hpp>

namespace Prism::Log
{
#if PRISM_TARGET_CRYPTIX == 0
    constexpr const char* s_LevelForegroundColors[] = {
        "\u001b[37m", "\u001b[35m", "\u001b[32m", "\u001b[36m",
        "\u001b[33m", "\u001b[31m", "\u001b[37m",
    };
#endif

#if PRISM_TARGET_CRYPTIX == 0
    static void PrintLevel(LogLevel level)
    {
        if (level == LogLevel::eNone) return;
        std::printf("[");
        std::printf("%s", s_LevelForegroundColors[ToUnderlying(level)]);
        if (level == LogLevel::eFatal) std::printf("\u001b[41");

        std::printf("%s", magic_enum::enum_name(level).data() + 1);

        std::printf("\u001b[37m");
        std::printf("\u001b[0m");
        std::printf("]: ");
    }
#endif

    void Print(LogLevel logLevel, StringView str)
    {
#if PRISM_TARGET_CRYPTIX != 0
        Logger::Log(logLevel, str);
#else
        PrintLevel(logLevel);
        std::printf("%s", str.Raw());

        std::putchar('\n');
#endif
    }
}; // namespace Prism::Log
