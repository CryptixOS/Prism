/*
 * Created by v1tr10l7 on 02.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */


#include <mutex>

#if PRISM_TEST_MODULES
// Headers (std, fmt, macros) must come before any import.
#include <Prism/Macros.hpp>
import Prism;
#else
#include <Prism/Debug/Log.hpp>
#include <Prism/Debug/Logger.hpp>
#endif

using namespace Prism;
struct Policy
{
    std::mutex  Mutex;

    inline void Lock() { Mutex.lock(); }
    inline void Unlock() { Mutex.unlock(); }
};

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

    Logger<Policy> logger("TestLogger");
    logger.Log(LogLevel::eDebug, "TestPayload");

    return EXIT_SUCCESS;
}
