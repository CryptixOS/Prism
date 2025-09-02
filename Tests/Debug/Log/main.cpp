/*
 * Created by v1tr10l7 on 02.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Debug/Log.hpp>

using namespace Prism;

int main(int argc, char** argv)
{
    IgnoreUnused(argc);
    IgnoreUnused(argv);

    Log::Log(LogLevel::eTrace, "Hello");
    Log::Log(LogLevel::eInfo, "Hello");
    Log::Log(LogLevel::eDebug, "Hello");
    Log::Log(LogLevel::eWarn, "Hello");
    Log::Log(LogLevel::eError, "Hello");
    Log::Log(LogLevel::eFatal, "Hello");

    return EXIT_SUCCESS;
}
